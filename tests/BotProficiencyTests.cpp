#include "bots/BotProficiency.h"
#include <gtest/gtest.h>

using namespace BrandedBots;

// A max-level (or higher) bot expresses at the full effect level.
TEST(ProficiencyLevelForBotLevel, MaxBotLevelYieldsFullStrength)
{
    EXPECT_EQ(ProficiencyLevelForBotLevel(80, 80, 50), 50);
    EXPECT_EQ(ProficiencyLevelForBotLevel(99, 80, 50), 50);   // clamped, never overshoots
}

// Level 0 / degenerate configs express nothing.
TEST(ProficiencyLevelForBotLevel, ZeroInputsYieldZero)
{
    EXPECT_EQ(ProficiencyLevelForBotLevel(0, 80, 50), 0);
    EXPECT_EQ(ProficiencyLevelForBotLevel(40, 0, 50), 0);    // no bot-level span
    EXPECT_EQ(ProficiencyLevelForBotLevel(40, 80, 0), 0);    // effects disabled / no ceiling
}

// The scale is linear in bot level, rounded to the nearest proficiency level.
TEST(ProficiencyLevelForBotLevel, LinearRoundedScale)
{
    EXPECT_EQ(ProficiencyLevelForBotLevel(40, 80, 50), 25);  // exactly half
    EXPECT_EQ(ProficiencyLevelForBotLevel(20, 80, 50), 13);  // 12.5 -> 13 (round half up)
    EXPECT_EQ(ProficiencyLevelForBotLevel(60, 80, 50), 38);  // 37.5 -> 38
    EXPECT_EQ(ProficiencyLevelForBotLevel(1, 80, 50), 1);    // 0.625 -> 1
}

// Monotonic non-decreasing: a higher-level bot never expresses at a lower proficiency.
TEST(ProficiencyLevelForBotLevel, MonotonicInBotLevel)
{
    uint8_t prev = 0;
    for (uint32_t lvl = 0; lvl <= 80; ++lvl)
    {
        uint8_t const cur = ProficiencyLevelForBotLevel(lvl, 80, 50);
        EXPECT_GE(cur, prev) << "regressed at bot level " << lvl;
        prev = cur;
    }
    EXPECT_EQ(prev, 50);
}
