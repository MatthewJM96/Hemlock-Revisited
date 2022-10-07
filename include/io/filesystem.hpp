#ifndef __hemlock_io_filesystem_hpp
#define __hemlock_io_filesystem_hpp

namespace hemlock {
    namespace io {
        namespace fs {
            using namespace std::filesystem;

            // TODO(Matthew): Do we abstract these to hide their open calls?
            using mapped_file        = boost::iostreams::mapped_file;
            using mapped_file_source = boost::iostreams::mapped_file_source;
            using mapped_file_sink   = boost::iostreams::mapped_file_sink;

            using paths = std::vector<path>;
        }
    }
}
namespace hio = hemlock::io;

#include "io/fs/glob.h"

#endif // __hemlock_io_filesystem_hpp
