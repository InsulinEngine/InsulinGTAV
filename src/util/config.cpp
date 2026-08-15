#include "util/config.h"
#include "util/json.h"
#include "platform/paths.h"
#include <orbis/libkernel.h>


namespace util::config {
    // In-memory config document, loaded once and re-saved on each write.
    static tj::json g_root;

    static void build_path(stl::vector<stl::string>& path, stl::stack<stl::string> ns, stl::vector<stl::string>& additional) {
        while (!ns.empty()) { path.push_back(ns.top()); ns.pop(); }
        for (size_t i = 0; i < additional.size(); i++) path.push_back(additional[i]);
    }

    // Vivifying descend for writes.
    static tj::json* descend_write(stl::vector<stl::string>& path) {
        tj::json* node = &g_root;
        for (size_t i = 0; i < path.size(); i++) node = &(*node)[path[i].c_str()];
        return node;
    }

    // Non-vivifying descend for reads; nullptr if any segment is missing.
    static const tj::json* descend_read(stl::vector<stl::string>& path) {
        const tj::json* node = &g_root;
        for (size_t i = 0; i < path.size(); i++) {
            node = node->try_get(path[i].c_str());
            if (!node) return nullptr;
        }
        return node;
    }

    // Writes normally persist immediately. A batch defers that to the outermost
    // end_batch(), so a caller writing many keys at once (the theme's 31
    // colours) pays for one serialisation instead of 31.
    static int  g_batch_depth = 0;
    static bool g_batch_dirty = false;

    static void save() {
        if (g_batch_depth > 0) { g_batch_dirty = true; return; }
        g_root.save_to_file(OZARK_CONFIG, 2);
    }

    void config::load() {
        platform::ensure_data_dir();
        g_root = tj::json::load_from_file(OZARK_CONFIG);
    }

    void begin_batch() { g_batch_depth++; }

    void end_batch() {
        if (g_batch_depth > 0) g_batch_depth--;
        if (g_batch_depth == 0 && g_batch_dirty) {
            g_batch_dirty = false;
            g_root.save_to_file(OZARK_CONFIG, 2);
        }
    }

    // --- reads --------------------------------------------------------------
    stl::string config::read_string(stl::stack<stl::string> name_stack, stl::string key, stl::string default_string, stl::vector<stl::string> additional_stacks) {
        stl::vector<stl::string> path; build_path(path, name_stack, additional_stacks);
        const tj::json* node = descend_read(path);
        if (!node) return default_string;
        const tj::json* v = node->try_get(key.c_str());
        return (v && v->is_string()) ? stl::string(v->get_string()) : default_string;
    }

    int config::read_int(stl::stack<stl::string> name_stack, stl::string key, int default_int, stl::vector<stl::string> additional_stacks) {
        stl::vector<stl::string> path; build_path(path, name_stack, additional_stacks);
        const tj::json* node = descend_read(path);
        if (!node) return default_int;
        const tj::json* v = node->try_get(key.c_str());
        return (v && v->is_number()) ? (int)v->get_int() : default_int;
    }

    uint64_t config::read_uint64(stl::stack<stl::string> name_stack, stl::string key, uint64_t default_int, stl::vector<stl::string> additional_stacks) {
        stl::vector<stl::string> path; build_path(path, name_stack, additional_stacks);
        const tj::json* node = descend_read(path);
        if (!node) return default_int;
        const tj::json* v = node->try_get(key.c_str());
        return (v && v->is_number()) ? (uint64_t)v->get_int() : default_int;
    }

    float config::read_float(stl::stack<stl::string> name_stack, stl::string key, float default_float, stl::vector<stl::string> additional_stacks) {
        stl::vector<stl::string> path; build_path(path, name_stack, additional_stacks);
        const tj::json* node = descend_read(path);
        if (!node) return default_float;
        const tj::json* v = node->try_get(key.c_str());
        return (v && v->is_number()) ? (float)v->get_float() : default_float;
    }

    bool config::read_bool(stl::stack<stl::string> name_stack, stl::string key, bool default_bool, stl::vector<stl::string> additional_stacks) {
        stl::vector<stl::string> path; build_path(path, name_stack, additional_stacks);
        const tj::json* node = descend_read(path);
        if (!node) return default_bool;
        const tj::json* v = node->try_get(key.c_str());
        return (v && v->is_boolean()) ? v->get_bool() : default_bool;
    }

    bool config::read_color(stl::stack<stl::string> name_stack, stl::string key, color_rgba* color, stl::vector<stl::string> additional_stacks) {
        if (!color) return false;
        stl::vector<stl::string> path; build_path(path, name_stack, additional_stacks);
        const tj::json* node = descend_read(path);
        if (!node) return false;
        const tj::json* v = node->try_get(key.c_str());
        if (!v || !v->is_object()) return false;
        color->r = (int)v->value_int("r", color->r);
        color->g = (int)v->value_int("g", color->g);
        color->b = (int)v->value_int("b", color->b);
        color->a = (int)v->value_int("a", color->a);
        return true;
    }

    bool config::read_vector(stl::stack<stl::string> name_stack, stl::string key, math::vector3_<float>* vec, stl::vector<stl::string> additional_stacks) {
        if (!vec) return false;
        stl::vector<stl::string> path; build_path(path, name_stack, additional_stacks);
        const tj::json* node = descend_read(path);
        if (!node) return false;
        const tj::json* v = node->try_get(key.c_str());
        if (!v || !v->is_object()) return false;
        vec->x = (float)v->value_float("x", vec->x);
        vec->y = (float)v->value_float("y", vec->y);
        vec->z = (float)v->value_float("z", vec->z);
        return true;
    }

    // --- writes -------------------------------------------------------------
    void config::write_string(stl::stack<stl::string> name_stack, stl::string key, stl::string value, stl::vector<stl::string> additional_stacks) {
        stl::vector<stl::string> path; build_path(path, name_stack, additional_stacks);
        (*descend_write(path))[key.c_str()] = tj::json(value.c_str());
        save();
    }

    void config::write_int(stl::stack<stl::string> name_stack, stl::string key, int value, stl::vector<stl::string> additional_stacks) {
        stl::vector<stl::string> path; build_path(path, name_stack, additional_stacks);
        (*descend_write(path))[key.c_str()] = tj::json((long long)value);
        save();
    }

    void config::write_uint64(stl::stack<stl::string> name_stack, stl::string key, uint64_t value, stl::vector<stl::string> additional_stacks) {
        stl::vector<stl::string> path; build_path(path, name_stack, additional_stacks);
        (*descend_write(path))[key.c_str()] = tj::json((long long)value);
        save();
    }

    void config::write_float(stl::stack<stl::string> name_stack, stl::string key, float value, stl::vector<stl::string> additional_stacks) {
        stl::vector<stl::string> path; build_path(path, name_stack, additional_stacks);
        (*descend_write(path))[key.c_str()] = tj::json((double)value);
        save();
    }

    void config::write_bool(stl::stack<stl::string> name_stack, stl::string key, bool value, stl::vector<stl::string> additional_stacks) {
        stl::vector<stl::string> path; build_path(path, name_stack, additional_stacks);
        (*descend_write(path))[key.c_str()] = tj::json(value);
        save();
    }

    void config::write_color(stl::stack<stl::string> name_stack, stl::string key, color_rgba value, stl::vector<stl::string> additional_stacks) {
        stl::vector<stl::string> path; build_path(path, name_stack, additional_stacks);
        tj::json& obj = (*descend_write(path))[key.c_str()];
        obj["r"] = tj::json((long long)value.r);
        obj["g"] = tj::json((long long)value.g);
        obj["b"] = tj::json((long long)value.b);
        obj["a"] = tj::json((long long)value.a);
        save();
    }

    void config::write_vector(stl::stack<stl::string> name_stack, stl::string key, math::vector3_<float> value, stl::vector<stl::string> additional_stacks) {
        stl::vector<stl::string> path; build_path(path, name_stack, additional_stacks);
        tj::json& obj = (*descend_write(path))[key.c_str()];
        obj["x"] = tj::json((double)value.x);
        obj["y"] = tj::json((double)value.y);
        obj["z"] = tj::json((double)value.z);
        save();
    }

    config* get_config() {
        static config instance;
        return &instance;
    }
}
