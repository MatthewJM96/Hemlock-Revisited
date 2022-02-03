#include "stdafx.h"

#include "io/glob.h"

#include "io/iomanager.h"


bool hio::IOManagerBase::can_access_file(const fs::path& path) const {
    fs::path abs_path{};
    if (!resolve_path(path, abs_path)) return false;

    return fs::is_regular_file(abs_path);
}

bool hio::IOManagerBase::can_access_directory(const fs::path& path) const {
    fs::path abs_path{};
    if (!resolve_path(path, abs_path)) return false;

    return fs::is_directory(abs_path);
}

bool hio::IOManagerBase::create_directories(const fs::path& path) const {
    fs::path abs_path{};
    if (!resolve_path(path, abs_path)) return false;

    return fs::create_directories(abs_path);
}

bool hio::IOManagerBase::rename(const fs::path& src, const fs::path& dest, bool force /*= false*/) const {
    fs::path abs_src{};
    fs::path abs_dest{};
    
    if (force) {
        if (!assure_path(src,  abs_src,  true))   return false;
        if (!assure_path(dest, abs_dest, true)) return false;
    } else {
        if (!resolve_path(src,  abs_src))  return false;
        if (!resolve_path(dest, abs_dest)) return false;
    }

    fs::rename(abs_src, abs_dest);

    return true;
}

void hio::IOManagerBase::apply_to_paths(std::vector<fs::path>&& paths, Delegate<void(const fs::path&)> func) const {
    for (const auto& path : paths) apply_to_path(path, func);
}

ui32 hio::IOManagerBase::apply_to_paths(std::vector<fs::path>&& paths, Delegate<bool(const fs::path&)> func) const {
    ui32 successes = 0;
    for (const auto& path : paths) {
        if (apply_to_path(path, func)) ++successes;
    }
    return successes;
}

void hio::IOManagerBase::apply_to_globpath(const fs::path& globpath, Delegate<void(const fs::path&)> func) const {
    apply_to_paths(glob::glob(globpath), func);
}

ui32 hio::IOManagerBase::apply_to_globpath(const fs::path& globpath, Delegate<bool(const fs::path&)> func) const {
    return apply_to_paths(glob::glob(globpath), func);
}

bool hio::IOManagerBase::read_file_to_string(const fs::path& path, std::string& buffer) const {
    fs::path abs_path{};
    if (!resolve_path(path, abs_path)) return false;

    // Open file, if we can't then fail.
    FILE* file = fopen(abs_path.c_str(), "rb");
    if (file == nullptr) return false;

    // Get length of file contents in bytes.
    fseek(file, 0, SEEK_END);
    ui64 length = ftell(file);
    fseek(file, 0, SEEK_SET);

    // Reserve memory in buffer.
    buffer = std::string{};
    buffer.resize(length);

    // Read data into buffer.
    fread(&buffer[0], 1, length, file);

    // Close file.
    fclose(file);

    return true;
}

char* hio::IOManagerBase::read_file_to_string(const fs::path& path, ui64* length /*= nullptr*/) const {
    fs::path abs_path{};
    if (!resolve_path(path, abs_path)) return nullptr;

    // Open file, if we can't then fail.
    FILE* file = fopen(abs_path.c_str(), "rb");
    if (file == nullptr) return nullptr;

    // Get length of file contents in bytes.
    fseek(file, 0, SEEK_END);
    ui64 _length = ftell(file);
    fseek(file, 0, SEEK_SET);

    // Pass back length if requested.
    if (length) *length = _length;

    // Reserve memory in buffer.
    char* buffer = new char[_length + 1];
    // Set last character to null-byte.
    buffer[_length] = '\0';

    // Read data into buffer.
    fread(buffer, 1, _length, file);

    // Close file.
    fclose(file);

    return buffer;
}


bool hio::IOManagerBase::read_file_to_binary(const fs::path& path, std::vector<ui8>& buffer) const {
    fs::path abs_path{};
    if (!resolve_path(path, abs_path)) return false;

    // Open file, if we can't then fail.
    FILE* file = fopen(abs_path.c_str(), "rb");
    if (file == nullptr) return false;

    // Get length of file contents in bytes.
    fseek(file, 0, SEEK_END);
    ui64 length = ftell(file);
    fseek(file, 0, SEEK_SET);

    // Reserve memory in buffer.
    buffer = std::vector<ui8>{};
    buffer.resize(length);

    // Read data into buffer.
    fread(&buffer[0], 1, length, file);

    // Close file.
    fclose(file);

    return true;
}

ui8* hio::IOManagerBase::read_file_to_binary(const fs::path& path, ui64& length) const {
    fs::path abs_path{};
    if (!resolve_path(path, abs_path)) return nullptr;

    // Open file, if we can't then fail.
    FILE* file = fopen(abs_path.c_str(), "rb");
    if (file == nullptr) return nullptr;

    // Get length of file contents in bytes.
    fseek(file, 0, SEEK_END);
    length = ftell(file);
    fseek(file, 0, SEEK_SET);

    // Reserve memory in buffer.
    ui8* buffer = new ui8[length];

    // Read data into buffer.
    fread(buffer, 1, length, file);

    // Close file.
    fclose(file);

    return buffer;
}
