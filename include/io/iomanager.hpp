#ifndef __hemlock_io_iomanager_hpp
#define __hemlock_io_iomanager_hpp

namespace hemlock {
    namespace io {
        template <typename ReturnType>
            requires ( std::is_same_v<ReturnType, void> || std::is_pointer_v<ReturnType> || std::is_default_constructible_v<ReturnType> )
        using PathOperator = Delegate<ReturnType(const fs::path&)>;

        class IOManagerBase {
        public:
            IOManagerBase() { /* Empty. */ }
            virtual ~IOManagerBase() { /* Empty. */ }

            bool can_access_file     (const fs::path& path) const;
            bool can_access_directory(const fs::path& path) const;

            bool create_directories(const fs::path& path) const;

            bool rename(const fs::path& src, const fs::path& dest, bool force = false) const;

            template <typename ReturnType>
            ReturnType apply_to_path(const fs::path& path, PathOperator<ReturnType> func) const;
            template <typename ReturnType>
                requires ( !std::is_same_v<ReturnType, void> )
            ReturnType apply_to_path(const fs::path& path, PathOperator<ReturnType> func, ReturnType&& default_value) const;

            void apply_to_paths(std::vector<fs::path>&& paths, PathOperator<void> func) const;
            ui32 apply_to_paths(std::vector<fs::path>&& paths, PathOperator<bool> func) const;

            void apply_to_globpath(const fs::path& globpath, PathOperator<void> func) const;
            ui32 apply_to_globpath(const fs::path& globpath, PathOperator<bool> func) const;

            bool memory_map_file(const fs::path& path, OUT hio::fs::mapped_file& file) const;
            bool memory_map_read_only_file(const fs::path& path, OUT hio::fs::mapped_file_source& file) const;

                           bool read_file_to_string(const fs::path& path, OUT std::string& buffer) const;
            CALLER_DELETE char* read_file_to_string(const fs::path& path, OUT ui32* length = nullptr) const;

                          bool read_file_to_binary(const fs::path& path, OUT std::vector<ui8>& buffer) const;
            CALLER_DELETE ui8* read_file_to_binary(const fs::path& path, OUT ui32& length) const;
        protected:
            /**
             * @brief Resolves the given path to a directory or file.
             * It can be required that the resource be a file by setting
             * is_file to true. Resolution informs of access to the
             * resource as well as providing the absolute filepath to
             * the resource identified.
             *
             * @param path The path to resolve.
             * @param full_path The resolved absolute path.
             * @param is_file Whether the resource is required to be a
             * file.
             * @return True if a valid resource was resolved, false
             * otherwise.
             */
            virtual bool resolve_path(  const fs::path& path,
                                          OUT fs::path& full_path,
                                                   bool is_file = false) const = 0;
            /**
             * @brief Assures the given path. The path can be to a
             * directory or file, indicated by the is_file parameter.
             * In the case of a file, the file's existence is not assured,
             * merely the directory tree it requires to be created. The
             * absolute filepath of the resource is provided if it was
             * successfully assured. The final resource's existence, file
             * or directory, can be reported.
             *
             * @param path The path to assure.
             * @param full_path The assured absolute path.
             * @param is_file Whether the path provided is to a file
             * or to a leaf directory.
             * @param was_existing Informs if the resource was already
             * existent before the function call.
             * @return True if the path was assured, false otherwise.
             */
            virtual bool assure_path (  const fs::path& path,
                                          OUT fs::path& full_path,
                                                   bool is_file      = false,
                                              OUT bool* was_existing = nullptr  ) const = 0;

            /**
             * @brief Resolve a set of paths.
             *
             * @param paths The paths to resolve, this is modified
             * to become the absolute path on resolution.
             * @param successes The success state of each path's
             * resolution.
             * @param are_files Whether the paths are to files or
             * directories.
             * @return True if all paths were resolved, false
             * otherwise.
             */
            bool resolve_paths(IN OUT std::vector<fs::path>& paths, OUT std::vector<bool>* successes = nullptr, bool are_files = false) const;

            /**
             * @brief Assure a set of paths.
             *
             * @param paths The paths to assure, this is modified
             * to become the absolute path on assurance.
             * @param successes The success state of each path's
             * assurance.
             * @param was_existing Reports if each path was to
             * an already existent resource or not.
             * @param are_files Whether the paths are to files or
             * directories.
             * @return True if all paths were assured, false
             * otherwise.
             */
            bool assure_paths (IN OUT std::vector<fs::path>& paths, OUT std::vector<bool>* successes = nullptr, OUT std::vector<bool>* was_existing = nullptr, bool are_files = false) const;
        };

        class FileReference {
        public:
            FileReference() :
                m_iom(nullptr), m_filepath("")
            { /* Empty. */ }
            FileReference(fs::path filepath, IOManagerBase* iom) :
                m_iom(iom), m_filepath(filepath)
            { /* Empty. */}
            virtual ~FileReference() { /* Empty. */ };

            bool can_access_file() const;

            bool rename(const fs::path& dest, bool force = false) const;

            template <typename ReturnType>
            ReturnType apply_to_path(PathOperator<ReturnType> func) const;
            template <typename ReturnType>
                requires ( !std::is_same_v<ReturnType, void> )
            ReturnType apply_to_path(PathOperator<ReturnType> func, ReturnType&& default_value) const;

            bool memory_map_file(OUT hio::fs::mapped_file& file) const;
            bool memory_map_read_only_file(OUT hio::fs::mapped_file_source& file) const;

                           bool read_file_to_string(OUT std::string& buffer) const;
            CALLER_DELETE char* read_file_to_string(OUT ui32* length = nullptr) const;

                          bool read_file_to_binary(OUT std::vector<ui8>& buffer) const;
            CALLER_DELETE ui8* read_file_to_binary(OUT ui32& length) const;
        protected:
            IOManagerBase* m_iom;
            fs::path       m_filepath;
        };
    }
}
namespace hio = hemlock::io;

#include "iomanager.inl"

#endif // __hemlock_io_iomanager_hpp
