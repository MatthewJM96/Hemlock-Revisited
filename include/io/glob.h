#ifndef __hemlock_io_glob_h
#define __hemlock_io_glob_h

namespace hemlock {
    namespace io {
        namespace glob {
            namespace impl {
                /**
                 * @brief For each path in the builder, replace it
                 * with all entries contained within it if that
                 * path is a directory, otherwise remove the path.
                 * 
                 * @param builder The builder to process.
                 */
                void handle_all_file_match(PathBuilder& builder);

                /**
                 * @brief For each path in the builder, add all
                 * directories contained within it if that entry
                 * is a directory, if any path in the builder is
                 * not a directory, remove it.
                 * 
                 * @param builder The builder to process.
                 */
                void handle_recursive_directory_match(PathBuilder& builder);

                /**
                 * @brief Determine if the path part passed in
                 * contains any special characters relating to
                 * globbing.
                 * 
                 * @param part The part to assess.
                 * @return True if the part does contain a glob
                 * special character, false if not.
                 */
                bool contains_glob_char(const fs::path& part);

                /**
                 * @brief For each path in the builder, replace
                 * it with all entries contained within it whose
                 * filename matches the glob string passed in.
                 * 
                 * @param part The path part that is the glob
                 * string to be used for pattern matching
                 * @param builder The builder to proces.
                 */
                void handle_partial_glob(const fs::path& part, PathBuilder& builder);
            }

            PathBuilder glob(const fs::path& globpath, bool recursive = false);
        }
    }
}
namespace hio = hemlock::io;

#endif // __hemlock_io_glob_h
