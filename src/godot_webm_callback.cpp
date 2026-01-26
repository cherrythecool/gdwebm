#include "godot_webm_callback.h"

#include "godot_cpp/core/print_string.hpp"
#include "webm/status.h"

GodotWebMCallback::GodotWebMCallback(GodotWebMFileInfo* p_file_info) {
	file_info = p_file_info;
}

Status GodotWebMCallback::OnTrackEntry(const ElementMetadata& metadata,
                            const TrackEntry& track_entry) {
    if (track_entry.codec_id.is_present()) {
    	godot::Array arr;
     	arr.push_back(track_entry.codec_id.value().c_str());
   		godot::print_line(godot::String("Found codec with id %s!") % arr);

     	if (track_entry.video.is_present()) {
      		GodotWebMSupportedCodec codec = GDWEBM_UNSUPPORTED_CODEC;
    		if (track_entry.codec_id.value() == "V_AV1") {
      			codec = GDWEBM_SUPPORTED_CODEC_V_AV1;
      		} else if (track_entry.codec_id.value() == "V_VP9") {
        		codec = GDWEBM_SUPPORTED_CODEC_V_VP9;
       		} else if (track_entry.codec_id.value() == "V_VP8") {
         		codec = GDWEBM_SUPPORTED_CODEC_V_VP8;
         	}

    		file_info->videoTracks.push_back(GodotWebMVideoTrack {
      			codec,
         		track_entry.video.value().pixel_width.value(),
           		track_entry.video.value().pixel_height.value(),
      		});
      	}

      	if (track_entry.audio.is_present()) {
       		GodotWebMSupportedCodec codec = GDWEBM_UNSUPPORTED_CODEC;
         	if (track_entry.codec_id.value() == "A_OPUS") {
          		codec = GDWEBM_SUPPORTED_CODEC_A_OPUS;
      		} else if (track_entry.codec_id.value() == "A_VORBIS") {
          		codec = GDWEBM_SUPPORTED_CODEC_A_VORBIS;
      		}

     		file_info->audioTracks.push_back(GodotWebMAudioTrack {
       			codec,
        		track_entry.audio.value().channels.value(),
          		(uint64_t)track_entry.audio.value().sampling_frequency.value(),
       		});
       	}
    }

    return webm::Status(webm::Status::kOkCompleted);
}

Status GodotWebMCallback::OnClusterBegin(const ElementMetadata& metadata,
                              const Cluster& cluster, Action* action) {
    *action = Action::kRead;
    if (cluster.timecode.is_present()) {
   		godot::Array arr;
    	arr.push_back(cluster.timecode.value());
  		godot::print_line(godot::String("Found timecode %d!") % arr);
    }

    return Status(Status::kOkCompleted);
}

webm::Status GodotWebMCallback::OnInfo(const webm::ElementMetadata &metadata, const webm::Info &info) {
	if (info.timecode_scale.is_present()) {
		file_info->timecodeScale = info.timecode_scale.value();
	}

	if (info.duration.is_present()) {
		file_info->durationTimecode = info.duration.value();
	}

	return webm::Status(webm::Status::kOkCompleted);
}
