#ifndef __hemlock_tests_iomanager_hpp
#define __hemlock_tests_iomanager_hpp

#include "io/iomanager.hpp"

class MyIOManager : public hio::IOManagerBase {
protected:
    virtual bool resolve_path(const hio::fs::path& path, OUT hio::fs::path& full_path, bool is_file = false) const override {
        full_path = hio::fs::absolute(path);

        if (is_file && !hio::fs::is_regular_file(full_path)) return false;

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
};

#endif // __hemlock_tests_iomanager_hpp
