#include "video_stream_playback_webm.h"

#include "dav1d/data.h"
#include "dav1d/headers.h"
#include "dav1d/picture.h"
#include "godot_cpp/classes/file_access.hpp"
#include "godot_cpp/classes/global_constants.hpp"
#include "godot_cpp/classes/image.hpp"
#include "godot_cpp/classes/image_texture.hpp"
#include "godot_cpp/classes/ref.hpp"
#include "godot_cpp/classes/texture2d.hpp"
#include "godot_cpp/classes/video_stream_playback.hpp"
#include "godot_cpp/core/print_string.hpp"
#include "godot_cpp/variant/packed_byte_array.hpp"
#include "godot_cpp/variant/packed_float32_array.hpp"
#include "godot_cpp/variant/string.hpp"
#include "godot_webm_callback.h"
#include "godot_webm_reader.h"

using namespace godot;

#include "webm/webm_parser.h"
#include <mkvparser.hpp>

#include "opus.h"

#include "dav1d/dav1d.h"

#include "yuv2rgb.h"

Error VideoStreamPlaybackWebM::load_from_file(Ref<FileAccess> p_file) {
	if (p_file.is_null() || !p_file.is_valid()) {
		return ERR_FILE_CORRUPT;
	}

	current_file = p_file;
	file_info = { 0 };

	GodotWebMReader reader(current_file);
	GodotWebMCallback callback(&file_info);
	webm::WebmParser parser;
	webm::Status status = parser.Feed(&callback, &reader);
	if (!status.completed_ok() && status.code != STATUS_FILE_ENDED_CORRECTLY) {
		print_error("WebM parsing error! Error: ", status.code);
		return ERR_PARSE_ERROR;
	}

	GodotWebMTrack current_track = file_info.tracks[get_video_track()];
	webm::Video current_video = current_track.entry.video.value();
	current_image = Image::create(
			current_video.pixel_width.value(),
			current_video.pixel_height.value(),
			false,
			Image::FORMAT_RGBA8);

	current_texture = ImageTexture::create_from_image(current_image);
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
	last_video_index = -1;
	last_sent_index = -1;
	last_audio_index = -1;
}

bool VideoStreamPlaybackWebM::_is_playing() const {
	return is_playing;
}

void VideoStreamPlaybackWebM::_set_paused(bool p_paused) {
	print_line("Set paused to ", p_paused);
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
	print_line("Seek to time ", p_time);
	current_time = p_time;

	// TODO: make this work goodie
	if (has_video_track()) {
		GodotWebMTrack current_track = file_info.tracks[get_video_track()];
		for (int64_t i = 0; i < (int64_t)current_track.frames.size(); i++) {
			GodotWebMFrame current_frame = current_track.frames[i];
			if (current_time >= file_info.getScaledSeconds(current_frame.timecode)) {
				last_video_index = i;
				last_sent_index = -1;
			} else {
				break;
			}
		}
	}

	if (has_audio_track()) {
		GodotWebMTrack current_track = file_info.tracks[get_audio_track()];
		for (int64_t i = 0; i < (int64_t)current_track.frames.size(); i++) {
			GodotWebMFrame current_frame = current_track.frames[i];
			if (current_time >= file_info.getScaledSeconds(current_frame.timecode)) {
				last_audio_index = i;
			} else {
				break;
			}
		}
	}

}

void VideoStreamPlaybackWebM::_set_audio_track(int32_t p_idx) {
	audio_track = p_idx;
}

Ref<Texture2D> VideoStreamPlaybackWebM::_get_texture() const {
	return current_texture;
}

void VideoStreamPlaybackWebM::update_texture_to_picture(Dav1dPicture *picture) {
	int picture_width = picture->p.w;
	int picture_height = picture->p.h;
	int picture_bits_per_channel = picture->p.bpc;

	switch (picture->p.layout) {
		case DAV1D_PIXEL_LAYOUT_I420: {
			if (picture_bits_per_channel == 10) {
				uint8_t *data_y = new uint8_t[picture_width * picture_height];
				uint16_t *y_ptr = (uint16_t *)picture->data[0];
				for (size_t y = 0; y < picture_height; y++) {
					for (size_t x = 0; x < picture_width; x++) {
						ptrdiff_t stride_samples = picture->stride[0] / sizeof(uint16_t);
						uint16_t y_value = y_ptr[y * stride_samples + x];
						data_y[y * picture_width + x] = y_value >> 2;
					}
				}

				uint8_t *data_u = new uint8_t[(picture_width / 2) * (picture_height / 2)];
				uint16_t *u_ptr = (uint16_t *)picture->data[1];
				for (size_t y = 0; y < picture_height / 2; y++) {
					for (size_t x = 0; x < picture_width / 2; x++) {
						ptrdiff_t stride_samples = picture->stride[1] / sizeof(uint16_t);
						uint16_t u_value = u_ptr[y * stride_samples + x];
						data_u[y * (picture_width / 2) + x] = u_value >> 2;
					}
				}

				uint8_t *data_v = new uint8_t[(picture_width / 2) * (picture_height / 2)];
				uint16_t *v_ptr = (uint16_t *)picture->data[2];
				for (size_t y = 0; y < picture_height / 2; y++) {
					for (size_t x = 0; x < picture_width / 2; x++) {
						ptrdiff_t stride_samples = picture->stride[1] / sizeof(uint16_t);
						uint16_t v_value = v_ptr[y * stride_samples + x];
						data_v[y * (picture_width / 2) + x] = v_value >> 2;
					}
				}

				yuv420_2_rgb8888(
						current_image->ptrw(),
						(const uint8_t *)data_y,
						(const uint8_t *)data_u,
						(const uint8_t *)data_v,
						picture_width,
						picture_height,
						picture_width,
						picture_width / 2,
						picture_width << 2);

				delete[] data_y;
				delete[] data_u;
				delete[] data_v;
			} else {
				yuv420_2_rgb8888(
						current_image->ptrw(),
						(const uint8_t *)picture->data[0],
						(const uint8_t *)picture->data[1],
						(const uint8_t *)picture->data[2],
						picture_width,
						picture_height,
						picture->stride[0],
						picture->stride[1],
						picture_width << 2);
			}
		} break;
		default:
			break;
	}

	current_texture->update(current_image);
}

void VideoStreamPlaybackWebM::_update(double p_delta) {
	if ((!is_playing) || (is_paused)) {
		return;
	}

	current_time += p_delta;

	if (has_video_track()) {
		GodotWebMTrack current_track = file_info.tracks[get_video_track()];
		webm::Video current_video = current_track.entry.video.value();

		int64_t next = (last_video_index == -1) ? 0 : (last_video_index + 1);
		uint64_t frame = (last_video_index == -1) ? 0 : last_video_index;
		for (int64_t i = next; i < (int64_t)current_track.frames.size(); i++) {
			GodotWebMFrame current_frame = current_track.frames[i];
			if (current_time >= file_info.getScaledSeconds(current_frame.timecode)) {
				frame = (uint64_t)i;
			} else {
				break;
			}
		}

		switch (current_track.codec) {
			case GDWEBM_SUPPORTED_CODEC_V_AV1: {
				GodotWebMAV1Data* data = (GodotWebMAV1Data*)current_track.data;
				for (int64_t i = (last_sent_index == -1 ? 0 : last_sent_index + 1); i <= frame; i++) {
					GodotWebMFrame webm_frame = current_track.frames[i];
					size_t frame_size = webm_frame.data_size;
					uint8_t* frame_data = dav1d_data_create(&data->data, frame_size);
					if (!frame_data) {
						godot::print_error("Failed to create new Dav1dData for current frame!");
						break;
					}

					current_file->seek(webm_frame.data_index);
					uint64_t read_data = current_file->get_buffer(frame_data, webm_frame.data_size);

					if (read_data != webm_frame.data_size) {
						godot::print_error("Failed to read all data for current frame from file! Only read", read_data, "when was expecting", webm_frame.data_size);
						break;
					}

					data->data.m.offset = i;
					data->data.m.timestamp = current_track.frames[i].timecode;

					while (true) {
						int data_result = dav1d_send_data(data->context, &data->data);

						if (data_result == 0) {
							break;
						} else if (data_result == DAV1D_ERR(EAGAIN)) {
							Dav1dPicture drained = {};

							int drain_result = dav1d_get_picture(data->context, &drained);
							if (drain_result == 0) {
								dav1d_picture_unref(&drained);
							} else if (drain_result != DAV1D_ERR(EAGAIN)) {
								godot::print_error("Failed to get picture from dav1d for decoding (from inside a send data loop)! Code: ", drain_result);
								break;
							}
						} else {
							godot::print_error("Failed to send data to dav1d for decoding! Code: ", data_result);
							dav1d_data_unref(&data->data);
							goto done;
						}
					}

					last_sent_index = i;
				}

				{
					Dav1dPicture picture = {};
					int picture_result = dav1d_get_picture(data->context, &picture);
					if (picture_result == 0) {
						update_texture_to_picture(&picture);
						dav1d_picture_unref(&picture);
					} else if (picture_result != DAV1D_ERR(EAGAIN)) {
						godot::print_error("Failed to get picture from dav1d for decoding! Code: ", picture_result);
						break;
					}
				}

			done:;
			} break;
			default:
				break;
		}

		last_video_index = frame;
	}

	if (has_audio_track()) {
		GodotWebMTrack current_track = file_info.tracks[get_audio_track()];
		webm::Audio current_audio = current_track.entry.audio.value();

		int64_t frame = last_audio_index;
		for (int64_t i = last_audio_index + 1; i < current_track.frames.size(); i++) {
			GodotWebMFrame current_frame = current_track.frames[i];
			if (file_info.getScaledSeconds(current_frame.timecode) - current_time <= 0.001) {
				frame = i;
			} else {
				break;
			}
		}

		if (last_audio_index != frame) {
			switch (current_track.codec) {
				case GDWEBM_SUPPORTED_CODEC_A_OPUS: {
					GodotWebMOpusData* data = (GodotWebMOpusData*)current_track.data;
					OpusDecoder* decoder = data->decoder;

					for (int64_t i = last_audio_index + 1; i <= frame; i++) {
						GodotWebMFrame webm_frame = current_track.frames[i];

						current_file->seek(webm_frame.data_index);
						godot::PackedByteArray opus_frames = current_file->get_buffer(webm_frame.data_size);

						int current_decoded = opus_decode_float(decoder,
							opus_frames.ptr(),
							opus_frames.size(),
							data->pcm.ptrw(),
							data->pcm.size() / data->channels,
							0
						);
						
						if (current_decoded < 0) {
							godot::print_error(current_decoded);
							break;
						}
						
						last_audio_index = i;
						if (current_decoded > 0) {
							int mixed = mix_audio(
								current_decoded,
								data->pcm,
								0
							);
							if (mixed == -1) {
								godot::print_error("Failed to mix audio frames (", current_decoded, " decoded)");
							}
						}
					}

					break;
				}
				default:
					break;
			}
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

uint64_t VideoStreamPlaybackWebM::get_audio_track_count() const {
	uint64_t count = 0;
	for (godot::KeyValue<int64_t, GodotWebMTrack> track : file_info.tracks) {
		if (track.value.entry.audio.is_present()) {
			count++;
		}
	}

	return count;
}

int32_t VideoStreamPlaybackWebM::get_audio_track() const {
	int32_t returned_key = -1;
	int32_t counter = 0;
	for (godot::KeyValue<int64_t, GodotWebMTrack> track : file_info.tracks) {
		bool has_audio = track.value.entry.audio.is_present();
		if (!has_audio) {
			continue;
		}

		if (counter == audio_track) {
			returned_key = track.key;
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
	for (godot::KeyValue<int64_t, GodotWebMTrack> track : file_info.tracks) {
		if (track.value.entry.video.is_present()) {
			count++;
		}
	}

	return count;
}

int32_t VideoStreamPlaybackWebM::get_video_track() const {
	int32_t returned_key = -1;
	int32_t counter = 0;
	for (godot::KeyValue<int64_t, GodotWebMTrack> track : file_info.tracks) {
		bool has_video = track.value.entry.video.is_present();
		if (!has_video) {
			continue;
		}

		if (counter == video_track) {
			returned_key = track.key;
			break;
		}

		counter++;
	}

	return returned_key;
}

bool VideoStreamPlaybackWebM::has_video_track() const {
	return get_video_track_count() > 0;
}

VideoStreamPlaybackWebM::~VideoStreamPlaybackWebM() {
	for (size_t i = 0; i < file_info.tracks.size(); i++) {
		GodotWebMTrack webm_track = file_info.tracks[i];

		if (webm_track.data != NULL) {
			switch (webm_track.codec) {
				case GDWEBM_SUPPORTED_CODEC_V_AV1: {
					GodotWebMAV1Data *av1_data = (GodotWebMAV1Data *)webm_track.data;
					dav1d_close(&av1_data->context);
					delete av1_data;
				} break;
				case GDWEBM_SUPPORTED_CODEC_A_OPUS: {
					GodotWebMOpusData *opus_data = (GodotWebMOpusData *)webm_track.data;
					opus_decoder_destroy(opus_data->decoder);
					delete opus_data;
				} break;
				default:
					break;
			}

		}
	}

	file_info.tracks.clear();
}
