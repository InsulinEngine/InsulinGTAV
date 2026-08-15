#pragma once

// DIAGNOSTICS - remove once the phone-background artefact is closed.
//
// One flag per drawn part of the menu so the artefact can be bisected live on
// console, instead of one flash per guess. All default to true (draw normally);
// switching one off skips exactly that draw and nothing else.
//
// Only DRAW statements are gated, never the state around them - m_smooth_scroll,
// the option count and render_count are computed either way, because the footer
// and the counter are positioned from them. A gate that swallowed those would
// move the surviving parts and make the bisect lie.
namespace menu::parts {
    extern bool g_header;
    extern bool g_background;
    extern bool g_scroller;
    extern bool g_footer;
    extern bool g_scrollbar;
    extern bool g_counter;
    extern bool g_instructionals;
    extern bool g_panels;
}
