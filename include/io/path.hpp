#ifndef __hemlock_io_path_hpp
#define __hemlock_io_path_hpp

namespace hemlock {
    namespace io {
        using Path = fs::path;

        using PathBuilder = std::vector<Path>;
    }
}
namespace hio = hemlock::io;

#endif // __hemlock_io_path_hpp
