#include "bots/BrandPolicy.h"
#include <cstddef>

namespace BrandedBots
{
    // Uniform over the enabled-brand mask, after an optional "leave unbranded" roll. classId/level
    // are unused by the uniform policy (a weighted policy would consult them).
    BrandSelection UniformBotBrandPolicy::Select(BotContext const& /*bot*/, Branding::IRng& rng,
        IBotBrandConfig const& config) const
    {
        // "Leave unbranded" roll first. Short-circuit the deterministic extremes without touching the
        // RNG so behaviour is stable regardless of mask size.
        uint32_t const noBrandPermille = config.NoBrandChancePermille();
        if (noBrandPermille >= 1000u)
            return {};
        if (noBrandPermille > 0u && rng.Next(1000u) < noBrandPermille)
            return {};

        // Collect the enabled brands (bit set in the mask).
        uint8_t allowed[static_cast<size_t>(Branding::BrandId::COUNT)];
        uint32_t count = 0;
        uint32_t const mask = config.EnabledBrandMask();
        for (uint32_t i = 0; i < static_cast<uint32_t>(Branding::BrandId::COUNT); ++i)
            if (mask & (1u << i))
                allowed[count++] = static_cast<uint8_t>(i);

        if (count == 0)
            return {};   // empty / misconfigured mask: leave unbranded

        BrandSelection selection;
        selection.branded = true;
        selection.brand = static_cast<Branding::BrandId>(allowed[rng.Next(count)]);
        return selection;
    }
}
