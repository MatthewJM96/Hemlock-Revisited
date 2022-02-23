#ifndef __hemlock_io_iomanager_h
#define __hemlock_io_iomanager_h

namespace hemlock {
    namespace io {
        class IOManagerBase {
        public:
            IOManagerBase() { /* Empty. */ }
            virtual ~IOManagerBase() { /* Empty. */ }

            virtual bool resolve_path(const fs::path& path, OUT fs::path& full_path) const = 0;
            virtual bool assure_path (  const fs::path& path,
                                          OUT fs::path& full_path,
                                                   bool is_file      = false,
                                              OUT bool* was_existing = nullptr  ) const = 0;

            virtual bool resolve_paths(IN OUT std::vector<fs::path>& paths) const = 0;
            virtual bool assure_paths (IN OUT std::vector<fs::path>& paths) const = 0;

            bool can_access_file     (const fs::path& path) const;
            bool can_access_directory(const fs::path& path) const;

            bool create_directories(const fs::path& path) const;

            bool rename(const fs::path& src, const fs::path& dest, bool force = false) const;

            template <typename ReturnType>
            ReturnType apply_to_path(const fs::path& path, Delegate<ReturnType(const fs::path&)> func) const;
            template <typename ReturnType>
            ReturnType apply_to_path(const fs::path& path, Delegate<ReturnType(const fs::path&)> func, ReturnType&& default_value) const;

            void apply_to_paths(std::vector<fs::path>&& paths, Delegate<void(const fs::path&)> func) const;
            ui32 apply_to_paths(std::vector<fs::path>&& paths, Delegate<bool(const fs::path&)> func) const;

            void apply_to_globpath(const fs::path& globpath, Delegate<void(const fs::path&)> func) const;
            ui32 apply_to_globpath(const fs::path& globpath, Delegate<bool(const fs::path&)> func) const;

                           bool read_file_to_string(const fs::path& path, OUT std::string& buffer) const;
            CALLER_DELETE char* read_file_to_string(const fs::path& path, OUT ui32* length = nullptr) const;


                          bool read_file_to_binary(const fs::path& path, OUT std::vector<ui8>& buffer) const;
            CALLER_DELETE ui8* read_file_to_binary(const fs::path& path, OUT ui32& length) const;
        };
    }
}
namespace hio = hemlock::io;

template <typename ReturnType>
inline ReturnType hio::IOManagerBase::apply_to_path(
                        const fs::path&   path,
    Delegate<ReturnType(const fs::path&)> func
) const {
    fs::path abs_path{};
    if (!resolve_path(path, abs_path)) {
        if constexpr (std::same_as<ReturnType, void>) {
            return;
        } else if constexpr (
            std::is_pointer_v<ReturnType>
                || std::is_default_constructible_v<ReturnType>
        ) {
            return ReturnType{};
        } else {
            debug_printf("Cannot call apply_to_path with non-defaultable and non-void return type.\n");
            assert(false);
        }
    }

    if constexpr (std::same_as<ReturnType, void>) {
        func(abs_path);
    } else if constexpr (
        std::is_pointer_v<ReturnType>
            || std::is_default_constructible_v<ReturnType>
    ) {
        return func(abs_path);
    } else {
        debug_printf("Cannot call apply_to_path with non-defaultable and non-void return type.\n");
        assert(false);
    }
}


template <typename ReturnType>
inline ReturnType hio::IOManagerBase::apply_to_path(
                        const fs::path&   path,
    Delegate<ReturnType(const fs::path&)> func,
                             ReturnType&& default_value
) const {
    fs::path abs_path{};
    if (!resolve_path(path, abs_path)) return std::forward<ReturnType>(default_value);

    return func(abs_path);
}

#endif // __hemlock_io_iomanager_h
