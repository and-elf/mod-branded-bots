#ifndef MOD_BRANDED_BOTS_CORE_BOTS_BOTPROFICIENCY_H
#define MOD_BRANDED_BOTS_CORE_BOTS_BOTPROFICIENCY_H

#include <cstdint>

// Pure core. No AzerothCore includes are permitted anywhere under core/ (determinism + fast TDD).
namespace BrandedBots
{
    // Map a bot's *character* level to a branding *proficiency* level, so a branded bot expresses its
    // brand effect at a strength that "scales to bot level" (design decision): a max-level bot reaches
    // full strength (maxEffectLevel), a low-level bot proportionally less. mod-branding's effect model
    // uses strength = proficiencyLevel / maxEffectLevel, so this is the only knob that turns a bot's
    // brand from inert (level 0 -> multiplier 1.0) into a visible, scaling combat effect.
    //
    // Pure and branch-clamped -- deterministic, no game state, unit-tested:
    //   - maxBotLevel == 0 or maxEffectLevel == 0 -> 0 (nothing to scale onto / effects disabled)
    //   - botLevel >= maxBotLevel -> maxEffectLevel (clamped; never overshoots the ceiling)
    //   - otherwise a linear scale rounded to the nearest proficiency level
    uint8_t ProficiencyLevelForBotLevel(uint32_t botLevel, uint32_t maxBotLevel, uint8_t maxEffectLevel);
}

#endif // MOD_BRANDED_BOTS_CORE_BOTS_BOTPROFICIENCY_H
