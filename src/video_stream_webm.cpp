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
#include "godot_cpp/variant/packed_float32_array.hpp"
#include "godot_cpp/variant/string.hpp"
#include "godot_webm_callback.h"
#include "godot_webm_reader.h"

using namespace godot;

#include <mkvparser.hpp>
#include "webm/webm_parser.h"

#include "opus.h"

Error VideoStreamPlaybackWebM::load_from_file(Ref<FileAccess> p_file) {
	if (p_file.is_null() || !p_file.is_valid()) {
		return ERR_FILE_CORRUPT;
	}

	file_info = {0};

	GodotWebMReader reader(p_file);
	GodotWebMCallback callback(&file_info);
	webm::WebmParser parser;
	webm::Status status = parser.Feed(&callback, &reader);
	if (!status.completed_ok() && status.code != STATUS_FILE_ENDED_CORRECTLY) {
		Array values;
		values.push_back(status.code);
		print_error(String("WebM parsing error! Error: %d") % values);
		return ERR_PARSE_ERROR;
	}

	GodotWebMTrack current_track = file_info.tracks[get_video_track()];
	webm::Video current_video = current_track.entry.video.value();
	current_texture = ImageTexture::create_from_image(Image::create(
		current_video.pixel_width.value(),
		current_video.pixel_height.value(),
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
	if (!is_playing) {
		return;
	}

	Array arr;
	arr.push_back(p_delta);
	current_time += p_delta;

	if (has_video_track()) {
		GodotWebMTrack current_track = file_info.tracks[get_video_track()];
		webm::Video current_video = current_track.entry.video.value();

		bool found = false;
		uint64_t frame = 0;
		for (uint64_t i = 0; i < current_track.frames.size(); i++) {
			GodotWebMFrame current_frame = current_track.frames[i];
			if (current_time >= file_info.getScaledSeconds(current_frame.timecode)) {
				frame = i;
				found = true;
			} else {
				break;
			}
		}

		GodotWebMFrame video_frame;
		if (frame > 0 && frame <= current_track.frames.size() - 1) {
			video_frame = current_track.frames[frame];
		} else {
			found = false;
		}

		float amount = current_time / _get_length();
		if (found) {
			amount = (float)video_frame.data[0] / 255.0f;
		}

		Ref<Image> new_frame = Image::create(
			current_video.pixel_width.value(),
			current_video.pixel_height.value(),
			false,
			Image::FORMAT_RGBA8
		);
		new_frame->fill(Color {
			amount, amount, amount, 1.0f
		});
		current_texture->update(new_frame);
	}

	if (has_audio_track()) {
		GodotWebMTrack current_track = file_info.tracks[get_audio_track()];
		webm::Audio current_audio = current_track.entry.audio.value();

		bool found = false;
		int64_t frame = 0;
		for (int64_t i = 0; i < current_track.frames.size(); i++) {
			GodotWebMFrame current_frame = current_track.frames[i];
			if (current_time >= file_info.getScaledSeconds(current_frame.timecode)) {
				frame = i;
				found = true;
			} else {
				break;
			}
		}

		if (last_audio_index != frame) {
			if (last_audio_index == -1) {
				last_audio_index = 0;
			}

			switch (current_track.codec) {
				case GDWEBM_SUPPORTED_CODEC_A_OPUS: {
					GodotWebMOpusData* data = (GodotWebMOpusData*)current_track.data;
					OpusDecoder* decoder = data->decoder;
					int decoded_frames = 0;
					for (int64_t i = last_audio_index; i < frame; i++) {
						int current_decoded = opus_decode_float(decoder,
							current_track.frames[i].data.ptr(),
							current_track.frames[i].data.size(),
							data->pcm.ptrw() + decoded_frames,
							(data->pcm.size() / data->channels) - decoded_frames,
							0
						);
						decoded_frames += current_decoded;
						data->pcm.resize(data->pcm.size() + (decoded_frames * data->channels));

						if (current_decoded < 0) {
							godot::print_error(current_decoded);
							break;
						}
					}

					if (decoded_frames > 0) {
						int mixed = mix_audio(decoded_frames, data->pcm, 0);
						if (mixed == -1) {
							godot::print_error("Failed to mix audio frames (", decoded_frames, " decoded)");
						}
					}
					break;
				} default:
					break;
			}

			last_audio_index = frame;
		}
	}

	if (current_time >= _get_length()) {
		is_playing = false;
		current_time = 0.0;
	}
}

int32_t VideoStreamPlaybackWebM::_get_channels() const {
	if (get_audio_track() == -1) {
		return 0;
	}

	return file_info.tracks[get_audio_track()].entry.audio.value().channels.value();
}

int32_t VideoStreamPlaybackWebM::_get_mix_rate() const {
	if (get_audio_track() == -1) {
		return 0;
	}

	return file_info.tracks[get_audio_track()].entry.audio.value().sampling_frequency.value();
}

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

uint64_t VideoStreamPlaybackWebM::get_audio_track_count() const {
	uint64_t count = 0;
	for (uint64_t i = 0; i < file_info.tracks.size(); i++) {
		if (file_info.tracks.get_array()[i].value.entry.audio.is_present()) {
			count++;
		}
	}

	return count;
}

int32_t VideoStreamPlaybackWebM::get_audio_track() const {
	int32_t returned_key = -1;
	int32_t counter = 0;
	for (uint64_t i = 0; i < file_info.tracks.size(); i++) {
		bool has_audio = file_info.tracks.get_array()[i].value.entry.audio.is_present();
		if (!has_audio) {
			continue;
		}

		if (counter == audio_track) {
			godot::VMap<int64_t, GodotWebMTrack>::Pair pair = file_info.tracks.get_array()[i];
			returned_key = pair.key;
			break;
		}

		counter++;
	}

	return returned_key;
}

bool VideoStreamPlaybackWebM::has_audio_track() const {
	return get_audio_track_count() > 0;
}

uint64_t VideoStreamPlaybackWebM::get_video_track_count() const {
	uint64_t count = 0;
	for (uint64_t i = 0; i < file_info.tracks.size(); i++) {
		if (file_info.tracks.get_array()[i].value.entry.video.is_present()) {
			count++;
		}
	}

	return count;
}

int32_t VideoStreamPlaybackWebM::get_video_track() const {
	int32_t number = 0;
	for (uint64_t i = 0; i < file_info.tracks.size(); i++) {
		bool has_video = file_info.tracks.get_array()[i].value.entry.video.is_present();
		if (!has_video) {
			continue;
		}

		if (number == video_track) {
			godot::VMap<int64_t, GodotWebMTrack>::Pair pair = file_info.tracks.get_array()[i];
			number = pair.key;
			break;
		}

		number++;
	}

	return number;
}

bool VideoStreamPlaybackWebM::has_video_track() const {
	return get_video_track_count() > 0;
}
