#include "stdafx.h"

#include "io/ordered_iomanager.h"

void hio::OrderedIOManager::init(const std::vector<fs::path>& search_directories) {
    m_search_directories = search_directories;
}

void hio::OrderedIOManager::init(std::vector<fs::path>&& search_directories) {
    m_search_directories = std::move(search_directories);
}

void hio::OrderedIOManager::dispose() {
    std::vector<fs::path>().swap(m_search_directories);
}

bool hio::OrderedIOManager::resolve_path(const fs::path& path, OUT fs::path& full_path)
    const {
    for (const auto& base : m_search_directories) {
        auto candidate_path = fs::absolute(base / path);

        if (fs::exists(candidate_path)) {
            full_path = candidate_path;

            return true;
        }
    }

    return false;
}

bool hio::OrderedIOManager::assure_path(
    const fs::path&, OUT fs::path&, bool /*= false*/, OUT bool* /*= nullptr*/
) const {
    // Cannot assure path via this IO manager, as it cannot know which base to operate
    // with.
    return false;
}
