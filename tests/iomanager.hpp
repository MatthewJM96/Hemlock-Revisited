#ifndef __hemlock_tests_iomanager_hpp
#define __hemlock_tests_iomanager_hpp

#include "io/iomanager.h"

class MyIOManager : public hio::IOManagerBase {
public:
    virtual bool resolve_path(const hio::fs::path& path, OUT hio::fs::path& full_path) const override {
        full_path = hio::fs::absolute(path);
        return true;
    }
    virtual bool assure_path (  const hio::fs::path& path,
                                    OUT hio::fs::path& full_path,
                                            bool is_file      = false,
                                        OUT bool* was_existing [[maybe_unused]] = nullptr ) const override {
        if (is_file) {
            // bleh
        } else {
            hio::fs::create_directories(path);
        }
        full_path = hio::fs::absolute(path);
        return true;
    }

    virtual bool resolve_paths(IN OUT std::vector<hio::fs::path>& paths) const override {
        bool bad = false;
        for (auto& path : paths) {
            hio::fs::path tmp{};
            bad |= !resolve_path(path, tmp);
            path = tmp;
        }
        return !bad;
    }
    virtual bool assure_paths (IN OUT std::vector<hio::fs::path>& paths) const override {
        bool bad = false;
        for (auto& path : paths) {
            hio::fs::path tmp{};
            bad |= !assure_path(path, tmp);
            path = tmp;
        }
        return !bad;
    }
};

#endif // __hemlock_tests_iomanager_hpp
