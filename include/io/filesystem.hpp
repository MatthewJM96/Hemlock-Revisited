#ifndef __hemlock_io_filesystem_hpp
#define __hemlock_io_filesystem_hpp

// TODO(Matthew): Do we abstract these to hide their open calls?
namespace std::filesystem {
    using mapped_file        = boost::iostreams::mapped_file;
    using mapped_file_source = boost::iostreams::mapped_file_source;
    using mapped_file_sink   = boost::iostreams::mapped_file_sink;
};

namespace hemlock {
    namespace io {
        namespace fs = std::filesystem;
    }
}
namespace hio = hemlock::io;

#endif // __hemlock_io_filesystem_hpp
