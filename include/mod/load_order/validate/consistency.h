#ifndef __hemlock_mod_load_order_validate_consistency_h
#define __hemlock_mod_load_order_validate_consistency_h

#include "mod/metadata.h"

namespace hemlock {
    namespace mod {
        /**
         * @brief Validates the provided mod is self-consistent with regards to its
         * load order conditions.
         *
         * @param metadata The metadata of the mod to validate the self-consistency of.
         * @return True if the mod is self-consistent, false otherwise.
         */
        bool is_self_consistent(const hmod::ModMetadata& metadata);
    }  // namespace mod
}  // namespace hemlock
namespace hmod = hemlock::mod;

#endif  // __hemlock_mod_load_order_validate_consistency_h
