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
void hmod::ScriptModEnvironment<ScriptEnvironment>::activate() {
    // Load each mod's metadata.

    // Validate load order.

    // For each mod, run load sequence (TBD)
    //   Load in any static data that should be preloaded.
    //   Run an on_load (TBN) script of that mod.
    //     Mod should register various functions including its update function in this.
    //       Some functions like update function if not registered will be checked for
    //       as some standardised name. Mod invalid if this does not exist and metadata
    //       suggests it has an updatable script component.

    // Activation successful if all mods completed load sequence correctly.
}

template <typename ScriptEnvironment>
void hmod::ScriptModEnvironment<ScriptEnvironment>::deactivate() { }

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
