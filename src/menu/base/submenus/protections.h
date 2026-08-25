#pragma once
#include "menu/base/submenu.h"

// One row per protection filter, each a three-way Off / Log / Enforce.
//
// New filters default to Log: they run the detection and report it, then chain
// to the original anyway. Enforce is a separate, deliberate flip per filter once
// the log shows the detection firing only when it should.
//
// feature_update() is kept for a second, unrelated group: three local
// state-keepers (anti-fire, anti-ragdoll, explosion proof) that hold the
// player out of a state an attack tries to put them into. They are not
// network filters - nothing here detects or blocks an event - so they are
// rendered separately, below the filter list. See protections.cpp.
class protections_menu : public menu::submenu::submenu {
public:
    void load() override;
    void update() override;
    void feature_update() override;
    static protections_menu* get();
};
