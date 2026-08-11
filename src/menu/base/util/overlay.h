#pragma once
#include "platform/stdafx.h"
#include "stl/vector.h"
#include "stl/string.h"
#include "stl/function.h"

// Centered input-capturing overlays (dropdown list + modal dialog), built on the
// same pattern as the HSV colour picker in menu_input: while active they freeze
// menu navigation (set_disable_input_this_frame) and read the D-pad/accept/cancel
// directly via disabled-control natives. update() is called every frame from
// menu::input::mi_update().
namespace menu::overlay {
    // Open a dropdown listing `items`; on Accept, calls on_pick(chosenIndex).
    void open_dropdown(stl::string title, stl::vector<stl::string> items, int current, stl::function<void(int)> on_pick);

    // Open a modal dialog (title + message) with Confirm / Cancel buttons.
    void open_modal(stl::string title, stl::string message, stl::function<void()> on_confirm, stl::function<void()> on_cancel);

    bool active();
    void update();
}
