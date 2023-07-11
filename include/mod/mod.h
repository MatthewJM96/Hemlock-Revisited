#ifndef __hemlock_mod_loader_order_h
#define __hemlock_mod_loader_order_h

#include "io/ordered_iomanager.h"

#include "metadata.h"

namespace hemlock {
    namespace mod {
        class Mod {
        public:
            Mod();
            ~Mod();

            void update(FrameTime time) = 0;
        protected:
            ModMetadata m_metadata;

            io::IOManager m_iom;
        }
    }  // namespace mod
}  // namespace hemlock
namespace hmod = hemlock::mod;

#endif  // __hemlock_mod_loader_order_h
