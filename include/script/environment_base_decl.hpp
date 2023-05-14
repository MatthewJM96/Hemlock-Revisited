#ifndef __hemlock_script_environment_base_decl_hpp
#define __hemlock_script_environment_base_decl_hpp

namespace hemlock {
    namespace script {
        template <
            typename EnvironmentImpl,
            template <typename, typename>
            typename ContinuableFuncImpl,
            bool   HasRPCManager  = false,
            size_t CallBufferSize = 0>
        class EnvironmentBase;
    }
}  // namespace hemlock
namespace hscript = hemlock::script;

#endif  // __hemlock_script_environment_base_decl_hpp
