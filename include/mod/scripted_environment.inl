template <typename ScriptEnvironment>
hmod::ScriptModEnvironment<ScriptEnvironment>::ScriptModEnvironment() :
    m_is_active(false), m_base_iom(nullptr) { /* Empty. */
}

template <typename ScriptEnvironment>
void hmod::ScriptModEnvironment<ScriptEnvironment>::init(
    ModRegistry&&                   registry,
    LoadOrder&&                     load_order,
    hmem::Handle<ScriptEnvironment> script_environment
) {
    init(registry, load_order);

    m_script_environment = script_environment;
}

template <typename ScriptEnvironment>
void hmod::ScriptModEnvironment<ScriptEnvironment>::dispose() {
    hmod::ModEnvironment::dispose();

    m_script_environment = {};
}

template <typename ScriptEnvironment>
void hmod::ScriptModEnvironment<ScriptEnvironment>::update(FrameTime time) { }

template <typename ScriptEnvironment>
bool hmod::ScriptModEnvironment<ScriptEnvironment>::set_script_environment(
    hmem::Handle<ScriptEnvironment> script_environment
) {
    if (m_is_active) return false;

    m_script_environment = script_environment;

    return true;
}
