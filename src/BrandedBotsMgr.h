#ifndef MOD_BRANDED_BOTS_SRC_BRANDEDBOTSMGR_H
#define MOD_BRANDED_BOTS_SRC_BRANDEDBOTSMGR_H

#include "bots/BrandConfig.h"
#include "bots/BrandPolicy.h"
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

        bool _enabled = false;
        uint32_t _enabledMask = 0;
        uint32_t _noBrandPermille = 0;
        UniformBotBrandPolicy _policy;
    };
}

#define sBrandedBotsMgr BrandedBots::BrandedBotsMgr::instance()

#endif // MOD_BRANDED_BOTS_SRC_BRANDEDBOTSMGR_H
