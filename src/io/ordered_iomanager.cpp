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
    const { }

bool hio::OrderedIOManager::assure_path(
    const fs::path& path,
    OUT fs::path& full_path,
    bool          is_file /*= false*/,
    OUT bool*     was_existing /*= nullptr*/
) const { }
