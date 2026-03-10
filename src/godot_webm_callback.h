#pragma once

#include "godot_cpp/templates/vector.hpp"
#include "godot_cpp/templates/vmap.hpp"
#include "godot_cpp/variant/packed_byte_array.hpp"
#include "godot_cpp/variant/packed_float32_array.hpp"

#include "dav1d/dav1d.h"
#include "opus.h"
#include "webm/callback.h"

#include <cstdint>

enum GodotWebMSupportedCodec {
	GDWEBM_UNSUPPORTED_CODEC = 0,

	GDWEBM_SUPPORTED_CODEC_V_AV1 = 0x100,
	GDWEBM_SUPPORTED_CODEC_V_VP9 = 0x110,
	GDWEBM_SUPPORTED_CODEC_V_VP8 = 0x120,

	GDWEBM_SUPPORTED_CODEC_A_OPUS = 0x200,
	GDWEBM_SUPPORTED_CODEC_A_VORBIS = 0x201,
};

struct GodotWebMFrame {
	godot::PackedByteArray data;
	uint64_t timecode;
};

struct GodotWebMOpusData {
	OpusDecoder *decoder;
	godot::PackedFloat32Array pcm;
	size_t channels;
	size_t sample_rate;
};

struct GodotWebMAV1Data {
	Dav1dContext *context;
	Dav1dData data;
	Dav1dPicture picture;
};

struct GodotWebMTrack {
	GodotWebMSupportedCodec codec = GDWEBM_UNSUPPORTED_CODEC;
	webm::TrackEntry entry;
	godot::Vector<GodotWebMFrame> frames;
	void *data;
};

struct GodotWebMFileInfo {
	uint64_t durationTimecode = 0;
	uint64_t timecodeScale = 1000000;

	godot::VMap<int64_t, GodotWebMTrack> tracks;

public:
	double getScaledSeconds(const uint64_t timecode) const {
		return (double)(timecode * timecodeScale) / 1000000000.0;
	}
};

using namespace webm;

class GodotWebMCallback : public webm::Callback {
private:
	uint64_t current_track = 0;
	uint64_t current_timecode_base = 0;
	uint64_t current_timecode = 0;
	uint64_t current_frame_index = 0;

public:
	GodotWebMFileInfo *file_info;

	GodotWebMCallback(GodotWebMFileInfo *p_file_info);
	virtual webm::Status OnInfo(const webm::ElementMetadata &metadata, const webm::Info &info) override;
	virtual Status OnTrackEntry(const ElementMetadata &metadata,
			const TrackEntry &track_entry) override;
	virtual Status OnClusterBegin(const ElementMetadata &metadata,
			const Cluster &cluster, Action *action) override;
	virtual Status OnFrame(const FrameMetadata &metadata, Reader *reader,
			std::uint64_t *bytes_remaining) override;
	virtual Status OnBlockBegin(const ElementMetadata &metadata,
			const Block &block, Action *action) override;
	virtual Status OnSimpleBlockBegin(const ElementMetadata &metadata,
			const SimpleBlock &simple_block,
			Action *action) override;
};
