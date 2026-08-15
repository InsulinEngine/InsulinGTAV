#pragma once
#include <stdio.h>

// The mini-STL has no to_string. Panels format numbers every frame, so these
// take a caller-owned buffer rather than returning an stl::string (a 128-byte
// copy per call).
namespace util {
    inline const char* itos(int value, char* buf, int len) {
        snprintf(buf, len, "%d", value);
        return buf;
    }

    inline const char* ftos(float value, int decimals, char* buf, int len) {
        switch (decimals) {
            case 0:  snprintf(buf, len, "%.0f", value); break;
            case 1:  snprintf(buf, len, "%.1f", value); break;
            case 3:  snprintf(buf, len, "%.3f", value); break;
            default: snprintf(buf, len, "%.2f", value); break;
        }
        return buf;
    }
}
