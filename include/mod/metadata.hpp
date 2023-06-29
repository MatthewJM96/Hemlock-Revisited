#ifndef __hemlock_mod_metadata_hpp
#define __hemlock_mod_metadata_hpp

namespace hemlock {
    namespace mod {
        using ModID = ui64;

        struct ModMetadataShort {
            ModID id;

            std::array<char, 256> name;
            std::array<char, 256> author;

            // Last updated?
            // Version?
        };

        struct ModMetadata : public ModMetadataShort {
            std::string description;

            // ???
        };
    }  // namespace mod
}  // namespace hemlock
namespace hmod = hemlock::mod;

#endif  // __hemlock_mod_metadata_hpp
