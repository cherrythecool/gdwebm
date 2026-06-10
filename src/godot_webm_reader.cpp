#include "godot_webm_reader.h"
#include "godot_cpp/core/math.hpp"
#include <cstdint>

GodotWebMReader::GodotWebMReader(godot::Ref<godot::FileAccess> p_file) {
	file_position = 0;

	if (!p_file.is_null() && p_file.is_valid()) {
		file = p_file;
		file_length = file->get_length();
	}
}

webm::Status GodotWebMReader::Read(std::size_t num_to_read, std::uint8_t *buffer,
		std::uint64_t *num_actually_read) {
	if (num_to_read == 0) {
		if (num_actually_read) {
			*num_actually_read = 0;
		}

		return webm::Status(webm::Status::kOkCompleted);
	}

	if (!file.is_valid()) {
		if (num_actually_read) {
			*num_actually_read = 0;
		}

		return webm::Status(webm::Status::kEndOfFile);
	}

	if (file_position + 1 >= file_length && num_to_read > 0) {
		if (num_actually_read) {
			*num_actually_read = 0;
		}

		return webm::Status(STATUS_FILE_ENDED_CORRECTLY);
	}

	if (num_actually_read) {
		*num_actually_read = file->get_buffer(buffer, num_to_read);
		file_position += *num_actually_read;

		if (*num_actually_read != num_to_read) {
			return webm::Status(webm::Status::kOkPartial);
		}
	}

	return webm::Status(webm::Status::kOkCompleted);
}

webm::Status GodotWebMReader::Skip(std::uint64_t num_to_skip,
		std::uint64_t *num_actually_skipped) {
	if (!file.is_valid()) {
		if (num_actually_skipped) {
			*num_actually_skipped = 0;
		}

		return webm::Status(webm::Status::kEndOfFile);
	}

	if (file_position + 1 >= file_length) {
		if (num_actually_skipped) {
			*num_actually_skipped = 0;
		}

		return webm::Status(webm::Status::kEndOfFile);
	}

	if (num_actually_skipped) {
		uint64_t advance = godot::Math::min(num_to_skip, file_length - file_position);
		file->seek(file_position + advance);
		file_position += advance;
		*num_actually_skipped = advance;

		if (advance != num_to_skip) {
			return webm::Status(webm::Status::kOkPartial);
		}
	}

	return webm::Status(webm::Status::kOkCompleted);
}

std::uint64_t GodotWebMReader::Position() const {
	return file_position;
}
