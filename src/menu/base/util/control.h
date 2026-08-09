#pragma once
#include "platform/stdafx.h"
#include "rage/types/base_types.h"
#include "stl/queue.h"

// Asset/entity request manager (the "control baustein"). Story-Mode trimmed:
// the async model/animation/particle/weapon request queues (what spawns need)
// are kept; request_control uses the single-player fast path (you own every
// entity), and the fiber-based simple_request_* blocking helpers are dropped.
namespace menu::control {
    struct request_control_context {
        Entity m_entity;
        stl::vector<stl::function<void(Entity)>> m_callbacks;
        stl::function<void(Entity, int)> m_callback_with_owner;
        int m_tries;
        bool m_take_owner;
    };

    struct request_model_context {
        uint32_t m_model;
        stl::function<void(uint32_t)> m_callback;
        int m_tries;
    };

    struct request_particle_fx_context {
        stl::pair<const char*, const char*> m_asset;
        stl::function<void(stl::pair<const char*, const char*>)> m_callback;
        int m_tries;
    };

    struct request_weapon_asset_context {
        uint32_t m_model;
        stl::function<void(uint32_t)> m_callback;
        int m_tries;
    };

    struct request_animation_context {
        stl::string m_animation;
        stl::function<void()> m_callback;
        int m_tries;
    };

    class control_manager {
    public:
        void update();

        void request_control(Entity entity, stl::function<void(Entity)> callback, bool take_owner = false);
        void request_control(Entity entity, stl::function<void(Entity, int)> callback, bool take_owner = false);
        void request_model(uint32_t model, stl::function<void(uint32_t)> callback);
        void request_animation(stl::string animation, stl::function<void()> callback);
        void request_particle(stl::pair<const char*, const char*> asset, stl::function<void(stl::pair<const char*, const char*>)> callback);
        void request_weapon(uint32_t weapon, stl::function<void(uint32_t)> callback);
    private:
        stl::vector<request_control_context> m_control;
        stl::queue<request_model_context> m_model;
        stl::queue<request_particle_fx_context> m_particle;
        stl::queue<request_weapon_asset_context> m_weapon;
        stl::queue<request_animation_context> m_animation;
    };

    control_manager* get_control_manager();

    inline void update() { get_control_manager()->update(); }
    inline void request_control(Entity entity, stl::function<void(Entity)> callback, bool take_owner = false) { get_control_manager()->request_control(entity, callback, take_owner); }
    inline void request_control(Entity entity, stl::function<void(Entity, int)> callback, bool take_owner = false) { get_control_manager()->request_control(entity, callback, take_owner); }
    inline void request_model(uint32_t model, stl::function<void(uint32_t)> callback) { get_control_manager()->request_model(model, callback); }
    inline void request_animation(stl::string animation, stl::function<void()> callback) { get_control_manager()->request_animation(animation, callback); }
    inline void request_particle(stl::pair<const char*, const char*> asset, stl::function<void(stl::pair<const char*, const char*>)> callback) { get_control_manager()->request_particle(asset, callback); }
    inline void request_weapon(uint32_t weapon, stl::function<void(uint32_t)> callback) { get_control_manager()->request_weapon(weapon, callback); }
}
