#include "video_stream_webm.h"
#include "video_stream_playback_webm.h"

#include <godot_cpp/classes/file_access.hpp>
#include <godot_cpp/classes/global_constants.hpp>
#include <godot_cpp/classes/ref.hpp>
#include <godot_cpp/classes/video_stream_playback.hpp>

Ref<VideoStreamPlayback> VideoStreamWebM::_instantiate_playback() {
	Ref<FileAccess> file = FileAccess::open(get_file(), FileAccess::READ);
	if (!file.is_valid()) {
		return Ref<VideoStreamPlayback>();
	}

	Ref<VideoStreamPlaybackWebM> playback;
	playback.instantiate();
	if (playback->load_from_file(file) != OK) {
		return nullptr;
	}

	return playback;
}
