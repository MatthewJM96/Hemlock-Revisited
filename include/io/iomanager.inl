template <typename ReturnType>
inline ReturnType hio::IOManagerBase::apply_to_path(const fs::path& path, PathOperator<ReturnType> func) const {
    fs::path abs_path{};
    if (!resolve_path(path, abs_path)) {
        if constexpr (std::is_same<ReturnType, void>()) {
            return;
        } else {
            return ReturnType{};
        }
    }

    if constexpr (std::is_same<ReturnType, void>()) {
        func(abs_path);
    } else {
        return func(abs_path);
    }
}

template <typename ReturnType>
    requires ( !std::is_same_v<ReturnType, void> )
inline ReturnType hio::IOManagerBase::apply_to_path(const fs::path& path, PathOperator<ReturnType> func, ReturnType&& default_value) const {
    fs::path abs_path{};
    if (!resolve_path(path, abs_path)) return std::forward<ReturnType>(default_value);

    return func(abs_path);
}

template <typename ReturnType>
inline ReturnType hio::FileReference::apply_to_path(PathOperator<ReturnType> func) const {
    if constexpr (std::is_same<ReturnType, void>()) {
        m_iom->apply_to_path(m_filepath, func);
    } else {
        return m_iom->apply_to_path(m_filepath, func);
    }
}

template <typename ReturnType>
    requires ( !std::is_same_v<ReturnType, void> )
inline ReturnType hio::FileReference::apply_to_path(PathOperator<ReturnType> func, ReturnType&& default_value) const {
    return m_iom->apply_to_path(m_filepath, func, std::forward<ReturnType>(default_value));
}
