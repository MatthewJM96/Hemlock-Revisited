template <typename ScriptEnvironment>
hmod::ScriptModEnvironment<ScriptEnvironment>::ScriptModEnvironment() :
    m_is_active(false), m_base_iom(nullptr) { /* Empty. */
}

template <typename ScriptEnvironment>
void hmod::ScriptModEnvironment<ScriptEnvironment>::init(
    ModRegistry&&                registry,
    LoadOrder&&                  load_order,
    _ModScriptEnvironmentBuilder script_environment_builder
) {
    init(registry, load_order);

    m_script_env_registry.init();

    m_script_env_builder = std::move(script_environment_builder);
}

template <typename ScriptEnvironment>
void hmod::ScriptModEnvironment<ScriptEnvironment>::dispose() {
    hmod::ModEnvironment::dispose();

    m_script_env_builder = {};

    m_script_env_registry.dispose();
}

template <typename ScriptEnvironment>
void hmod::ScriptModEnvironment<ScriptEnvironment>::activate_environment() { }

template <typename ScriptEnvironment>
void hmod::ScriptModEnvironment<ScriptEnvironment>::deactivate_environment() { }

template <typename ScriptEnvironment>
void hmod::ScriptModEnvironment<ScriptEnvironment>::update(FrameTime time) { }

template <typename ScriptEnvironment>
bool hmod::ScriptModEnvironment<ScriptEnvironment>::set_script_environment_builder(
    _ModScriptEnvironmentBuilder builder
) {
    if (m_is_active) return false;

    m_script_env_builder = builder;

    return true;
}
