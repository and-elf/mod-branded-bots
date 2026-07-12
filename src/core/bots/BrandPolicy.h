#ifndef MOD_BRANDED_BOTS_CORE_BOTS_BRANDPOLICY_H
#define MOD_BRANDED_BOTS_CORE_BOTS_BRANDPOLICY_H

#include "bots/BrandConfig.h"
#include "branding/common/Brand.h"   // Branding::BrandId -- single source of truth (mod-branding core)
#include "branding/common/Rng.h"     // Branding::IRng -- injected randomness

namespace BrandedBots
{
    // Immutable description of the bot whose brand is being chosen. classId/level let richer policies
    // weight by class/spec (e.g. Physical for a warrior); the default uniform policy ignores them.
    struct BotContext
    {
        uint8_t classId = 0;   // core Classes enum value (adapter passes Player::getClass()); 0 = unknown
        uint8_t level = 0;
    };

    // Result of a brand roll. branded == false => leave the bot unbranded (no weapon etch).
    struct BrandSelection
    {
        bool branded = false;
        Branding::BrandId brand = Branding::BrandId::Fire;
    };

    // Injected strategy (DI, mirrors mod-branded-mercenary's policy interfaces): pick a brand for a
    // bot. Deterministic given the injected RNG; no globals, no <random>, no wall-clock.
    class IBotBrandPolicy
    {
    public:
        virtual ~IBotBrandPolicy() = default;

        virtual BrandSelection Select(BotContext const& bot, Branding::IRng& rng,
            IBotBrandConfig const& config) const = 0;
    };

    // Default policy: uniform over the enabled-brand mask, after an optional "no brand" roll.
    // Contract (GoogleTested): never returns a brand whose bit is clear in EnabledBrandMask();
    // empty mask => unbranded; NoBrandChancePermille == 0 with a non-empty mask => always branded;
    // == 1000 => always unbranded; deterministic per seed.
    class UniformBotBrandPolicy : public IBotBrandPolicy
    {
    public:
        BrandSelection Select(BotContext const& bot, Branding::IRng& rng,
            IBotBrandConfig const& config) const override;
    };
}

#endif // MOD_BRANDED_BOTS_CORE_BOTS_BRANDPOLICY_H
