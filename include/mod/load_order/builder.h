#ifndef __hemlock_mod_load_order_builder_h
#define __hemlock_mod_load_order_builder_h

#include "mod/manager.h"
#include "mod/state.h"

namespace hemlock {
    namespace mod {
        enum class ModState {
            VALID,
            NOT_REGISTERED,
            INCONSISTENT,
            INCOMPATIBLE,
            VERSION_MISMATCH
        };

        class LoadOrderBuilder {
        public:
            LoadOrderBuilder() { /* Empty. */
            }

            ~LoadOrderBuilder() { /* Empty. */
            }

            void init(const ModManager* mod_manager);
            void init(const ModManager* mod_manager, const LoadOrder& load_order);
            void dispose();

            void set_name(std::string&& name);

            void set_description(std::string&& description);

            void set_version(hemlock::SemanticVersion&& version);

            LoadOrderState add_mod(
                const boost::uuids::uuid& id,
                bool                      allow_version_mismatch = false,
                bool                      allow_invaid_order     = true
            );

            // TODO(Matthew): add mod at index in load order.

            // TODO(Matthew): capability to change load order manually, to sort mod
            //                using topological sort, and to get the current mod
            //                ordering.

            /**
             * @brief Get the load order as the builder currently has it.
             *
             * @return The load order the builder is currently holding.
             */
            LoadOrder get_load_order() { return m_load_order; }
        protected:
            const ModManager* m_mod_manager;

            LoadOrder m_load_order;
        };
    }  // namespace mod
}  // namespace hemlock
namespace hmod = hemlock::mod;

#endif  // __hemlock_mod_load_order_builder_h
