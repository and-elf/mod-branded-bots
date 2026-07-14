#include "BrandedBotsMgr.h"
#include "Config.h"
#include "ItemBrandingMgr.h"        // sItemBrandingMgr, ItemBrandingMgr::BrandEquipped
#include "LoadoutMgr.h"             // sLoadoutMgr->SetActiveBrand (active-school loadout)
#include "Log.h"
#include "Player.h"
#include "ProficiencyMgr.h"         // sProficiencyMgr: UnlockBrand / SetBrandLevel
#include "RandomPlayerbotMgr.h"     // sRandomPlayerbotMgr.IsRandomBot
#include "World.h"                  // sWorld->getIntConfig(CONFIG_MAX_PLAYER_LEVEL)
#include "bots/BotProficiency.h"    // ProficiencyLevelForBotLevel (pure core)
#include "branding/common/Brand.h"  // Branding::BrandId
#include "branding/common/Rng.h"    // Branding::IRng
#include <exception>
#include <sstream>
#include <string>

namespace
{
    // Parse a comma-separated list of spell ids into `out` (index = school / BrandId). Blank or
    // malformed fields become 0 (no visual). Extra fields past the school count are ignored; missing
    // trailing fields leave 0. Adapter-side (pure core stays AzerothCore-free).
    void ParseVisualSpellCsv(std::string const& csv,
        std::array<uint32_t, static_cast<size_t>(Branding::BrandId::COUNT)>& out)
    {
        out.fill(0);
        std::stringstream ss(csv);
        std::string field;
        size_t idx = 0;
        while (idx < out.size() && std::getline(ss, field, ','))
        {
            try
            {
                out[idx] = static_cast<uint32_t>(std::stoul(field));
            }
            catch (std::exception const&)
            {
                out[idx] = 0;   // blank / non-numeric -> no visual for this school
            }
            ++idx;
        }
    }
}

namespace
{
    // Deterministic per-seed LCG: a given bot GUID always yields the same brand, so the brand
    // "follows the bot" across logins and gear swaps with no persistence of our own. Adapter-side
    // (the pure core stays RNG-free); shares the constants of the test FakeRng.
    class SeededRng : public Branding::IRng
    {
    public:
        explicit SeededRng(uint32_t seed) : _state(seed ? seed : 1u) { }

        uint32_t Next(uint32_t bound) override
        {
            _state = _state * 1664525u + 1013904223u;
            return bound == 0 ? 0 : _state % bound;
        }

    private:
        uint32_t _state;
    };

    uint32_t AllBrandsMask()
    {
        uint32_t mask = 0;
        for (uint32_t i = 0; i < static_cast<uint32_t>(Branding::BrandId::COUNT); ++i)
            mask |= (1u << i);
        return mask;
    }
}

namespace BrandedBots
{
    BrandedBotsMgr* BrandedBotsMgr::instance()
    {
        static BrandedBotsMgr instance;
        return &instance;
    }

    void BrandedBotsMgr::LoadConfig()
    {
        _enabled = sConfigMgr->GetOption<bool>("BrandedBots.Enable", false);
        _enabledMask = sConfigMgr->GetOption<uint32>("BrandedBots.EnabledSchoolMask", AllBrandsMask());
        _noBrandPermille = sConfigMgr->GetOption<uint32>("BrandedBots.NoBrandChancePermille", 0);

        _expressBrand = sConfigMgr->GetOption<bool>("BrandedBots.ExpressBrand", true);
        // Same key mod-branding's EffectConfig reads -- single source of truth for the effect ceiling.
        _maxEffectLevel = static_cast<uint8_t>(sConfigMgr->GetOption<uint32>("Branding.Effect.MaxEffectLevel", 50));
        ParseVisualSpellCsv(sConfigMgr->GetOption<std::string>("BrandedBots.VisualSpells", ""), _visualSpells);
    }

    void BrandedBotsMgr::OnBotLogin(Player* player)
    {
        if (!_enabled || !player)
            return;

        if (!sRandomPlayerbotMgr.IsRandomBot(player))
            return;

        // Deterministic per-bot seed from the GUID (GetRawValue, not GetCounter -- codestyle).
        SeededRng rng(static_cast<uint32_t>(player->GetGUID().GetRawValue()));

        BotContext const ctx{ static_cast<uint8_t>(player->getClass()), static_cast<uint8_t>(player->GetLevel()) };
        BrandSelection const selection = _policy.Select(ctx, rng, *this);
        if (!selection.branded)
            return;

        // Etch onto the bot's equipped main-hand. Returns false if it has no eligible weapon yet;
        // that just means we try again on the next login once playerbots has geared it. Kept for the
        // addon/status readout and future item-intensity work; the observable brand comes from the
        // account-scoped express/visual path below, which does not depend on a specific item instance.
        if (sItemBrandingMgr->BrandEquipped(player, selection.brand))
        {
            LOG_DEBUG("module.brandedbots", "Branded bot {} with school {}",
                player->GetGUID().ToString(), static_cast<uint32>(selection.brand));
        }

        ExpressAndVisualize(player, selection.brand);
    }

    void BrandedBotsMgr::ExpressAndVisualize(Player* bot, Branding::BrandId brand) const
    {
        if (!bot)
            return;

        // Cosmetic first (independent of effects): cast the school's aura so the bot is visibly branded.
        if (auto const idx = static_cast<size_t>(brand); idx < _visualSpells.size())
        {
            if (uint32 const visual = _visualSpells[idx])
                bot->CastSpell(bot, visual, true);
        }

        if (!_expressBrand)
            return;

        // Functional: unlock the school for the bot's account, make it the active loadout brand, and
        // grant a bot-level-scaled proficiency so mod-branding's effect model (strength = level /
        // maxEffectLevel) actually expresses. All three persist to the DB immediately -- this script
        // runs before mod-branding's own login load, which reloads from the DB, so the grants survive.
        uint32 const accountId = bot->GetSession()->GetAccountId();
        sProficiencyMgr->UnlockBrand(accountId, brand);
        sLoadoutMgr->SetActiveBrand(bot, brand);

        uint32 const maxBotLevel = sWorld->getIntConfig(CONFIG_MAX_PLAYER_LEVEL);
        uint8 const profLevel = ProficiencyLevelForBotLevel(bot->GetLevel(), maxBotLevel, _maxEffectLevel);
        sProficiencyMgr->SetBrandLevel(bot->GetGUID(), brand, profLevel);
    }
}
