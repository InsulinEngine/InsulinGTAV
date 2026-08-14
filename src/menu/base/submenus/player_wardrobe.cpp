#include "menu/base/submenus/player_wardrobe.h"
#include "menu/base/submenus/player_appearance.h"
#include "menu/base/options/button.h"
#include "menu/base/options/number.h"
#include "menu/base/options/break.h"
#include "menu/base/util/notify.h"
#include "rage/invoker/natives.h"

namespace {
    struct slot { const char* name; int id; };

    // Component slots. The ids are the game's, not indices into this table - the
    // labels are what the slots actually hold rather than their internal names.
    const slot k_components[] = {
        { "Face",        0 },
        { "Mask",        1 },
        { "Hair",        2 },
        { "Torso",       3 },
        { "Legs",        4 },
        { "Bag",         5 },
        { "Shoes",       6 },
        { "Scarf",       7 },
        { "Undershirt",  8 },
        { "Body Armor",  9 },
        { "Decal",      10 },
        { "Top",        11 },
    };
    constexpr int COMPONENT_COUNT = (int)(sizeof(k_components) / sizeof(k_components[0]));

    // Prop slots. 3, 4 and 5 exist in the enum but carry nothing on any model, so
    // showing them would just be four dead sliders.
    const slot k_props[] = {
        { "Hat",       0 },
        { "Glasses",   1 },
        { "Earpiece",  2 },
        { "Watch",     6 },
        { "Bracelet",  7 },
    };
    constexpr int PROP_COUNT = (int)(sizeof(k_props) / sizeof(k_props[0]));

    int g_comp_draw[COMPONENT_COUNT] = {};
    int g_comp_tex[COMPONENT_COUNT]  = {};
    int g_prop_draw[PROP_COUNT] = {};
    int g_prop_tex[PROP_COUNT]  = {};

    uint32_t g_bound_model = 0;   // model the mirrors above belong to

    Ped self_ped() { return native::get_player_ped(-1); }

    int clamp_i(int v, int lo, int hi) { return v < lo ? lo : (v > hi ? hi : v); }

    void apply_component(int i) {
        Ped ped = self_ped();
        if (!ped)
            return;
        int id = k_components[i].id;

        // Counts are per model and per slot, so clamp against the live value rather
        // than trusting whatever the slider was left on after a model change.
        int draws = native::get_number_of_ped_drawable_variations(ped, id);
        if (draws <= 0)
            return;
        g_comp_draw[i] = clamp_i(g_comp_draw[i], 0, draws - 1);

        int texs = native::get_number_of_ped_texture_variations(ped, id, g_comp_draw[i]);
        g_comp_tex[i] = clamp_i(g_comp_tex[i], 0, texs > 0 ? texs - 1 : 0);

        native::set_ped_component_variation(ped, id, g_comp_draw[i], g_comp_tex[i], 0);
    }

    void apply_prop(int i) {
        Ped ped = self_ped();
        if (!ped)
            return;
        int id = k_props[i].id;

        int draws = native::get_number_of_ped_prop_drawable_variations(ped, id);
        // -1 clears the slot, so the usable range starts one below zero.
        g_prop_draw[i] = clamp_i(g_prop_draw[i], -1, draws > 0 ? draws - 1 : -1);

        if (g_prop_draw[i] < 0) {
            native::clear_ped_prop(ped, id);
            return;
        }

        int texs = native::get_number_of_ped_prop_texture_variations(ped, id, g_prop_draw[i]);
        g_prop_tex[i] = clamp_i(g_prop_tex[i], 0, texs > 0 ? texs - 1 : 0);

        native::set_ped_prop_index(ped, id, g_prop_draw[i], g_prop_tex[i], true);
    }

    // Read what the ped is currently wearing into the mirrors.
    void read_all() {
        Ped ped = self_ped();
        if (!ped)
            return;

        for (int i = 0; i < COMPONENT_COUNT; i++) {
            g_comp_draw[i] = native::get_ped_drawable_variation(ped, k_components[i].id);
            g_comp_tex[i]  = native::get_ped_texture_variation(ped, k_components[i].id);
        }
        for (int i = 0; i < PROP_COUNT; i++) {
            g_prop_draw[i] = native::get_ped_prop_index(ped, k_props[i].id);
            g_prop_tex[i]  = 0;
        }
        g_bound_model = native::get_entity_model(ped);
    }
}

void player_wardrobe_menu::load() {
    set_name("Wardrobe");
    set_parent<player_appearance_menu>();

    add_option(button_option("Random Outfit")
        .add_click([] {
            native::set_ped_random_component_variation(self_ped(), 0);
            read_all();
            menu::notify::stacked("Wardrobe", "Randomised");
        }));

    add_option(button_option("Default Outfit")
        .add_click([] {
            native::set_ped_default_component_variation(self_ped());
            read_all();
            menu::notify::stacked("Wardrobe", "Reset");
        }));

    add_option(break_option("Clothing").ref());

    for (int i = 0; i < COMPONENT_COUNT; i++) {
        int idx = i;    // tiny capture: stl::function caps captures at 64 bytes

        add_option(number_option<int>(SCROLLSELECT, k_components[i].name)
            .add_number(g_comp_draw[i], "%i", 1)
            .add_min(0).add_max(255)
            .add_tooltip("Item. Out-of-range values snap back to what this model has.")
            .add_update([idx](number_option<int>*, int) { apply_component(idx); }));

        add_option(number_option<int>(SCROLLSELECT, "  Texture")
            .add_number(g_comp_tex[i], "%i", 1)
            .add_min(0).add_max(255)
            .add_tooltip("Colour variant of the item above")
            .add_update([idx](number_option<int>*, int) { apply_component(idx); }));
    }

    add_option(break_option("Props").ref());

    for (int i = 0; i < PROP_COUNT; i++) {
        int idx = i;

        add_option(number_option<int>(SCROLLSELECT, k_props[i].name)
            .add_number(g_prop_draw[i], "%i", 1)
            .add_min(-1).add_max(255)
            .add_tooltip("-1 removes the item")
            .add_update([idx](number_option<int>*, int) { apply_prop(idx); }));

        add_option(number_option<int>(SCROLLSELECT, "  Texture")
            .add_number(g_prop_tex[i], "%i", 1)
            .add_min(0).add_max(255)
            .add_update([idx](number_option<int>*, int) { apply_prop(idx); }));
    }
}

void player_wardrobe_menu::update() {
    // A model change invalidates every index, so re-read rather than leaving the
    // sliders showing the previous ped's outfit.
    Ped ped = self_ped();
    if (ped && native::get_entity_model(ped) != g_bound_model)
        read_all();
}

void player_wardrobe_menu::update_once() {
    read_all();
}

player_wardrobe_menu* player_wardrobe_menu::get() {
    static player_wardrobe_menu instance;
    return &instance;
}
