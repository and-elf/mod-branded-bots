#ifndef MOD_BRANDED_BOTS_TESTS_FAKES_FAKERNG_H
#define MOD_BRANDED_BOTS_TESTS_FAKES_FAKERNG_H

#include "branding/common/Rng.h"

namespace BrandedBots::Fakes
{
    // Deterministic LCG so tests are reproducible for a given seed (same construction as
    // mod-branding's FakeRng -- kept local to keep this module's test build self-contained).
    class FakeRng : public Branding::IRng
    {
    public:
        explicit FakeRng(uint32_t seed = 12345) : _state(seed) { }

        uint32_t Next(uint32_t bound) override
        {
            _state = _state * 1664525u + 1013904223u;
            return bound == 0 ? 0 : _state % bound;
        }

    private:
        uint32_t _state;
    };
}

#endif // MOD_BRANDED_BOTS_TESTS_FAKES_FAKERNG_H
