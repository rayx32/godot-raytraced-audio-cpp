#include "raytraced_audio_player_3d.h"
#include "godot_cpp/classes/scene_tree.hpp"
#include "raytraced_audio_listener.h"

#include <godot_cpp/classes/audio_effect_low_pass_filter.hpp>
#include <godot_cpp/classes/audio_server.hpp>
#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/classes/physics_ray_query_parameters3d.hpp>
#include <godot_cpp/classes/world3d.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

void RaytracedAudioPlayer3D::_bind_methods() {
    ClassDB::bind_method(D_METHOD("set_collision_mask", "mask"), &RaytracedAudioPlayer3D::set_collision_mask);
    ClassDB::bind_method(D_METHOD("get_collision_mask"), &RaytracedAudioPlayer3D::get_collision_mask);

    ADD_PROPERTY(PropertyInfo(Variant::INT, "collision_mask", PROPERTY_HINT_LAYERS_3D_PHYSICS), "set_collision_mask", "get_collision_mask");
}

RaytracedAudioPlayer3D::RaytracedAudioPlayer3D() {}
RaytracedAudioPlayer3D::~RaytracedAudioPlayer3D() {}

void RaytracedAudioPlayer3D::_ready() {
    if (Engine::get_singleton()->is_editor_hint()) return;

    // Cache the RaytracedAudioListener instance in the current scene tree
    Node *root = get_tree()->get_current_scene();
    if (root) {
        TypedArray<Node> listeners = root->find_children("*", "RaytracedAudioListener", true, false);
        if (listeners.size() > 0) {
            listener_node = Object::cast_to<Node3D>(listeners[0]);
        }
    }
}

void RaytracedAudioPlayer3D::_process(double delta) {
    if (Engine::get_singleton()->is_editor_hint() || !listener_node) return;

    time_since_check += delta;
    if (time_since_check >= check_interval) {
        time_since_check = 0.0;
        evaluate_occlusion();
    }
}

void RaytracedAudioPlayer3D::evaluate_occlusion() {
    Ref<World3D> world = get_world_3d();
    if (!world.is_valid()) return;

    PhysicsDirectSpaceState3D *space_state = world->get_direct_space_state();
    if (!space_state) return;

    Vector3 origin = get_global_position();
    Vector3 target = listener_node->get_global_position();

    Ref<PhysicsRayQueryParameters3D> query = PhysicsRayQueryParameters3D::create(origin, target, collision_mask);
    Dictionary result = space_state->intersect_ray(query);

    // Dynamically throttle local playback stream properties based on ray obstacles
    if (!result.is_empty()) {
        // Path is blocked: Apply low pass muffle effect or decrease volume attenuation
        // Note: For custom effects per-stream, dynamically managing a runtime audio bus 
        // index allocated exclusively to this instance yields the cleanest result.
        this->set_attenuation_filter_cutoff_hz(1200.0); // Muffle
        this->set_attenuation_filter_db(-12.0);
    } else {
        // Direct line of sight
        this->set_attenuation_filter_cutoff_hz(20000.0); // Clear audio
        this->set_attenuation_filter_db(0.0);
    }
}