#ifndef RAYTRACED_AUDIO_LISTENER_H
#define RAYTRACED_AUDIO_LISTENER_H

#include <godot_cpp/classes/node3d.hpp>
#include <godot_cpp/classes/physics_direct_space_state3d.hpp>
#include <godot_cpp/variant/string_name.hpp>

namespace godot {

class RaytracedAudioListener : public Node3D {
	GDCLASS(RaytracedAudioListener, Node3D);

private:
	int rays_count = 32;
	double max_ray_distance = 50.0;
	double update_interval = 0.1; // Seconds between raycast sweeps
	double time_since_update = 0.0;
	uint32_t collision_mask = 1;

	StringName reverb_bus_name = "Reverb";
	StringName ambient_bus_name = "Ambient";

	void perform_raycasts();

protected:
	static void _bind_methods();

public:
	RaytracedAudioListener();
	~RaytracedAudioListener();

	void _process(double delta) override;

	// Getters / Setters for Godot properties
	void set_rays_count(int p_count) { rays_count = p_count; }
	int get_rays_count() const { return rays_count; }

	void set_max_ray_distance(double p_dist) { max_ray_distance = p_dist; }
	double get_max_ray_distance() const { return max_ray_distance; }

	void set_collision_mask(uint32_t p_mask) { collision_mask = p_mask; }
	uint32_t get_collision_mask() const { return collision_mask; }
};

} // namespace godot

#endif // RAYTRACED_AUDIO_LISTENER_H