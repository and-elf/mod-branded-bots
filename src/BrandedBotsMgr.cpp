#include "BrandedBotsMgr.h"
#include "Config.h"
#include "ItemBrandingMgr.h"        // sItemBrandingMgr, ItemBrandingMgr::BrandEquipped
#include "Log.h"
#include "Player.h"
#include "RandomPlayerbotMgr.h"     // sRandomPlayerbotMgr.IsRandomBot
#include "branding/common/Brand.h"  // Branding::BrandId
#include "branding/common/Rng.h"    // Branding::IRng

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
        // that just means we try again on the next login once playerbots has geared it.
        if (sItemBrandingMgr->BrandEquipped(player, selection.brand))
        {
            LOG_DEBUG("module.brandedbots", "Branded bot {} with school {}",
                player->GetGUID().ToString(), static_cast<uint32>(selection.brand));
        }
    }
}
