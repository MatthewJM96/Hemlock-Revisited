#ifndef __hemlock_io_iomanager_h
#define __hemlock_io_iomanager_h

namespace hemlock {
    namespace io {
        class IOManagerBase {
        public:
            IOManagerBase() { /* Empty. */
            }

            virtual ~IOManagerBase() { /* Empty. */
            }

            /**
             * @brief Resolve a relative filepath into a valid absolute filepath.
             * Permissibility of access is not implemented here, rather should this
             * function succeed then permissibility is assumed.
             *
             * @param path The relative filepath to resolve.
             * @param full_path The absolute filepath if successfully resolved.
             * @return True if the path was resolved successfully, false otherwise.
             */
            virtual bool
            resolve_path(const fs::path& path, OUT fs::path& full_path) const
                = 0;
            /**
             * @brief Resolve a relative filepath into a valid absolute filepath,
             * creating the underlying directory structure where it does not already
             * exist. Permissibility of access is not implemented here, rather should
             * this function succeed then permissibility is assumed.
             *
             * @param path The relative filepath to resolve.
             * @param full_path The absolute filepath if successfully resolved.
             * @param is_file Whether the path to assure is a regular file (true) or
             * not (false).
             * @param was_existing Whether the leaf node in the filepath was already
             * existing or not prior to assuring it.
             * @return True if the path was resolved successfully, false otherwise.
             */
            virtual bool assure_path(
                const fs::path& path,
                OUT fs::path& full_path,
                bool          is_file      = false,
                OUT bool*     was_existing = nullptr
            ) const
                = 0;

            /**
             * @brief Reports whether a file is accessible or not via this IO manager
             * context.
             *
             * @param path The relative filepath to determine access for.
             * @return True if accessible, false otherwise.
             */
            bool can_access_file(const fs::path& path) const;
            /**
             * @brief Reports whether a directory is accessible or not via this IO
             * manager context.
             *
             * @param path The relative filepath to determine access for.
             * @return True if accessible, false otherwise.
             */
            bool can_access_directory(const fs::path& path) const;

            /**
             * @brief Creates a directory tree up to and including the leaf node of the
             * provided path.
             *
             * @param path The path to create a directory tree from.
             * @return True if the directories were created, false otherwise.
             */
            bool create_directories(const fs::path& path) const;

            /**
             * @brief Renames a resource, that is to say a resource is moved from the
             * source filepath to the destination filepath.
             *
             * @param src The source filepath where the resource is originally located.
             * @param dest The filepath to move the resource to.
             * @param force If true, the directory tree needed to move the resource is
             * created.
             * @return True if the resource was successfully moved, false otherwise.
             */
            bool
            rename(const fs::path& src, const fs::path& dest, bool force = false) const;

            /**
             * Apply a function to the provided relative filepath after resolution.
             *
             * @param path The relative filepath to resolve and then pass to the
             * provided function.
             * @param func The function to receive the resolved filepath.
             * @return The results of calling the function, or else a
             * default-constructed instance of the same type as the return of the
             * function should the filepath not successfully resolve.
             */
            template <typename ReturnType>
            ReturnType apply_to_path(
                const fs::path& path, Delegate<ReturnType(const fs::path&)> func
            ) const;
            /**
             * Apply a function to the provided relative filepath after resolution.
             *
             * @param path The relative filepath to resolve and then pass to the
             * provided function.
             * @param func The function to receive the resolved filepath.
             * @param default_value A default value to return on failed resolution.
             * @return The results of calling the function, or else the provided default
             * should the filepath not successfully resolve.
             */
            template <typename ReturnType>
            ReturnType apply_to_path(
                const fs::path&                       path,
                Delegate<ReturnType(const fs::path&)> func,
                ReturnType&&                          default_value
            ) const;

            /**
             * Apply a function to the provided relative filepaths after resolution.
             *
             * Note: The function must at most return a bool to apply to multiple paths.
             *
             * @param path The relative filepaths to resolve and then pass to the
             * provided function.
             * @param func The function to receive the resolved filepaths.
             */
            void apply_to_paths(
                std::vector<fs::path>&& paths, Delegate<void(const fs::path&)> func
            ) const;
            /**
             * Apply a function to the provided relative filepaths after resolution.
             *
             * Note: The function must at most return a bool to apply to multiple paths.
             *
             * @param path The relative filepaths to resolve and then pass to the
             * provided function.
             * @param func The function to receive the resolved filepaths.
             * @return The number of function calls that returned true, all paths that
             * failed to resolve are assumed to receive a false return from the
             * function.
             */
            ui32 apply_to_paths(
                std::vector<fs::path>&& paths, Delegate<bool(const fs::path&)> func
            ) const;

            // TODO(Matthew): These are currently incorrectly implemented as they
            //                currently glob the globpath directly, as opposed to
            //                globbing after choosing base(s) to attach it to.
            /**
             * Apply a function to all resources located after resolving the globpath
             * provided.
             *
             * Note: The function must at most return a bool to apply to multiple paths.
             *
             * @param globpath The globpath to resolve and then pass all resources it
             * resolves to to the provided function.
             * @param func The function to receive the resolved filepaths, if no
             * resources resolved on the globpath, then false is returned.
             */
            void apply_to_globpath(
                const fs::path& globpath, Delegate<void(const fs::path&)> func
            ) const;
            /**
             * Apply a function to all resources located after resolving the globpath
             * provided.
             *
             * Note: The function must at most return a bool to apply to multiple paths.
             *
             * @param globpath The globpath to resolve and then pass all resources it
             * resolves to to the provided function.
             * @param func The function to receive the resolved filepaths.
             * @return The number of function calls that returned true, if no resources
             * resolved on the globpath, then 0 is returned.
             */
            ui32 apply_to_globpath(
                const fs::path& globpath, Delegate<bool(const fs::path&)> func
            ) const;

            /**
             * @brief Memory maps the resource located at the provided relative filepath
             * if the filepath resolves.
             *
             * @param path The relative filepath to resolve and then memory map the
             * resource of on success.
             * @param file The resulting memory-mapped resource.
             * @return True if the underlying resource was successfully memory mapped.
             */
            bool
            memory_map_file(const fs::path& path, OUT hio::fs::mapped_file& file) const;
            /**
             * @brief Memory maps the resource located at the provided relative filepath
             * if the filepath resolves. The memory-mapped resource has read-only
             * status.
             *
             * @param path The relative filepath to resolve and then memory map the
             * resource of on success.
             * @param file The resulting memory-mapped resource.
             * @return True if the underlying resource was successfully memory mapped.
             */
            bool memory_map_read_only_file(
                const fs::path& path, OUT hio::fs::mapped_file_source& file
            ) const;

            /**
             * @brief Reads the resource located at the provided relative filepath to a
             * string buffer if the filepath resolves.
             *
             * @param path The relative filepath to resolve and then read to buffer on
             * success.
             * @param buffer The buffer to read the resource into.
             * @return True if the resource was successfully read to the buffer.
             */
            bool
            read_file_to_string(const fs::path& path, OUT std::string& buffer) const;
            /**
             * @brief Reads the resource located at the provided relative filepath to a
             * C-string buffer if the filepath resolves.
             *
             * @param path The relative filepath to resolve and then read to buffer on
             * success.
             * @param length The length of the resulting buffer.
             * @return Pointer to the start of the buffer if the filepath resolved,
             * otherwise nullptr.
             */
            CALLER_DELETE char*
            read_file_to_string(const fs::path& path, OUT ui32* length = nullptr) const;
            /**
             * @brief Reads the resource located at the provided relative filepath to a
             * char vector buffer if the filepath resolves.
             *
             * @param path The relative filepath to resolve and then read to buffer on
             * success.
             * @param buffer The buffer to read the resource into.
             * @return True if the resource was successfully read to the buffer.
             */
            bool read_file_to_binary(const fs::path& path, OUT std::vector<ui8>& buffer)
                const;
            /**
             * @brief Reads the resource located at the provided relative filepath to a
             * char buffer if the filepath resolves. Note that this does not promise to
             * return a valid C-string as the buffer is not necessarily null-terminated.
             *
             * @param path The relative filepath to resolve and then read to buffer on
             * success.
             * @param length The length of the resulting buffer.
             * @return Pointer to the start of the buffer if the filepath resolved,
             * otherwise nullptr.
             */
            CALLER_DELETE ui8*
            read_file_to_binary(const fs::path& path, OUT ui32& length) const;
        };
    }  // namespace io
}  // namespace hemlock
namespace hio = hemlock::io;

template <typename ReturnType>
inline ReturnType hio::IOManagerBase::apply_to_path(
    const fs::path& path, Delegate<ReturnType(const fs::path&)> func
) const {
    fs::path abs_path{};
    if (!resolve_path(path, abs_path)) {
        if constexpr (std::same_as<ReturnType, void>) {
            return;
        } else if constexpr (std::is_pointer_v<ReturnType> || std::is_default_constructible_v<ReturnType>)
        {
            return ReturnType{};
        } else {
            debug_printf(
                "Cannot call apply_to_path with non-defaultable and non-void return "
                "type.\n"
            );
            assert(false);
        }
    }

    if constexpr (std::same_as<ReturnType, void>) {
        func(abs_path);
    } else if constexpr (std::is_pointer_v<ReturnType> || std::is_default_constructible_v<ReturnType>)
    {
        return func(abs_path);
    } else {
        debug_printf(
            "Cannot call apply_to_path with non-defaultable and non-void return "
            "type.\n"
        );
        assert(false);
    }
}

template <typename ReturnType>
inline ReturnType hio::IOManagerBase::apply_to_path(
    const fs::path&                       path,
    Delegate<ReturnType(const fs::path&)> func,
    ReturnType&&                          default_value
) const {
    fs::path abs_path{};
    if (!resolve_path(path, abs_path)) return std::forward<ReturnType>(default_value);

    return func(abs_path);
}

#endif  // __hemlock_io_iomanager_h
