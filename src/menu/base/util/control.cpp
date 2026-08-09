#include "control.h"
#include "rage/invoker/natives.h"

// Story-Mode control manager. The network-ownership machinery (net objects,
// migration, owner index) from the PC source is omitted: request_control's
// single-player fast path fires the callback immediately (you own every entity
// in Story Mode), so the m_control queue only fills if ever run in a session.
namespace menu::control {
    void control_manager::update() {
        if (m_control.size()) {
            request_control_context& request = m_control[0];

            if (native::does_entity_exist(request.m_entity)) {
                if (native::network_has_control_of_entity(request.m_entity) || request.m_tries > 50) {
                    if (request.m_tries < 50) {
                        for (size_t i = 0; i < request.m_callbacks.size(); i++)
                            if (request.m_callbacks[i]) request.m_callbacks[i](request.m_entity);
                        if (request.m_callback_with_owner) request.m_callback_with_owner(request.m_entity, 0);
                    }
                    m_control.erase(m_control.begin());
                } else {
                    request.m_tries++;
                    native::network_request_control_of_entity(request.m_entity);
                }
            } else {
                m_control.erase(m_control.begin());
            }
        }

        if (m_model.size()) {
            request_model_context& request = m_model.front();
            if (native::has_model_loaded(request.m_model) || request.m_tries > 30) {
                if (request.m_tries < 30) request.m_callback(request.m_model);
                m_model.pop();
            } else {
                request.m_tries++;
                native::request_model(request.m_model);
            }
        }

        if (m_particle.size()) {
            request_particle_fx_context& request = m_particle.front();
            if (native::has_named_ptfx_asset_loaded(request.m_asset.first) || request.m_tries > 30) {
                if (request.m_tries < 30) request.m_callback(request.m_asset);
                m_particle.pop();
            } else {
                request.m_tries++;
                native::request_named_ptfx_asset(request.m_asset.first);
            }
        }

        if (m_weapon.size()) {
            request_weapon_asset_context& request = m_weapon.front();
            if (native::has_weapon_asset_loaded(request.m_model) || request.m_tries > 30) {
                if (request.m_tries < 30) request.m_callback(request.m_model);
                m_weapon.pop();
            } else {
                request.m_tries++;
                native::request_weapon_asset(request.m_model, 31, 0);
            }
        }

        if (m_animation.size()) {
            request_animation_context& request = m_animation.front();
            if (native::has_anim_dict_loaded(request.m_animation.c_str()) || request.m_tries > 30) {
                if (request.m_tries < 30) request.m_callback();
                m_animation.pop();
            } else {
                request.m_tries++;
                native::request_anim_dict(request.m_animation.c_str());
            }
        }
    }

    void control_manager::request_control(Entity entity, stl::function<void(Entity)> callback, bool take_owner) {
        if (!native::network_is_in_session()) {
            callback(entity);   // Story Mode: you already own the entity
        } else {
            request_control_context ctx;
            ctx.m_entity = entity;
            ctx.m_callbacks.push_back(callback);
            ctx.m_tries = 0;
            ctx.m_take_owner = take_owner;
            m_control.push_back(ctx);
        }
    }

    void control_manager::request_control(Entity entity, stl::function<void(Entity, int)> callback, bool take_owner) {
        request_control_context ctx;
        ctx.m_entity = entity;
        ctx.m_callback_with_owner = callback;
        ctx.m_tries = 0;
        ctx.m_take_owner = take_owner;
        m_control.push_back(ctx);
    }

    void control_manager::request_model(uint32_t model, stl::function<void(uint32_t)> callback) {
        request_model_context ctx; ctx.m_model = model; ctx.m_callback = callback; ctx.m_tries = 0;
        m_model.push(ctx);
    }

    void control_manager::request_animation(stl::string animation, stl::function<void()> callback) {
        request_animation_context ctx; ctx.m_animation = animation; ctx.m_callback = callback; ctx.m_tries = 0;
        m_animation.push(ctx);
    }

    void control_manager::request_particle(stl::pair<const char*, const char*> asset, stl::function<void(stl::pair<const char*, const char*>)> callback) {
        request_particle_fx_context ctx; ctx.m_asset = asset; ctx.m_callback = callback; ctx.m_tries = 0;
        m_particle.push(ctx);
    }

    void control_manager::request_weapon(uint32_t weapon, stl::function<void(uint32_t)> callback) {
        request_weapon_asset_context ctx; ctx.m_model = weapon; ctx.m_callback = callback; ctx.m_tries = 0;
        m_weapon.push(ctx);
    }

    control_manager* get_control_manager() {
        static control_manager instance;
        return &instance;
    }
}
