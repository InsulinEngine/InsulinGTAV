#include "menu/base/util/textures.h"

namespace menu::textures {
    textures* get_textures() {
        static textures instance;
        return &instance;
    }
}
