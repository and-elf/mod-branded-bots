#ifndef MOD_BRANDED_BOTS_TESTS_FAKES_FAKEBOTBRANDCONFIG_H
#define MOD_BRANDED_BOTS_TESTS_FAKES_FAKEBOTBRANDCONFIG_H

#include "bots/BrandConfig.h"
#include "branding/common/Brand.h"

namespace BrandedBots::Fakes
{
    // Injectable config with pinned values so brand rolls are deterministic under test.
    class FakeBotBrandConfig : public IBotBrandConfig
    {
    public:
        uint32_t enabledMask = AllBrandsMask();
        uint32_t noBrandPermille = 0;

        uint32_t EnabledBrandMask() const override { return enabledMask; }
        uint32_t NoBrandChancePermille() const override { return noBrandPermille; }

        // Every defined brand enabled.
        static uint32_t AllBrandsMask()
        {
            uint32_t mask = 0;
            for (uint32_t i = 0; i < static_cast<uint32_t>(Branding::BrandId::COUNT); ++i)
                mask |= (1u << i);
            return mask;
        }
    };
}

#endif // MOD_BRANDED_BOTS_TESTS_FAKES_FAKEBOTBRANDCONFIG_H
