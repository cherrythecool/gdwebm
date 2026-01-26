#include "resource_format_loader_webm.h"
#include "godot_cpp/variant/string.hpp"
#include "video_stream_webm.h"

Variant ResourceFormatLoaderWebM::_load(const String &path, const String &original_path, bool use_sub_threads, int32_t cache_mode) const {
    Ref<VideoStreamWebM> stream;
    stream.instantiate();
    stream->set_file(path);
    return stream;
}

PackedStringArray ResourceFormatLoaderWebM::_get_recognized_extensions() const {
	PackedStringArray recognized_extensions;
	recognized_extensions.push_back("webm");
	return recognized_extensions;
}

bool ResourceFormatLoaderWebM::_handles_type(const StringName &p_type) const {
	return p_type == StringName("VideoStream", true);
}

String ResourceFormatLoaderWebM::_get_resource_type(const String &p_path) const {
	if (p_path.get_extension() == "webm") {
		return "VideoStreamWebM";
	}

	return "";
}
