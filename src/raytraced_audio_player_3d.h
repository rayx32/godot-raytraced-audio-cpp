#ifndef RAYTRACED_AUDIO_PLAYER_3D_H
#define RAYTRACED_AUDIO_PLAYER_3D_H

#include <godot_cpp/classes/audio_stream_player3d.hpp>
#include <godot_cpp/classes/physics_direct_space_state3d.hpp>

namespace godot {

class RaytracedAudioPlayer3D : public AudioStreamPlayer3D {
	GDCLASS(RaytracedAudioPlayer3D, AudioStreamPlayer3D);

private:
	Node3D *listener_node = nullptr;
	uint32_t collision_mask = 1;
	double check_interval = 0.05;
	double time_since_check = 0.0;

	void evaluate_occlusion();

protected:
	static void _bind_methods();

public:
	RaytracedAudioPlayer3D();
	~RaytracedAudioPlayer3D();

	void _ready() override;
	void _process(double delta) override;

	void set_collision_mask(uint32_t p_mask) { collision_mask = p_mask; }
	uint32_t get_collision_mask() const { return collision_mask; }
};

} // namespace godot

#endif // RAYTRACED_AUDIO_PLAYER_3D_H
