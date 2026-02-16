#include "godot_webm_callback.h"

#include "godot_cpp/core/math.hpp"
#include "godot_cpp/core/print_string.hpp"
#include "godot_cpp/variant/packed_float32_array.hpp"
#include "webm/status.h"

#include "opus.h"
#include <cstdlib>

GodotWebMCallback::GodotWebMCallback(GodotWebMFileInfo* p_file_info) {
	file_info = p_file_info;
}

Status GodotWebMCallback::OnTrackEntry(const ElementMetadata& metadata,
                            const TrackEntry& track_entry) {
    if (track_entry.codec_id.is_present()) {
    	const std::string& codec_id = track_entry.codec_id.value();
    	GodotWebMTrack track;
   		GodotWebMSupportedCodec codec = GDWEBM_UNSUPPORTED_CODEC;
     	track.entry = track_entry;
      	track.data = nullptr;

     	if (track_entry.video.is_present()) {
    		if (codec_id == "V_AV1") {
      			codec = GDWEBM_SUPPORTED_CODEC_V_AV1;
      		} else if (codec_id == "V_VP9") {
        		codec = GDWEBM_SUPPORTED_CODEC_V_VP9;
       		} else if (codec_id == "V_VP8") {
         		codec = GDWEBM_SUPPORTED_CODEC_V_VP8;
         	}
      	} else if (track_entry.audio.is_present()) {
         	if (codec_id == "A_OPUS") {
          		codec = GDWEBM_SUPPORTED_CODEC_A_OPUS;

           		GodotWebMOpusData* data = new GodotWebMOpusData;
             	if (data == nullptr) {
              		godot::print_error("Failed to allocate GodotWebMOpusData when loading Opus track");
                	return webm::Status(webm::Status::kNotEnoughMemory);
              	}

             	data->sample_rate = track_entry.audio.value().sampling_frequency.value();
              	data->channels = track_entry.audio.value().channels.value();
                data->pcm.resize(data->channels * godot::Math::ceil(data->sample_rate * (120.0 / 1000.0)));

            	int error;
            	data->decoder = opus_decoder_create(
             		data->sample_rate,
               		data->channels,
                 	&error
             	);

             	track.data = (void*)data;

             	if (error != OPUS_OK) {
              		godot::Array values;
                	values.push_back(error);
              		godot::print_error("Error creating Opus Decoder: %d", values);
              	}
      		} else if (codec_id == "A_VORBIS") {
          		codec = GDWEBM_SUPPORTED_CODEC_A_VORBIS;
      		}
       	}

     	track.codec = codec;
     	file_info->tracks[track_entry.track_number.value()] = track;
    }

    return webm::Status(webm::Status::kOkCompleted);
}

Status GodotWebMCallback::OnClusterBegin(const ElementMetadata& metadata,
                              const Cluster& cluster, Action* action) {
    *action = Action::kRead;
    if (cluster.timecode.is_present()) {
   		current_timecode_base = cluster.timecode.value();
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

webm::Status GodotWebMCallback::OnFrame(const FrameMetadata& metadata, Reader* reader, std::uint64_t* bytes_remaining) {
	if (metadata.parent_element.id != webm::Id::kBlock && metadata.parent_element.id != webm::Id::kSimpleBlock) {
		std::uint64_t skipped;
		webm::Status skip_status = reader->Skip(*bytes_remaining, &skipped);
		*bytes_remaining -= skipped;
		while (!skip_status.completed_ok() && *bytes_remaining > 0) {
			skip_status = reader->Skip(*bytes_remaining, &skipped);
			*bytes_remaining -= skipped;
		}

		if (!skip_status.completed_ok()) {
			return skip_status;
		}

		return webm::Status(webm::Status::kOkCompleted);
	}

	GodotWebMFrame frame;
	frame.timecode = current_timecode_base + current_timecode;
	frame.data.resize(metadata.size);

	std::uint64_t read;
	webm::Status read_status = reader->Read(*bytes_remaining, frame.data.ptrw(), &read);
	*bytes_remaining -= read;
	while (!read_status.completed_ok() && *bytes_remaining > 0) {
		read_status = reader->Read(*bytes_remaining, frame.data.ptrw(), &read);
		*bytes_remaining -= read;
	}

	godot::Vector<GodotWebMFrame>* frames = &file_info->tracks[current_track].frames;
	frames->resize(frames->size() + 1);
	frames->ptrw()[frames->size() - 1] = frame;
    return webm::Status(webm::Status::kOkCompleted);
}

webm::Status GodotWebMCallback::OnBlockBegin(const ElementMetadata& metadata, const Block& block, Action* action) {
	*action = Action::kRead;
	current_track = block.track_number;
	current_timecode = block.timecode;

	return webm::Status(webm::Status::kOkCompleted);
}

webm::Status GodotWebMCallback::OnSimpleBlockBegin(const ElementMetadata& metadata, const SimpleBlock& simple_block, Action* action) {
	*action = Action::kRead;
	current_track = simple_block.track_number;
	current_timecode = simple_block.timecode;

	return webm::Status(webm::Status::kOkCompleted);
}
