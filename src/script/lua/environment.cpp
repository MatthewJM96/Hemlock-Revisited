#include "stdafx.h"

#include "io/iomanager.h"

#include "script/lua/environment.hpp"

void hscript::lua::Environment::init(hio::IOManagerBase* io_manager, ui32 max_script_length /*= HEMLOCK_DEFAULT_MAX_SCRIPT_LENGTH*/) {
    m_io_manager        = io_manager;
    m_max_script_length = max_script_length;

    // Initialise the Lua environment.
    m_state = luaL_newstate();
    luaL_openlibs(m_state);

    // TODO(Matthew): Error handler.

    // Create a table for storing registered functions.
    lua_newtable(m_state);
    lua_setfield(m_state, LUA_REGISTRYINDEX, H_LUA_SCRIPT_FUNCTION_TABLE);

    // Set global namespace.
    set_global_namespace();

    // TODO(Matthew): Do stuff like this & event subscription.
    // // Create a Lua function registration closure that captures this environment instance.
    // ValueMediator<void*>::push(m_state, (void*)this);
    // addCClosure("RegisterFunction", 1, &registerLFunction);
}

void hscript::lua::Environment::init(EnvironmentBase* parent, hio::IOManagerBase* io_manager, ui32 max_script_length /*= HEMLOCK_DEFAULT_MAX_SCRIPT_LENGTH*/) {
    m_io_manager        = io_manager;
    m_max_script_length = max_script_length;

    // Cast to Lua environment.
    Environment* lua_parent = reinterpret_cast<Environment*>(parent);

    // Initialise the Lua environment.
    m_state = lua_newthread(lua_parent->m_state);

    // Set parent.
    m_parent = lua_parent;

    // Set global namespace.
    set_global_namespace();

    // TODO(Matthew): Anything else?
}

void hscript::lua::Environment::dispose() {
    if (m_state) {
        lua_close(m_state);

        m_state = nullptr;
    }
}

bool hscript::lua::Environment::load(const hio::fs::path& filepath) {
    ui32 length = std::numeric_limits<ui32>::max();
    const char* script = m_io_manager->read_file_to_string(filepath, &length);

    if (!script || length > m_max_script_length) {
        return false;
    }

    bool success = luaL_loadstring(m_state, script) == 0;
    delete[] script;

    return success;
}

bool hscript::lua::Environment::load(const std::string& script) {
    // Load script, returning success state.
    return luaL_loadstring(m_state, script.c_str()) == 0;
}

bool hscript::lua::Environment::run() {
    // Run Lua function currently on top of Lua state stack, returning success state.
    return lua_pcall(m_state, 0, LUA_MULTRET, 0) == 0;
}

bool hscript::lua::Environment::run(const hio::fs::path& filepath) {
    ui32 length = std::numeric_limits<ui32>::max();
    const char* script = m_io_manager->read_file_to_string(filepath, &length);

    if (!script || length > m_max_script_length) {
        return false;
    }

    bool success = luaL_dostring(m_state, script) == 0;
    delete[] script;

    return success;
}

bool hscript::lua::Environment::run(const std::string& script) {
    // Load and run script, and return success state.
    return luaL_dostring(m_state, script.c_str()) == 0;
}

void hscript::lua::Environment::set_global_namespace() {
    lua_pushglobaltable(m_state);
}

void hscript::lua::Environment::push_namespace(const std::string& _namespace) {
    // See if a field already exists with the key of this namespace.
    lua_getfield(m_state, -1, _namespace.c_str());
    // If it doesn't, we can add it.
    if (lua_isnil(m_state, -1)) {
        // Pop the result of looking for the namespace.
        lua_pop(m_state, 1);
        // Add a clean table to the stack.
        lua_newtable(m_state);
        // Add this clean table to the previous table as a field with key as the namespace.
        lua_setfield(m_state, -2, _namespace.c_str());
        // Get the new namespace back on top of the stack (lua_setfield pops the top value on the stack).
        lua_getfield(m_state, -1, _namespace.c_str());
    }
}
