#pragma once

#include <godot_cpp/classes/ref.hpp>
#include <godot_cpp/classes/video_stream.hpp>
#include <godot_cpp/classes/video_stream_playback.hpp>
#include <godot_cpp/classes/wrapped.hpp>

using namespace godot;

class VideoStreamWebM : public VideoStream {
	GDCLASS(VideoStreamWebM, VideoStream);

protected:
	static void _bind_methods();

public:
	virtual Ref<VideoStreamPlayback> _instantiate_playback() override;

	static Ref<VideoStreamWebM> load_from_file(const String& path);
};
