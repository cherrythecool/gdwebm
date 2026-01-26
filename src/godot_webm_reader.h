#pragma once

#include "godot_cpp/classes/file_access.hpp"
#include "godot_cpp/classes/ref.hpp"
#include "webm/reader.h"

enum GodotWebMStatus {
	STATUS_FILE_ENDED_CORRECTLY = 1,
};

class GodotWebMReader : public webm::Reader {
	private:
		godot::Ref<godot::FileAccess> file;

	public:
		GodotWebMReader(godot::Ref<godot::FileAccess> p_file);

		virtual webm::Status Read(std::size_t num_to_read, std::uint8_t* buffer,
		                      std::uint64_t* num_actually_read);

		virtual webm::Status Skip(std::uint64_t num_to_skip,
		                      std::uint64_t* num_actually_skipped);

		virtual std::uint64_t Position() const;
};
