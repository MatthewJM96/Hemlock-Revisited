template <typename Environment>
hscript::EnvironmentRegistry<Environment>& hscript::EnvironmentRegistry<Environment>::operator=(hscript::EnvironmentRegistry<Environment>&& rhs) {
    m_ungrouped_envs = std::move(rhs.m_ungrouped_envs);
    m_next_group_id  = rhs.m_next_group_id;
    m_groups         = std::move(rhs.m_groups);

    return *this;
}

template <typename Environment>
void hscript::EnvironmentRegistry<Environment>::init(hio::IOManagerBase* io_manager, i32 max_script_length/* = HEMLOCK_DEFAULT_MAX_SCRIPT_LENGTH*/) {
    m_io_manager = io_manager;

    m_max_script_length = max_script_length;
}

template <typename Environment>
void hscript::EnvironmentRegistry<Environment>::dispose() {
    for (auto& env : m_ungrouped_envs) {
        env->dispose();

        delete env;
    }
    Environments<Environment>().swap(m_ungrouped_envs);

    for (auto& [id, group] : m_groups) {
        for (auto& env : group.children) {
            env->dispose();

            delete env;
        }

        group.parent->dispose();

        delete group.parent;
    }
    EnvironmentGroups<Environment>().swap(m_groups);
}

template <typename Environment>
hscript::EnvironmentGroupID hscript::EnvironmentRegistry<Environment>::create_group(EnvironmentBuilder<Environment>* builder/* = nullptr*/) {
    Environment* group_env = new Environment();
    group_env->init(m_io_manager, m_max_script_length);

    if (builder != nullptr) {
        builder(group_env);
    }

    m_groups.insert(std::make_pair(m_next_group_id, { group_env, {} }));

    return m_next_group_id++;
}

template <typename Environment>
Environment* hscript::EnvironmentRegistry<Environment>::create_environment(EnvironmentBuilder<Environment>* builder/* = nullptr*/) {
    Environment* env = new Environment();
    env->init(m_io_manager, m_max_script_length);

    if (builder != nullptr) {
        builder(env);
    }

    m_ungrouped_envs.push_back(env);

    return env;
}

template <typename Environment>
Environment* hscript::EnvironmentRegistry<Environment>::create_environment(EnvironmentGroupID group_id) {
    EnvironmentGroup<Environment>& group;
    try {
        group = m_groups.at(group_id);
    } catch (std::out_of_range e) {
        return nullptr;
    }

    Environment* env = new Environment();
    env->init(group.parent, m_io_manager, m_max_script_length);

    group.children.push_back(env);

    return env;
}

template <typename Environment>
Environment* hscript::EnvironmentRegistry<Environment>::create_environments(ui32 num, EnvironmentBuilder<Environment>* builder/* = nullptr*/) {
    Environment* envs = new Environment[num]();

    m_ungrouped_envs.reserve(m_ungrouped_envs.size() + num);

    for (ui32 env_idx = 0; env_idx != num; ++env_idx) {
        Environment* env = &envs[env_idx];

        env->init(m_io_manager, m_max_script_length);

        if (builder != nullptr) {
            builder(env);
        }

        m_ungrouped_envs.push_back(env);
    }

    return envs;
}

template <typename Environment>
Environment* hscript::EnvironmentRegistry<Environment>::create_environments(ui32 num, EnvironmentGroupID group_id) {
    EnvironmentGroup<Environment>& group;
    try {
        group = m_groups.at(group_id);
    } catch (std::out_of_range e) {
        return nullptr;
    }

    Environment* envs = new Environment[num]();

    group.children.reserve(group.children.size() + num);

    for (ui32 env_idx = 0; env_idx != num; ++env_idx) {
        Environment* env = &envs[env_idx];

        env->init(group.parent, m_io_manager, m_max_script_length);

        group.children.push_back(env);
    }

    return envs;
}
