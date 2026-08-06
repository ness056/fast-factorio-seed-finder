#pragma once

#include "finder.hpp"

// Has to be copy/move assignable/constructible.
struct SeedCache {
    float stage1_area;
};

constexpr Finder<SeedCache>::StageSettings stage3_settings{
    // Finally we add elevation to check for water.
    .check_twin_seeds = true,
    .check_water_settings = false,
    .check_elevation_types = false,

    .seed_nb_to_next_stage = 100    // Only the top 100 seeds will make it to the output file.
};

Finder<SeedCache>::EvalResult stage3_eval(const MapGenSettings&, const NoisePrecompute&, NoiseCache&, uint32_t seed, SeedCache*);
