#pragma once
#include "menu/base/submenu.h"

// Live editor for the CHandlingData of the vehicle you are sitting in.
//
// Field offsets are the ones verified on-console against handling.meta (27/27 fields
// matched on two vehicles); see docs. Two things this editor has to get right and most
// hand-rolled handling menus do not:
//
//   * handling.meta and memory use DIFFERENT UNITS. Drag is meta/10000, top speed is
//     km/h -> m/s, steering lock and lateral traction are degrees -> radians. The
//     editor shows the meta-style value and converts on write.
//   * several fields are DERIVED. fTractionCurveMax has a reciprocal twin, and the
//     max/min pair feeds a third. Writing only the base value leaves the grip curve
//     inconsistent, so every write updates the whole family.
//
// CHandlingData is shared by all vehicles of a model, so edits affect every instance.
class handling_editor_menu : public menu::submenu::submenu {
public:
    void load() override;
    void update() override;
    void update_once() override;
    static handling_editor_menu* get();
};
