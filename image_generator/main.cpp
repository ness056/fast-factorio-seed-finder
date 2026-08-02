#define _USE_MATH_DEFINES
#include "noise.hpp"
#include <chrono>
#include <print>
#include <iostream>
#include <fstream>
#include <filesystem>
#include <argparse/argparse.hpp>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

namespace fs = std::filesystem;

int main(int argc, char* argv[]) {
    argparse::ArgumentParser program("image_generator");

    program.add_argument("-o", "--output")
        .help("The path to the output file.")
        .required()
        .metavar("PATH");

    program.add_argument("--seed")
        .help("The seed to generate")
        .scan<'i', uint32_t>()
        .metavar("UINT32");

    program.add_argument("--water-scale")
        .help("The water scale to use.")
        .default_value(1.f)
        .scan<'f', float>()
        .metavar("FLOAT");

    program.add_argument("--water-coverage")
        .help("The water coverage to use.")
        .default_value(1.f)
        .scan<'f', float>()
        .metavar("FLOAT");

    program.add_argument("--elevation-type")
        .help("The elevation type to use. Can be '2.0', '1.1' or 'island'.")
        .default_value(std::string("2.0"))
        .metavar("STRING");

    program.add_argument("--image-size")
        .help("Size of the preview")
        .default_value(1000)
        .scan<'i', int>()
        .metavar("INT");

    try {
        program.parse_args(argc, argv);
    }
    catch (const std::exception& err) {
        std::cerr << err.what() << std::endl;
        std::cerr << program;
        return 1;
    }

    MapGenSettings settings;
    for (ResourceType t = IRON; t < NB_RESOURCE_TYPE; ++t) {
        settings.frequencies[t] = 6.f;
        settings.sizes[t] = 6.f;
        settings.richness[t] = 6.f;
    }
    settings.biter_frequency = 0.f;
    settings.biter_size = 0.f;

    auto out_path = program.get<std::string>("-o");
    uint32_t seed0 = program.get<uint32_t>("--seed");
    settings.water_scale = program.get<float>("--water-scale");
    settings.water_coverage = program.get<float>("--water-coverage");
    int img_size = program.get<int>("--image-size");

    std::string elevation_type_str = program.get<std::string>("--elevation-type");
    if (elevation_type_str == "2.0") {
        settings.elevation_type = ELEVATION_2_0;
    } else if (elevation_type_str == "1.1") {
        settings.elevation_type = ELEVATION_1_1;
    } else if (elevation_type_str == "island") {
        settings.elevation_type = ELEVATION_ISLAND;
    } else {
        std::println("elevation-type has to be '2.0', '1.1' or 'island'.");
        return 1;
    }

    fs::path parent = fs::path(out_path).parent_path();
    if (!parent.empty()) {
        std::error_code ec;
        if (!fs::create_directories(parent, ec) && ec) {
            std::println(stderr, "Couldn't create parent directory {}\n{}", parent.string(), ec.message());
            return 1;
        }
    }

    NoisePrecompute precompute(settings);
    Noise noise(seed0, true, true);
    NoiseCache cache;

    auto regular = regular_patches(precompute, cache, seed0, { 0, 0 });
    auto starter = starter_patches(settings, precompute, noise, cache, seed0);
    std::array<PatchArray, 9> biters;
    if (settings.biter_frequency > 0 && settings.biter_size > 0) {
        for (int i = 0; i < (int)(biters.size()); i++) {
            biters[i] = enemy_bases(settings, precompute, seed0, { i % 3 - 1, i / 3 - 1 });
        }
    }

    // Output buffer: RGB bytes
    std::vector<unsigned char> img(img_size * img_size * 3, 255);

    const int shift = img_size / 2 ;

    for (int py = 0; py < img_size; ++py) {
        for (int px = 0; px < img_size; ++px) {
            // Map pixel -> world coordinates used by generate_candidates
            float wx = (float)(px - shift);
            float wy = (float)(py - shift);
            size_t idx = (py * img_size + px) * 3;
            size_t biter_idx = ((size_t)wx + 256 + 512) / 512 + ((size_t)wy + 256 + 512) / 512 * 3;

            if (noise.is_tile_water(settings, precompute, { wx, wy })) {
                img[idx+0] = 51; img[idx+1] = 83; img[idx+2] = 95;
                continue;
            }

            float best_val = -std::numeric_limits<float>::infinity();
            ResourceType best_type = NB_RESOURCE_TYPE;

            auto check_patches = [&](const Patches& patches) {
                for (ResourceType type = IRON; type < NB_RESOURCE_TYPE; ++type) {
                    float best_for_type = -std::numeric_limits<float>::infinity();
                    for (auto it = patches[type].begin(); it != patches[type].end(); ++it) {
                        const Patch &p = *it;
                        float dist = PositionF32::distance({ wx, wy }, p.pos);
    
                        float slope = 0.f;
                        if (p.radius > 0.f) slope = 3.f * p.quantity / float(M_PI * p.radius * p.radius * p.radius);
    
                        float val = (p.radius - dist) * slope;
                        if (val > best_for_type) best_for_type = val;
                    }
    
                    if (best_for_type > best_val) {
                        best_val = best_for_type;
                        best_type = type;
                    }
                }
            };

            check_patches(starter);
            check_patches(regular);

            if (best_val <= 0.f || best_type == NB_RESOURCE_TYPE) {
                img[idx+0] = 55; img[idx+1] = 53; img[idx+2] = 11;
            } else {
                switch (best_type) {
                    case IRON:   img[idx+0]=105; img[idx+1]=133; img[idx+2]=147; break; // blue
                    case COPPER: img[idx+0]=204; img[idx+1]=98;  img[idx+2]=54;  break; // red
                    case COAL:   img[idx+0]=0;   img[idx+1]=0;   img[idx+2]=0;   break; // black
                    case STONE:  img[idx+0]=175; img[idx+1]=155; img[idx+2]=108; break; // gray
                    case OIL:    img[idx+0]=198; img[idx+1]=51;  img[idx+2]=196; break; // purple
                    default:     img[idx+0]=255; img[idx+1]=255; img[idx+2]=255; break;
                }
            }

            if (settings.biter_frequency > 0 && settings.biter_size > 0 && biter_idx < biters.size()) {
                for (const auto& p : biters[biter_idx]) {
                    float dist = PositionF32::distance({ wx, wy }, p.pos);

                    float slope = 0.f;
                    if (p.radius > 0.f) slope = 3.f * p.quantity / float(M_PI * p.radius * p.radius * p.radius);

                    float val = (p.radius - dist) * slope;
                    if (val > 0) {
                        img[idx+0] = 255; img[idx+1] = 25; img[idx+2] = 25;
                    }
                }
            }
        }
    }

    stbi_write_png(out_path.c_str(), img_size, img_size, 3, img.data(), 3*img_size*sizeof(unsigned char));
    std::println("Created {}", out_path);

    return 0;
}