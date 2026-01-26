#pragma once

#include "godot_cpp/templates/vector.hpp"
#include "godot_cpp/variant/typed_array.hpp"

#include "webm/callback.h"
#include <cstdint>
#include <vector>

enum GodotWebMSupportedCodec {
	GDWEBM_UNSUPPORTED_CODEC = 0,

	GDWEBM_SUPPORTED_CODEC_V_AV1 = 0x100,
	GDWEBM_SUPPORTED_CODEC_V_VP9 = 0x110,
	GDWEBM_SUPPORTED_CODEC_V_VP8 = 0x120,

	GDWEBM_SUPPORTED_CODEC_A_OPUS = 0x200,
	GDWEBM_SUPPORTED_CODEC_A_VORBIS = 0x201,
};

struct GodotWebMAudioTrack {
	GodotWebMSupportedCodec codec = GDWEBM_UNSUPPORTED_CODEC;
	uint64_t channels = 0;
	uint64_t sampleRate = 0;
};

struct GodotWebMVideoTrack {
	GodotWebMSupportedCodec codec = GDWEBM_UNSUPPORTED_CODEC;
	uint64_t width = 0;
	uint64_t height = 0;
};

struct GodotWebMFileInfo {
	uint64_t durationTimecode = 0;
	uint64_t timecodeScale = 1000000;

	godot::Vector<GodotWebMVideoTrack> videoTracks;
	godot::Vector<GodotWebMAudioTrack> audioTracks;

	public:
		double getScaledSeconds(const uint64_t timecode) const {
			return (double)(timecode * timecodeScale) / 1000000000.0;
		}
};

using namespace webm;

class GodotWebMCallback : public webm::Callback {
	public:
		GodotWebMFileInfo* file_info;

		GodotWebMCallback(GodotWebMFileInfo* p_file_info);
		virtual webm::Status OnInfo(const webm::ElementMetadata &metadata, const webm::Info &info) override;
		virtual Status OnTrackEntry(const ElementMetadata& metadata,
                              const TrackEntry& track_entry) override;
		virtual Status OnClusterBegin(const ElementMetadata& metadata,
                                const Cluster& cluster, Action* action) override;
};
