#include "raytraced_audio_listener.h"

#include <cmath>
#include <godot_cpp/classes/audio_effect_reverb.hpp>
#include <godot_cpp/classes/audio_server.hpp>
#include <godot_cpp/classes/physics_ray_query_parameters3d.hpp>
#include "godot_cpp/classes/engine.hpp"
#include <godot_cpp/classes/world3d.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

void RaytracedAudioListener::_bind_methods() {
    ClassDB::bind_method(D_METHOD("set_rays_count", "count"), &RaytracedAudioListener::set_rays_count);
    ClassDB::bind_method(D_METHOD("get_rays_count"), &RaytracedAudioListener::get_rays_count);
    ClassDB::bind_method(D_METHOD("set_max_ray_distance", "distance"), &RaytracedAudioListener::set_max_ray_distance);
    ClassDB::bind_method(D_METHOD("get_max_ray_distance"), &RaytracedAudioListener::get_max_ray_distance);
    ClassDB::bind_method(D_METHOD("set_collision_mask", "mask"), &RaytracedAudioListener::set_collision_mask);
    ClassDB::bind_method(D_METHOD("get_collision_mask"), &RaytracedAudioListener::get_collision_mask);

    ADD_PROPERTY(PropertyInfo(Variant::INT, "rays_count"), "set_rays_count", "get_rays_count");
    ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "max_ray_distance"), "set_max_ray_distance", "get_max_ray_distance");
    ADD_PROPERTY(PropertyInfo(Variant::INT, "collision_mask", PROPERTY_HINT_LAYERS_3D_PHYSICS), "set_collision_mask", "get_collision_mask");
}

RaytracedAudioListener::RaytracedAudioListener() {}
RaytracedAudioListener::~RaytracedAudioListener() {}

void RaytracedAudioListener::_process(double delta) {
    if (Engine::get_singleton()->is_editor_hint()) return;

    time_since_update += delta;
    if (time_since_update >= update_interval) {
        time_since_update = 0.0;
        perform_raycasts();
    }
}

void RaytracedAudioListener::perform_raycasts() {
    Ref<World3D> world = get_world_3d();
    if (!world.is_valid()) return;

    PhysicsDirectSpaceState3D *space_state = world->get_direct_space_state();
    if (!space_state) return;

    Vector3 origin = get_global_position();
    double total_distance = 0.0;
    int hit_count = 0;

    // Use Fibonacci Sphere algorithm to distribute ray directions evenly
    double golden_ratio = (1.0 + std::sqrt(5.0)) / 2.0;
    for (int i = 0; i < rays_count; ++i) {
        double theta = 2.0 * Math_PI * i / golden_ratio;
        double phi = std::acos(1.0 - 2.0 * (i + 0.5) / rays_count);

        Vector3 direction(
            std::cos(theta) * std::sin(phi),
            std::sin(theta) * std::sin(phi),
            std::cos(phi)
        );

        Vector3 target = origin + direction * max_ray_distance;

        Ref<PhysicsRayQueryParameters3D> query = PhysicsRayQueryParameters3D::create(origin, target, collision_mask);
        Dictionary result = space_state->intersect_ray(query);

        if (!result.is_empty()) {
            Vector3 hit_position = result["position"];
            total_distance += origin.distance_to(hit_position);
            hit_count++;
        } else {
            total_distance += max_ray_distance;
        }
    }

    // Evaluate room profile based on structural containment
    double average_distance = total_distance / rays_count;
    double open_space_ratio = 1.0 - ((double)hit_count / rays_count);

    // Fetch and adjust the project's Reverb AudioEffect
    AudioServer *audio_server = AudioServer::get_singleton();
    int reverb_bus_idx = audio_server->get_bus_index(reverb_bus_name);

    if (reverb_bus_idx != -1) {
        // Find the Reverb effect inside the targeted audio bus
        for (int i = 0; i < audio_server->get_bus_effect_count(reverb_bus_idx); ++i) {
            Ref<AudioEffectReverb> reverb = audio_server->get_bus_effect(reverb_bus_idx, i);
            if (reverb.is_valid()) {
                // Larger average distance expands wet mix and room size dynamically
                float room_scale = CLAMP(average_distance / max_ray_distance, 0.0, 1.0);
                reverb->set_room_size(room_scale);
                reverb->set_wet(room_scale * (1.0f - open_space_ratio));
                break;
            }
        }
    }
}
