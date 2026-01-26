#include "video_stream_webm.h"
#include "godot_cpp/classes/file_access.hpp"
#include "godot_cpp/classes/global_constants.hpp"
#include "godot_cpp/classes/image.hpp"
#include "godot_cpp/classes/image_texture.hpp"
#include "godot_cpp/classes/ref.hpp"
#include "godot_cpp/classes/texture2d.hpp"
#include "godot_cpp/classes/video_stream_playback.hpp"
#include "godot_cpp/core/print_string.hpp"
#include "godot_cpp/variant/array.hpp"
#include "godot_cpp/variant/color.hpp"
#include "godot_cpp/variant/string.hpp"
#include "godot_webm_callback.h"
#include "godot_webm_reader.h"

using namespace godot;

#include <mkvparser.hpp>
#include "webm/webm_parser.h"

Error VideoStreamPlaybackWebM::load_from_file(Ref<FileAccess> p_file) {
	if (p_file.is_null() || !p_file.is_valid()) {
		return ERR_FILE_CORRUPT;
	}

	file_info = {0};

	GodotWebMReader reader(p_file);
	GodotWebMCallback callback(&file_info);
	webm::WebmParser parser;
	webm::Status status = parser.Feed(&callback, &reader);
	if (!status.ok() && status.code != STATUS_FILE_ENDED_CORRECTLY) {
		Array values;
		values.push_back(status.code);
		print_error(String("WebM parsing error! Error: %d") % values);
		return ERR_PARSE_ERROR;
	}

	current_texture = ImageTexture::create_from_image(Image::create(
		file_info.videoWidth,
		file_info.videoHeight,
		false,
		Image::FORMAT_RGBA8
	));
	return OK;
}

void VideoStreamPlaybackWebM::_stop() {
	print_line("Stop video");
	is_playing = false;
	current_time = 0.0;
}

void VideoStreamPlaybackWebM::_play() {
	print_line("Play video");
	is_playing = true;
	current_time = 0.0;
}

bool VideoStreamPlaybackWebM::_is_playing() const {
	return is_playing;
}

void VideoStreamPlaybackWebM::_set_paused(bool p_paused) {
	Array arr;
	arr.push_back(p_paused);
	print_line(String("Set paused to %s") % arr);
	is_paused = p_paused;
}

bool VideoStreamPlaybackWebM::_is_paused() const {
	return is_paused;
}

double VideoStreamPlaybackWebM::_get_length() const {
	return file_info.getScaledSeconds(file_info.durationTimecode);
}

double VideoStreamPlaybackWebM::_get_playback_position() const {
	return current_time;
}

void VideoStreamPlaybackWebM::_seek(double p_time) {
	Array arr;
	arr.push_back(p_time);
	print_line(String("Seek to time %f") % arr);
	current_time = p_time;
}

void VideoStreamPlaybackWebM::_set_audio_track(int32_t p_idx) {
	audio_track = p_idx;
}

Ref<Texture2D> VideoStreamPlaybackWebM::_get_texture() const {
	return current_texture;
}

void VideoStreamPlaybackWebM::_update(double p_delta) {
	Array arr;
	arr.push_back(p_delta);
	current_time += p_delta;

	float amount = current_time / _get_length();
	Ref<Image> new_frame = Image::create(file_info.videoWidth, file_info.videoHeight, false, Image::FORMAT_RGBA8);
	new_frame->fill(Color {
		amount, amount, amount, 1.0f
	});
	current_texture->update(new_frame);

	if (current_time >= _get_length()) {
		is_playing = false;
		current_time = 0.0;
	}
}

int32_t VideoStreamPlaybackWebM::_get_channels() const {
	if (audio_track < 0 || audio_track > file_info.audioTracks.size() - 1) {
		return 0;
	}

	return file_info.audioTracks[audio_track].channels;
}

int32_t VideoStreamPlaybackWebM::_get_mix_rate() const {
	if (audio_track < 0 || audio_track > file_info.audioTracks.size() - 1) {
		return 0;
	}

	return file_info.audioTracks[audio_track].sampleRate;
}

Ref<VideoStreamPlayback> VideoStreamWebM::_instantiate_playback() {
	Ref<FileAccess> file = FileAccess::open(get_file(), FileAccess::READ);
	if (!file.is_valid()) {
		return Ref<VideoStreamPlayback>();
	}

	Ref<VideoStreamPlaybackWebM> playback;
	playback.instantiate();
	playback->load_from_file(file);
	return playback;
}
