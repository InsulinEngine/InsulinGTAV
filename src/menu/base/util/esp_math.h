#pragma once

// Pure ESP geometry: no engine types, no natives, no project headers. Kept
// separate from esp.cpp precisely so it can be compiled and tested on the host,
// where a wrong ratio costs a second instead of a deploy cycle.
namespace menu::esp_math {

    // A screen-space rectangle in the renderer's convention: x,y is the
    // TOP-LEFT corner (menu::renderer::draw_rect adds half the scale itself).
    struct box2d { float x, y, w, h; };

    // 2take1's recipe: project a point below the entity's origin and one above
    // it, and the distance between them on screen is the box height. A human is
    // roughly four times taller than wide, and deriving width from the measured
    // height is what keeps the box correct at every distance without a second
    // projection pair.
    inline box2d box_from_projections(float head_x, float head_y, float feet_y) {
        float h = head_y - feet_y;
        if (h < 0.f) h = -h;
        float w = h * 0.25f;
        float top = head_y < feet_y ? head_y : feet_y;
        box2d b;
        b.w = w;
        b.h = h;
        b.x = head_x - (w * 0.5f);
        b.y = top;
        return b;
    }

    // Health and armour share one bar, as in 2take1: the denominator is max
    // health plus the 50 armour GTA V allows, so a fully armoured player reads
    // as full rather than as 200%.
    inline float health_fraction(int health, int max_health, int armour) {
        if (max_health <= 0) return 0.f;
        if (health < 0) health = 0;
        if (armour < 0) armour = 0;
        float total = (float)max_health + 50.f;
        float have  = (float)health + (float)armour;
        float f = have / total;
        if (f < 0.f) f = 0.f;
        if (f > 1.f) f = 1.f;
        return f;
    }

    // Linear falloff between a near and a far scale. Anything at or beyond
    // max_distance reads at far_scale rather than continuing to shrink, so a
    // distant target stays legible instead of becoming a smudge.
    inline float distance_scale(float distance, float max_distance,
                                float near_scale, float far_scale) {
        if (max_distance <= 0.f) return far_scale;
        float ratio = distance / max_distance;
        if (ratio < 0.f) ratio = 0.f;
        if (ratio > 1.f) ratio = 1.f;
        return near_scale + (far_scale - near_scale) * ratio;
    }
}
