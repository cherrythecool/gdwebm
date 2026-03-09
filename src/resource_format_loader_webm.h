#pragma once

#include <godot_cpp/classes/resource_format_loader.hpp>
#include <godot_cpp/classes/wrapped.hpp>

using namespace godot;

class ResourceFormatLoaderWebM : public ResourceFormatLoader {
	GDCLASS(ResourceFormatLoaderWebM, ResourceFormatLoader);

protected:
	static void _bind_methods() {};

public:
	virtual Variant _load(const String &p_path, const String &p_original_path, bool p_use_sub_threads, int32_t p_cache_mode) const override;
	virtual PackedStringArray _get_recognized_extensions() const override;
	virtual bool _handles_type(const StringName &p_type) const override;
	virtual String _get_resource_type(const String &p_path) const override;
};
