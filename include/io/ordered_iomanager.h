#ifndef __hemlock_io_orderered_iomanager_h
#define __hemlock_io_orderered_iomanager_h

#include "iomanager.h"

namespace hemlock {
    namespace io {
        /**
         * @brief OrderedIOManager implements ordered search through a set of base
         * directories. When a resource is located in any base directory, a give search
         * ends, so where resources from one base directory should override those in
         * another base directory, the former directory should occur before the latter
         * in the list of base directories.
         */
        class OrderedIOManager : public IOManagerBase {
        public:
            OrderedIOManager() { /* Empty. */
            }

            virtual ~OrderedIOManager() { /* Empty. */
            }

            /**
             * @brief Initialise the IO manager with the base directories to search in.
             *
             * @param search_directories The base directories to search in in priority
             * order.
             */
            void init(const std::vector<fs::path>& search_directories);
            /**
             * @brief Initialise the IO manager with the base directories to search in.
             *
             * @param search_directories The base directories to search in in priority
             * order.
             */
            void init(std::vector<fs::path>&& search_directories);
            /**
             * @brief Dispose the IO manager.
             */
            void dispose();

            /**
             * @brief Resolve a relative filepath into a valid absolute filepath.
             * Permissibility of access is not implemented here, rather should this
             * function succeed then permissibility is assumed.
             *
             * @param path The relative filepath to resolve.
             * @param full_path The absolute filepath if successfully resolved.
             * @return True if the path was resolved successfully, false otherwise.
             */
            bool resolve_path(const fs::path& path, OUT fs::path& full_path)
                const override final;
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
            bool assure_path(
                const fs::path& path,
                OUT fs::path& full_path,
                bool          is_file      = false,
                OUT bool*     was_existing = nullptr
            ) const override final;
        protected:
            std::vector<fs::path> m_search_directories;
        };
    }  // namespace io
}  // namespace hemlock
namespace hio = hemlock::io;

#endif  // __hemlock_io_ordered_iomanager_h
