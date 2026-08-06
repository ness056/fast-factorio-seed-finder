#define _USE_MATH_DEFINES
#include <array>
#include "stages.hpp"

#include "noise.hpp"
#include "algorithm.hpp"


Finder<SeedCache>::EvalResult stage3_eval(
    const MapGenSettings& settings, const NoisePrecompute& precompute, NoiseCache& noise_cache, uint32_t seed, SeedCache*
) {
    Noise noise(seed, true, settings.elevation_type == ELEVATION_2_0);
    PositionF32 pos;
    float score = 0.f;
    /* Old slow approach: just counting water tiles
    for (float i = -512.f; i < 512.f; i = i + 1.f) {
        pos.x = i;
        for (float j = -512.f; j < 512.f; j = j + 1.f) {
            pos.y = j;
            if (noise.is_tile_water(settings, precompute, pos)) {
                score++;
            }
        }
    };
    */

    // Checking water presence in lines from outer 1024x1024 square towards center
    int32_t radius = 16; // half the square sizes in the lines
    int32_t sampling_distance = 8; // check every N tiles

    // lines as coordinates in a 32x32 grid, 1/8th defined for flipping around
    // 8 times to cover the whole circle. See detection_zones.png
    std::array<std::array<std::pair<int, int>, 8>, 10> lines = {{
        {{{ 0,15}, { 0,14}, { 0,13}, { 0,12}, { 0,11}, { 0,10}, { 0, 0}, { 0, 0}}}, // red
        {{{ 2,15}, { 2,14}, { 1,13}, { 1,12}, { 1,11}, { 0, 0}, { 0, 0}, { 0, 0}}}, // green
        {{{ 3,15}, { 3,14}, { 2,13}, { 2,12}, { 2,11}, { 1,10}, { 0, 0}, { 0, 0}}}, // blue
        {{{ 4,15}, { 4,14}, { 3,13}, { 3,12}, { 3,11}, { 2,10}, { 0, 0}, { 0, 0}}}, // gray
        {{{ 6,15}, { 6,14}, { 5,13}, { 5,12}, { 4,11}, { 4,10}, { 3, 9}, { 0, 0}}}, // orange
        {{{ 7,15}, { 7,14}, { 6,13}, { 6,12}, { 5,11}, { 5,10}, { 4, 9}, { 0, 0}}}, // yellow
        {{{ 8,15}, { 8,14}, { 7,13}, { 7,12}, { 7,11}, { 6,10}, { 6, 9}, { 5, 8}}}, // lgreen
        {{{10,15}, { 9,14}, { 9,13}, { 8,12}, { 8,11}, { 7,10}, { 7, 9}, { 6, 8}}}, // pink
        {{{12,15}, {11,14}, {10,13}, {10,12}, { 9,11}, { 8,10}, { 0, 0}, { 0, 0}}}, // teal
        {{{14,15}, {13,14}, {12,13}, {11,12}, {10,11}, { 9,10}, { 8, 9}, { 7, 8}}}  // dblue
    }};

    for (int i = 0; i < 8; i++) {
        // 8 permutations of three booleans by extracting bits
        bool swap_coords = (i >> 0) & 1;
        bool negative_x = (i >> 1) & 1;
        bool negative_y = (i >> 2) & 1;

        for (const auto& line : lines) {
            // does line contain water?
            for (const auto& [x, y] : line) {
                if (x == 0 && y == 0) {
                    // stop at null values
                    break;
                }
                // does square contain water?

                pos.x = (swap_coords ? x : y) * (negative_x ? -1 : 1) * radius * 2;
                pos.y = (swap_coords ? y : x) * (negative_y ? -1 : 1) * radius * 2;
                // std::cout << "(" << x << ", " << y << ") >" << "(" << pos.x << ", " << pos.y << ")" << std::endl;

                if (noise.any_water_in_box(settings, precompute, BoxI32(pos, radius), sampling_distance)) {
                    score++; // 10 lines x 8 flips = 80 max score
                    break;
                }
            }
        }
    }

    return { false, score };
}
