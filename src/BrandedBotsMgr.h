#ifndef MOD_BRANDED_BOTS_SRC_BRANDEDBOTSMGR_H
#define MOD_BRANDED_BOTS_SRC_BRANDEDBOTSMGR_H

#include "bots/BrandConfig.h"
#include "bots/BrandPolicy.h"
#include "branding/common/Brand.h"   // Branding::BrandId (school taxonomy / COUNT)
#include <array>
#include <cstdint>

class Player;

namespace BrandedBots
{
    // Adapter singleton (§4). On a random bot's login it derives a brand from the bot's GUID
    // (deterministic -> stable across logins and re-gears, no table of its own) and etches it onto
    // the bot's equipped weapon via mod-branding's ItemBrandingMgr. The brand is proc-based, so no
    // bot-AI awareness is needed; the actual brand state persists on the item (item_branding).
    //
    // Implements IBotBrandConfig so it can inject its own tunables into the pure-core policy.
    class BrandedBotsMgr : public IBotBrandConfig
    {
    public:
        static BrandedBotsMgr* instance();

        // (Re)read config on startup and `.reload config`.
        void LoadConfig();
        bool Enabled() const { return _enabled; }

        // Assign + apply the brand if `player` is a random bot. No-op otherwise, if disabled, or if
        // the bot has no eligible equipped weapon yet (retried on the next login once geared).
        void OnBotLogin(Player* player);

        // IBotBrandConfig
        uint32_t EnabledBrandMask() const override { return _enabledMask; }
        uint32_t NoBrandChancePermille() const override { return _noBrandPermille; }

    private:
        BrandedBotsMgr() = default;

        // Make a branded bot observable (design override of ARCHITECTURE.md §5): cast the school's
        // cosmetic aura so it is visibly branded, and -- when _expressBrand -- unlock the school for the
        // bot's account, set it as the active loadout brand, and grant a bot-level-scaled proficiency so
        // mod-branding's effect model expresses (a level-0 brand is inert). No-op if effects are off.
        void ExpressAndVisualize(Player* bot, Branding::BrandId brand) const;

        bool _enabled = false;
        uint32_t _enabledMask = 0;
        uint32_t _noBrandPermille = 0;

        // Whether to grant knowledge + active loadout + proficiency so the brand's combat effect
        // expresses (requires mod-branding's Branding.Effect.Enable = 1 to have any effect).
        bool _expressBrand = false;
        // mod-branding's effect ceiling (Branding.Effect.MaxEffectLevel), the target of the level scale.
        uint8_t _maxEffectLevel = 0;
        // Per-school cosmetic aura spell ids (index = BrandId); 0 = no visual for that school.
        std::array<uint32_t, static_cast<std::size_t>(Branding::BrandId::COUNT)> _visualSpells{};

        UniformBotBrandPolicy _policy;
    };
}

#define sBrandedBotsMgr BrandedBots::BrandedBotsMgr::instance()

#endif // MOD_BRANDED_BOTS_SRC_BRANDEDBOTSMGR_H
