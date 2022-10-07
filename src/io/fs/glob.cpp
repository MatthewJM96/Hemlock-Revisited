#include "stdafx.h"

#include "io/fs/glob.h"

void hio::fs::glob::impl::handle_all_file_match(paths& path_set) {
    // Build up new list of paths, which we expect to be
    // at least as many in count as the number in the
    // path path_set past in.
    paths new_path_set;
    new_path_set.reserve(path_set.size());

    // For each path in the existing path_set, check if
    // if it a directory. If it isn't, skip it, no
    // files can be contained within it. If it is,
    // then for each entry inside, add the path to
    // that entry to the new path_set.
    for (const auto& path: path_set) {
        if (!is_directory(path)) continue;

        for (const auto& dir_entry: directory_iterator{path}) {
            new_path_set.emplace_back(dir_entry);
        }
    }

    // We're done, swap the new path_set into the old one.
    new_path_set.swap(path_set);
}

void hio::fs::glob::impl::handle_recursive_directory_match(paths& path_set) {
    // Build up new list of paths, which we expect to be
    // at least as many in count as the number in the
    // path path_set past in.
    paths new_path_set;
    new_path_set.reserve(path_set.size());

    // For each path in the existing path_set, check if
    // if it a directory. If it isn't, skip it, no
    // files can be contained within it. If it is,
    // then for each entry inside, add the path to
    // that entry to the new path_set as long as that
    // entry is a directory.
    for (const auto& path: path_set) {
        if (!is_directory(path)) continue;

        // Retain the parent directory as a path entry
        // in the new path_set as "**" builds a
        // directory-only tree.
        new_path_set.emplace_back(path);

        for (const auto& dir_entry: recursive_directory_iterator{path}) {
            if (is_directory(dir_entry)) {
                new_path_set.emplace_back(dir_entry);
            }
        }
    }

    // We're done, swap the new path_set into the old one.
    new_path_set.swap(path_set);
}

bool hio::fs::glob::impl::contains_glob_char(const path& part) {
    const std::regex glob_chars_pattern("([^\\\\]\\*|[^\\\\]\\?|[^\\\\]\\[.*[^\\\\]\\])");

    return std::regex_search(part.string(), glob_chars_pattern);
}

void hio::fs::glob::impl::handle_partial_glob(const path& part, paths& path_set) {
    // Define our regex strings for glob patterns.
    const std::regex      asterisk("([^\\\\]\\*|^\\*)");
    const std::regex question_mark("([^\\\\]\\?)");
    const std::regex    not_any_of("([^\\\\]\\[\\!(.*[^\\\\])\\])");

    // Define our replacement strings, converting
    // glob patterns to equivalent regex patterns.
    const std::string      asterisk_replace(".+");
    const std::string question_mark_replace(".{1}");
    const std::string    not_any_of_replace("[^$2]");

    // Replace all glob patterns in this path part
    // with their equivalent regex patterns.
    std::string globstr = part.string();
    globstr = std::regex_replace(globstr,      asterisk,      asterisk_replace);
    globstr = std::regex_replace(globstr, question_mark, question_mark_replace);
    globstr = std::regex_replace(globstr,    not_any_of,    not_any_of_replace);

    // Build up new list of paths, which we expect to be
    // at least as many in count as the number in the
    // path path_set past in.
    paths new_path_set;
    new_path_set.reserve(path_set.size());

    // For each path in the existing path_set, check if
    // if it a directory. If it isn't, skip it, no
    // files can be contained within it. If it is,
    // then for each entry inside, add the path to
    // that entry to the new path_set if the final
    // part of the path matches the globstr.
    for (const auto& path: path_set) {
        if (!is_directory(path)) continue;

        for (const auto& dir_entry: directory_iterator{path}) {
            if (std::regex_match(
                    dir_entry.path().filename().string(),
                    std::regex(globstr))
                )
            {
                new_path_set.emplace_back(dir_entry);
            }
        }
    }

    // We're done, swap the new path_set into the old one.
    new_path_set.swap(path_set);
}

hio::fs::paths hio::fs::glob::glob(const fs::path& globpath, bool recursive /*= false*/) {
    paths path_set;

    bool offset = false;
    if (globpath.is_absolute()) {
        path_set.emplace_back(globpath.root_path());
    } else {
        path_set.emplace_back(path{"./"});
        offset = true;
    }

    path relpath = globpath.relative_path();

    auto it = relpath.begin();
    if (offset) ++it;
    for (; it != relpath.end(); ++it) {
        const fs::path& part = *it;

        if (part == "*") {
            impl::handle_all_file_match(path_set);
            continue;
        }

        if (part == "**") {
            if (!recursive) return paths();

            impl::handle_recursive_directory_match(path_set);
            continue;
        }

        if (impl::contains_glob_char(part)) {
            impl::handle_partial_glob(part, path_set);
            continue;
        }

        for (path& incomplet_path : path_set) {
            incomplet_path /= part;
        }
    }

    return path_set;
}
