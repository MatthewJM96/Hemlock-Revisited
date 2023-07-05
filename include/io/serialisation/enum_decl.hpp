#ifndef __hemlock_io_serialisation_enum_decl_hpp
#define __hemlock_io_serialisation_enum_decl_hpp

namespace hemlock {
    namespace io {
        template <typename Type>
        constexpr const char* serialisable_enum_name(Type val);

        template <typename Type>
        constexpr Type serialisable_enum_val(const std::string& name);
    }  // namespace io
}  // namespace hemlock
namespace hio = hemlock::io;

#endif  // __hemlock_io_serialisation_enum_decl_hpp
