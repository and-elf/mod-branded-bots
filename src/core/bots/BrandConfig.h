#ifndef MOD_BRANDED_BOTS_CORE_BOTS_BRANDCONFIG_H
#define MOD_BRANDED_BOTS_CORE_BOTS_BRANDCONFIG_H

#include <cstdint>

// Pure core. No AzerothCore includes are permitted anywhere under core/ (determinism + fast TDD).
namespace BrandedBots
{
    // Injected tunables for bot brand assignment. The pure core reads no globals; production wraps
    // sConfigMgr, tests inject a fake with pinned values so the roll is deterministic.
    class IBotBrandConfig
    {
    public:
        virtual ~IBotBrandConfig() = default;

        // Bitmask over Branding::BrandId: bit i set => BrandId(i) may be rolled for a bot. Bots roll
        // only brands whose bit is set. An empty mask (0) disables branding -- bots stay unbranded.
        virtual uint32_t EnabledBrandMask() const = 0;

        // Per-mille chance [0, 1000] that a bot is left UNBRANDED. 0 => every bot is branded (default
        // for "allies"); 1000 => none are. Lets the roster be a mix of branded and plain bots.
        virtual uint32_t NoBrandChancePermille() const = 0;
    };
}

#endif // MOD_BRANDED_BOTS_CORE_BOTS_BRANDCONFIG_H
