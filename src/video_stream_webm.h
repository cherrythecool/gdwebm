#pragma once

#include "godot_cpp/classes/file_access.hpp"
#include "godot_cpp/classes/image_texture.hpp"
#include "godot_cpp/classes/ref.hpp"
#include "godot_webm_callback.h"
#include <cstdint>
#include <godot_cpp/classes/video_stream.hpp>
#include <godot_cpp/classes/video_stream_playback.hpp>
#include <godot_cpp/classes/wrapped.hpp>
#include <godot_cpp/classes/texture2d.hpp>

using namespace godot;

class VideoStreamPlaybackWebM : public VideoStreamPlayback {
	GDCLASS(VideoStreamPlaybackWebM, VideoStreamPlayback);

private:
	GodotWebMFileInfo file_info;
	int32_t audio_track = 0;
	int32_t video_track = 0;
	bool is_playing = false;
	bool is_paused = false;
	double current_time = 0.0;
	Ref<ImageTexture> current_texture;

	int64_t last_audio_index = -1;

	int32_t get_audio_track() const;
	uint64_t get_audio_track_count() const;
	bool has_audio_track() const;

	int32_t get_video_track() const;
	uint64_t get_video_track_count() const;
	bool has_video_track() const;

protected:
	static void _bind_methods() {};

public:
	Error load_from_file(Ref<FileAccess> p_file);

	void _stop() override;
	void _play() override;
	bool _is_playing() const override;
	void _set_paused(bool p_paused) override;
	bool _is_paused() const override;
	double _get_length() const override;
	double _get_playback_position() const override;
	void _seek(double p_time) override;
	void _set_audio_track(int32_t p_idx) override;
	Ref<Texture2D> _get_texture() const override;
	void _update(double p_delta) override;
	int32_t _get_channels() const override;
	int32_t _get_mix_rate() const override;
};

class VideoStreamWebM : public VideoStream {
	GDCLASS(VideoStreamWebM, VideoStream);

protected:
	static void _bind_methods() {};

public:
	virtual Ref<VideoStreamPlayback> _instantiate_playback() override;
};
