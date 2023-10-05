#ifndef __hemlock_mod_manager_h
#define __hemlock_mod_manager_h

#include "io/iomanager.h"

#include "state.h"

namespace hemlock {
    namespace mod {
        /**
         * @brief Responsible for the registration of mods, and the validation,
         * correction, and registration of load orders.
         */
        class ModManager {
        public:
            // TODO(Matthew): throughout this class, replace bool err with actual err...

            ModManager() { /* Empty. */
            }

            ~ModManager() { /* Empty. */
            }

            void init(hio::IOManagerBase* root_io_manager);
            void dispose();

            /**
             * @brief Accesses the mod directories registered to this mod manager
             * instance.
             *
             * @return The mod directories registered to this mod manager instance.
             */
            const ModDirectories& mod_directories() const { return m_mod_directories; }

            /**
             * @brief Adds the given path as a directory into which a mod may be
             * stored. Any mod that gets downloaded can only be stored in one of these
             * directories.
             *
             * @param path The path to add as a mod directory.
             *
             * @return True if the mod directory was added, false otherwise.
             */
            bool add_mod_directory(const hio::fs::path& path);

            /**
             * @brief Sets the path provided as the default mod directory into which
             * mods that are downloaded will be stored. If this is a path that is not
             * already registered, then it will be added.
             *
             * @param path The path to set as the default mod directory.
             *
             * @return True if the mod directory was newly added, false otherwise.
             */
            bool set_default_mod_directory(const hio::fs::path& path);

            /**
             * @brief Access a mod given by the provided UUID.
             *
             * @param id The UUID of the mod to obtain.
             *
             * @return Pointer to the mod's metadata if the UUID named a registered mod,
             * otherwise nullptr.
             */
            const ModMetadata* get_mod(const boost::uuids::uuid& id) const;

            /**
             * @brief Access the map of all mods keyed by their UUIDs.
             *
             * @return The map of all mods registered keyed by their UUIDs.
             */
            const ModRegistry& get_mods() const { return m_mod_registry; }

            /**
             * @brief Register a mod. Registration of mods entails pointing to a
             * (possibly compressed) mod at some location, optionally expressing a
             * directory choice to ultimately store it at, extracting the mod's metadata
             * and registring the metadata in the registry, and then relocating the mod
             * for storage.
             *
             * @return True if the mod was registered successfully, false otherwise.
             */
            bool register_mod(const hio::fs::path& curr_path);

            /**
             * @brief Access a load order given by the provided UUID.
             *
             * @param id The UUID of the load order to obtain.
             *
             * @return Pointer to the load order if the UUID named a registered load
             * order, otherwise nullptr.
             */
            const LoadOrder* get_load_order(const boost::uuids::uuid& id) const;

            /**
             * @brief Access the map of all load orders keyed by their UUIDs.
             *
             * @return The map of all load orders registered keyed by their UUIDs.
             */
            const LoadOrderRegistry& get_load_orders() const {
                return m_load_order_registry;
            }

            /**
             * @brief Register a load order by passing in a load order struct. This
             * requires validating the load order - i.e. if building a load order with
             * the builder, do not register with this function.
             *
             * @return True if the load order was registered successfully, false
             * otherwise.
             */
            bool register_load_order(const LoadOrder& load_order);

            /**
             * @brief Register a load order by passing in a builder instance. This does
             * not require further validation as the builder is trusted to have
             * validated the load order.
             *
             * @return True if the load order was registered successfully, false
             * otherwise.
             */
            bool register_load_order(const LoadOrderBuilder& load_order_builder);
        protected:
            hio::IOManagerBase* m_root_io_manager;

            ModDirectories m_mod_directories;
            size_t         m_deafult_mod_directory;

            ModRegistry       m_mod_registry;
            LoadOrderRegistry m_load_order_registry;
        };
    }  // namespace mod
}  // namespace hemlock
namespace hmod = hemlock::mod;

#endif  // __hemlock_mod_manager_h
