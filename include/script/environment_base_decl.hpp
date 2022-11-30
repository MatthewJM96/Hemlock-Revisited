#ifndef __hemlock_script_environment_base_decl_hpp
#define __hemlock_script_environment_base_decl_hpp

namespace hemlock {
    namespace script {
        template <
            typename EnvironmentImpl,
            bool HasRPCManager = false,
            size_t CallBufferSize = 0
        >
        class EnvironmentBase;
    }
}
namespace hscript = hemlock::script;

#endif // __hemlock_script_environment_base_decl_hpp