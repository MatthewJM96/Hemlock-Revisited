#ifndef __hemlock_mod_mod_h
#define __hemlock_mod_mod_h

#include "io/ordered_iomanager.h"

#include "metadata.h"

namespace hemlock {
    namespace mod {
        class Mod {
        public:
            Mod();
            ~Mod();

            void update(FrameTime time);
        protected:
            ModMetadata m_metadata;

            io::IOManager m_iom;
        };
    }  // namespace mod
}  // namespace hemlock
namespace hmod = hemlock::mod;

#endif  // __hemlock_mod_mod_h
