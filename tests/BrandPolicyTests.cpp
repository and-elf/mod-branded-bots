#include "bots/BrandPolicy.h"
#include "fakes/FakeRng.h"
#include "fakes/FakeBotBrandConfig.h"
#include <gtest/gtest.h>
#include <set>

using namespace BrandedBots;
using Branding::BrandId;

namespace
{
    uint32_t Bit(BrandId brand)
    {
        return 1u << static_cast<uint32_t>(brand);
    }

    BotContext AnyBot()
    {
        return BotContext{ /*classId*/ 1, /*level*/ 60 };
    }
}

// Empty mask means branding is disabled -- the bot stays unbranded regardless of RNG.
TEST(UniformBotBrandPolicy, EmptyMaskLeavesUnbranded)
{
    UniformBotBrandPolicy policy;
    Fakes::FakeRng rng;
    Fakes::FakeBotBrandConfig config;
    config.enabledMask = 0;
    config.noBrandPermille = 0;

    for (int i = 0; i < 100; ++i)
    {
        BrandSelection const sel = policy.Select(AnyBot(), rng, config);
        EXPECT_FALSE(sel.branded);
    }
}

// A single enabled brand is always the one chosen.
TEST(UniformBotBrandPolicy, SingleBrandMaskAlwaysThatBrand)
{
    UniformBotBrandPolicy policy;
    Fakes::FakeRng rng;
    Fakes::FakeBotBrandConfig config;
    config.enabledMask = Bit(BrandId::Frost);
    config.noBrandPermille = 0;

    for (int i = 0; i < 1000; ++i)
    {
        BrandSelection const sel = policy.Select(AnyBot(), rng, config);
        ASSERT_TRUE(sel.branded);
        EXPECT_EQ(sel.brand, BrandId::Frost);
    }
}

// A chosen brand's bit is always set in the enabled mask -- never a disabled brand.
TEST(UniformBotBrandPolicy, NeverSelectsDisabledBrand)
{
    UniformBotBrandPolicy policy;
    Fakes::FakeRng rng;
    Fakes::FakeBotBrandConfig config;
    config.enabledMask = Bit(BrandId::Fire) | Bit(BrandId::Shadow) | Bit(BrandId::Venom);
    config.noBrandPermille = 0;

    for (int i = 0; i < 5000; ++i)
    {
        BrandSelection const sel = policy.Select(AnyBot(), rng, config);
        ASSERT_TRUE(sel.branded);
        EXPECT_TRUE(config.enabledMask & Bit(sel.brand)) << "picked a brand outside the mask";
    }
}

// NoBrandChancePermille == 1000 => never branded, even with a full mask.
TEST(UniformBotBrandPolicy, NoBrandChanceFullAlwaysUnbranded)
{
    UniformBotBrandPolicy policy;
    Fakes::FakeRng rng;
    Fakes::FakeBotBrandConfig config;
    config.enabledMask = Fakes::FakeBotBrandConfig::AllBrandsMask();
    config.noBrandPermille = 1000;

    for (int i = 0; i < 1000; ++i)
        EXPECT_FALSE(policy.Select(AnyBot(), rng, config).branded);
}

// NoBrandChancePermille == 0 with a non-empty mask => always branded.
TEST(UniformBotBrandPolicy, NoBrandChanceZeroAlwaysBranded)
{
    UniformBotBrandPolicy policy;
    Fakes::FakeRng rng;
    Fakes::FakeBotBrandConfig config;
    config.enabledMask = Fakes::FakeBotBrandConfig::AllBrandsMask();
    config.noBrandPermille = 0;

    for (int i = 0; i < 1000; ++i)
        EXPECT_TRUE(policy.Select(AnyBot(), rng, config).branded);
}

// Same seed => identical sequence of selections (no hidden globals / clock).
TEST(UniformBotBrandPolicy, DeterministicPerSeed)
{
    UniformBotBrandPolicy policy;
    Fakes::FakeBotBrandConfig config;   // default: full mask, always branded

    Fakes::FakeRng rngA(42);
    Fakes::FakeRng rngB(42);
    for (int i = 0; i < 1000; ++i)
    {
        BrandSelection const a = policy.Select(AnyBot(), rngA, config);
        BrandSelection const b = policy.Select(AnyBot(), rngB, config);
        EXPECT_EQ(a.branded, b.branded);
        EXPECT_EQ(a.brand, b.brand);
    }
}

// Over many rolls with a full mask, every enabled brand is eventually chosen (uniform coverage).
TEST(UniformBotBrandPolicy, CoversEveryEnabledBrand)
{
    UniformBotBrandPolicy policy;
    Fakes::FakeRng rng;
    Fakes::FakeBotBrandConfig config;   // default: full mask, always branded

    std::set<BrandId> seen;
    for (int i = 0; i < 20000; ++i)
    {
        BrandSelection const sel = policy.Select(AnyBot(), rng, config);
        ASSERT_TRUE(sel.branded);
        seen.insert(sel.brand);
    }

    EXPECT_EQ(seen.size(), static_cast<size_t>(BrandId::COUNT));
}
