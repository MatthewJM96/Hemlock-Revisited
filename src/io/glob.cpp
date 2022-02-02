#include "stdafx.h"

#include "io/glob.h"

void hio::glob::impl::handle_all_file_match(PathBuilder& builder) {
    // Build up new list of paths, which we expect to be
    // at least as many in count as the number in the
    // path builder past in.
    PathBuilder new_builder;
    new_builder.reserve(builder.size());

    // For each path in the existing builder, check if
    // if it a directory. If it isn't, skip it, no
    // files can be contained within it. If it is,
    // then for each entry inside, add the path to
    // that entry to the new builder.
    for (const auto& path: builder) {
        if (!hio::fs::is_directory(path)) continue;

        for (const auto& dir_entry: hio::fs::directory_iterator{path}) {
            new_builder.emplace_back(dir_entry);
        }
    }

    // We're done, swap the new builder into the old one.
    new_builder.swap(builder);
}

void hio::glob::impl::handle_recursive_directory_match(PathBuilder& builder) {
    // Build up new list of paths, which we expect to be
    // at least as many in count as the number in the
    // path builder past in.
    PathBuilder new_builder;
    new_builder.reserve(builder.size());

    // For each path in the existing builder, check if
    // if it a directory. If it isn't, skip it, no
    // files can be contained within it. If it is,
    // then for each entry inside, add the path to
    // that entry to the new builder as long as that
    // entry is a directory.
    for (const auto& path: builder) {
        if (!hio::fs::is_directory(path)) continue;

        // Retain the parent directory as a path entry
        // in the new builder as "**" builds a
        // directory-only tree.
        new_builder.emplace_back(path);

        for (const auto& dir_entry: hio::fs::recursive_directory_iterator{path}) {
            if (hio::fs::is_directory(dir_entry)) {
                new_builder.emplace_back(dir_entry);
            }
        }
    }

    // We're done, swap the new builder into the old one.
    new_builder.swap(builder);
}

bool hio::glob::impl::contains_glob_char(const fs::path& part) {
    const std::regex glob_chars_pattern("([^\\\\]\\*|[^\\\\]\\?|[^\\\\]\\[.*[^\\\\]\\])");

    return std::regex_search(part.string(), glob_chars_pattern);
}

void hio::glob::impl::handle_partial_glob(const fs::path& part, PathBuilder& builder) {
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
    // path builder past in.
    PathBuilder new_builder;
    new_builder.reserve(builder.size());

    // For each path in the existing builder, check if
    // if it a directory. If it isn't, skip it, no
    // files can be contained within it. If it is,
    // then for each entry inside, add the path to
    // that entry to the new builder if the final
    // part of the path matches the globstr.
    for (const auto& path: builder) {
        if (!hio::fs::is_directory(path)) continue;

        for (const auto& dir_entry: hio::fs::directory_iterator{path}) {
            if (std::regex_match(
                    dir_entry.path().filename().string(),
                    std::regex(globstr))
                )
            {
                new_builder.emplace_back(dir_entry);
            }
        }
    }

    // We're done, swap the new builder into the old one.
    new_builder.swap(builder);
}

hio::PathBuilder hio::glob::glob(const fs::path& globpath, bool recursive /*= false*/) {
    PathBuilder builder;

    bool offset = false;
    if (globpath.is_absolute()) {
        builder.emplace_back(globpath.root_path());
    } else {
        builder.emplace_back(fs::path{"./"});
        offset = true;
    }

    fs::path relpath = globpath.relative_path();

    auto it = relpath.begin();
    if (offset) ++it;
    for (; it != relpath.end(); ++it) {
        const fs::path& part = *it;

        if (part == "*") {
            impl::handle_all_file_match(builder);
            continue;
        }

        if (part == "**") {
            if (!recursive) return PathBuilder();

            impl::handle_recursive_directory_match(builder);
            continue;
        }

        if (impl::contains_glob_char(part)) {
            impl::handle_partial_glob(part, builder);
            continue;
        }

        for (fs::path& path : builder) {
            path /= part;
        }
    }

    return builder;
}