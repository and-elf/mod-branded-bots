#include "bots/BotProficiency.h"

namespace BrandedBots
{
    uint8_t ProficiencyLevelForBotLevel(uint32_t botLevel, uint32_t maxBotLevel, uint8_t maxEffectLevel)
    {
        if (maxBotLevel == 0 || maxEffectLevel == 0)
            return 0;

        if (botLevel >= maxBotLevel)
            return maxEffectLevel;

        // Rounded linear scale: (botLevel * maxEffectLevel + maxBotLevel/2) / maxBotLevel. The
        // +maxBotLevel/2 term rounds to the nearest level; 64-bit math avoids overflow on the product.
        uint64_t const scaled =
            (static_cast<uint64_t>(botLevel) * maxEffectLevel + maxBotLevel / 2) / maxBotLevel;
        return static_cast<uint8_t>(scaled);
    }
}
