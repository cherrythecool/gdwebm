#include "register_types.h"

#include <gdextension_interface.h>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/core/defs.hpp>
#include <godot_cpp/godot.hpp>

#include "godot_cpp/classes/resource_loader.hpp"
#include "resource_format_loader_webm.h"
#include "video_stream_webm.h"

using namespace godot;

static Ref<ResourceFormatLoaderWebM> resource_loader_webm;

void initialize_gdextension_types(ModuleInitializationLevel p_level) {
	if (p_level != MODULE_INITIALIZATION_LEVEL_EDITOR) {
		return;
	}

	GDREGISTER_CLASS(VideoStreamWebM);
	GDREGISTER_CLASS(VideoStreamPlaybackWebM);
	GDREGISTER_CLASS(ResourceFormatLoaderWebM);
	resource_loader_webm.instantiate();
	ResourceLoader::get_singleton()->add_resource_format_loader(resource_loader_webm);
}

void uninitialize_gdextension_types(ModuleInitializationLevel p_level) {
	if (p_level != MODULE_INITIALIZATION_LEVEL_EDITOR) {
		return;
	}

	ResourceLoader::get_singleton()->remove_resource_format_loader(resource_loader_webm);
	resource_loader_webm.unref();
}

extern "C" {
	// Initialization
	GDExtensionBool GDE_EXPORT gdwebm_init(GDExtensionInterfaceGetProcAddress p_get_proc_address,
													GDExtensionClassLibraryPtr p_library,
													GDExtensionInitialization *r_initialization) {
		GDExtensionBinding::InitObject init_obj(p_get_proc_address, p_library, r_initialization);
		init_obj.register_initializer(initialize_gdextension_types);
		init_obj.register_terminator(uninitialize_gdextension_types);
		init_obj.set_minimum_library_initialization_level(MODULE_INITIALIZATION_LEVEL_EDITOR);

		return init_obj.init();
	}
}
