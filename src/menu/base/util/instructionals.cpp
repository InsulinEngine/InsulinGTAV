#include "menu/base/util/instructionals.h"

namespace instructionals {
    instructionals* get_instructionals() {
        static instructionals instance;
        return &instance;
    }
}
