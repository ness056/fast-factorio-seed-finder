#include "stages.hpp"

int main(int argc, char* argv[]) {
    // Maps that have a starting peninsula surrounded by water

    MapGenSettings settings;

    // iron, copper, coal, stone, oil.
    settings.frequencies = { 0.f, 0.f, 0.f, 0.f, 0.f };
    settings.sizes = { 0.f, 0.f, 0.f, 0.f, 0.f };
    settings.biter_frequency = 0.f;
    settings.biter_size = 0.f;

    Finder<SeedCache> finder(settings);

    finder.set_water_scales({ 1.f });
    finder.set_water_coverages({ 1.f });

    finder.set_elevation_types({ ELEVATION_2_0 });

    finder.add_stage(stage3_eval, stage3_settings);

    return finder.run("peninsula", argc, argv);
}