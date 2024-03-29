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

bool hio::IOManagerBase::
    rename(const fs::path& src, const fs::path& dest, bool force /*= false*/) const {
    fs::path abs_src{};
    fs::path abs_dest{};

    if (force) {
        if (!assure_path(src, abs_src, true)) return false;
        if (!assure_path(dest, abs_dest, true)) return false;
    } else {
        if (!resolve_path(src, abs_src)) return false;
        if (!resolve_path(dest, abs_dest)) return false;
    }

    fs::rename(abs_src, abs_dest);

    return true;
}

void hio::IOManagerBase::apply_to_paths(
    std::vector<fs::path>&& paths, Delegate<void(const fs::path&)> func
) const {
    for (const auto& path : paths) apply_to_path(path, func);
}

ui32 hio::IOManagerBase::apply_to_paths(
    std::vector<fs::path>&& paths, Delegate<bool(const fs::path&)> func
) const {
    ui32 successes = 0;
    for (const auto& path : paths) {
        if (apply_to_path(path, func)) ++successes;
    }
    return successes;
}

void hio::IOManagerBase::apply_to_globpath(
    const fs::path& globpath, Delegate<void(const fs::path&)> func
) const {
    apply_to_paths(glob::glob(globpath), func);
}

ui32 hio::IOManagerBase::apply_to_globpath(
    const fs::path& globpath, Delegate<bool(const fs::path&)> func
) const {
    return apply_to_paths(glob::glob(globpath), func);
}

bool hio::IOManagerBase::memory_map_file(
    const fs::path& path, OUT hio::fs::mapped_file& file
) const {
    fs::path abs_path{};
    if (!resolve_path(path, abs_path)) return false;

    file.open(abs_path.string());

    return true;
}

bool hio::IOManagerBase::memory_map_read_only_file(
    const fs::path& path, OUT hio::fs::mapped_file_source& file
) const {
    fs::path abs_path{};
    if (!resolve_path(path, abs_path)) return false;

    file.open(abs_path.string());

    return true;
}

bool hio::IOManagerBase::read_file_to_string(const fs::path& path, std::string& buffer)
    const {
    fs::path abs_path{};
    if (!resolve_path(path, abs_path)) return false;

    // Open file, if we can't then fail.
    FILE* file = fopen(abs_path.string().data(), "rb");
    if (file == nullptr) return false;

    // TODO(Matthew): Only support files up to 4GB in size. Need some paging approach
    //                for larger files.

    // Get length of file contents in bytes.
    fseek(file, 0, SEEK_END);
    ui32 length = static_cast<ui32>(ftell(file));
    fseek(file, 0, SEEK_SET);

    // Reserve memory in buffer.
    buffer = std::string{};
    buffer.resize(length);

    // Read data into buffer.
    size_t chars_read = fread(&buffer[0], 1, length, file);

#if DEBUG
    if (chars_read != length) {
#  if defined(HEMLOCK_OS_WINDOWS)
        debug_printf("%ls could not be read.", abs_path.c_str());
#  else
        debug_printf("%s could not be read.", abs_path.c_str());
#  endif
    }
#else   // DEBUG
    (void)chars_read;
#endif  // !DEBUG

    // Close file.
    fclose(file);

    return true;
}

char* hio::IOManagerBase::
    read_file_to_string(const fs::path& path, ui32* length /*= nullptr*/) const {
    fs::path abs_path{};
    if (!resolve_path(path, abs_path)) return nullptr;

    // Open file, if we can't then fail.
    FILE* file = fopen(abs_path.string().data(), "rb");
    if (file == nullptr) return nullptr;

    // TODO(Matthew): Only support files up to 4GB in size. Need some paging approach
    //                for larger files.

    // Get length of file contents in bytes.
    fseek(file, 0, SEEK_END);
    ui32 _length = static_cast<ui32>(ftell(file));
    fseek(file, 0, SEEK_SET);

    // Pass back length if requested.
    if (length) *length = _length;

    // Reserve memory in buffer.
    char* buffer = new char[_length + 1];
    // Set last character to null-byte.
    buffer[_length] = '\0';

    // Read data into buffer.
    size_t chars_read = fread(buffer, 1, _length, file);

#if DEBUG
    if (chars_read != _length) {
#  if defined(HEMLOCK_OS_WINDOWS)
        debug_printf("%ls could not be read.", abs_path.c_str());
#  else
        debug_printf("%s could not be read.", abs_path.c_str());
#  endif
    }
#else   // DEBUG
    (void)chars_read;
#endif  // !DEBUG

    // Close file.
    fclose(file);

    return buffer;
}

bool hio::IOManagerBase::read_file_to_binary(
    const fs::path& path, std::vector<ui8>& buffer
) const {
    fs::path abs_path{};
    if (!resolve_path(path, abs_path)) return false;

    // Open file, if we can't then fail.
    FILE* file = fopen(abs_path.string().data(), "rb");
    if (file == nullptr) return false;

    // TODO(Matthew): Only support files up to 4GB in size. Need some paging approach
    //                for larger files.

    // Get length of file contents in bytes.
    fseek(file, 0, SEEK_END);
    ui32 length = static_cast<ui32>(ftell(file));
    fseek(file, 0, SEEK_SET);

    // Reserve memory in buffer.
    buffer = std::vector<ui8>{};
    buffer.resize(length);

    // Read data into buffer.
    size_t chars_read = fread(&buffer[0], 1, length, file);

#if DEBUG
    if (chars_read != length) {
#  if defined(HEMLOCK_OS_WINDOWS)
        debug_printf("%ls could not be read.", abs_path.c_str());
#  else
        debug_printf("%s could not be read.", abs_path.c_str());
#  endif
    }
#else   // DEBUG
    (void)chars_read;
#endif  // !DEBUG

    // Close file.
    fclose(file);

    return true;
}

ui8* hio::IOManagerBase::read_file_to_binary(const fs::path& path, ui32& length) const {
    fs::path abs_path{};
    if (!resolve_path(path, abs_path)) return nullptr;

    // Open file, if we can't then fail.
    FILE* file = fopen(abs_path.string().data(), "rb");
    if (file == nullptr) return nullptr;

    // TODO(Matthew): Only support files up to 4GB in size. Need some paging approach
    //                for larger files.

    // Get length of file contents in bytes.
    fseek(file, 0, SEEK_END);
    length = static_cast<ui32>(ftell(file));
    fseek(file, 0, SEEK_SET);

    // Reserve memory in buffer.
    ui8* buffer = new ui8[length];

    // Read data into buffer.
    size_t chars_read = fread(buffer, 1, length, file);

#if DEBUG
    if (chars_read != length) {
#  if defined(HEMLOCK_OS_WINDOWS)
        debug_printf("%ls could not be read.", abs_path.c_str());
#  else
        debug_printf("%s could not be read.", abs_path.c_str());
#  endif
    }
#else   // DEBUG
    (void)chars_read;
#endif  // !DEBUG

    // Close file.
    fclose(file);

    return buffer;
}
