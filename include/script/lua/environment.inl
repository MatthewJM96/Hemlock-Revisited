#include "io/iomanager.h"
#include "script/rpc_manager.hpp"

#include "c_function.hpp"
#include "lua_value.hpp"

template <bool HasRPCManager, size_t CallBufferSize>
void hscript::lua::Environment<HasRPCManager, CallBufferSize>::init(
    hio::IOManagerBase*                io_manager,
    EnvironmentRegistry<_Environment>* registry /*= nullptr*/,
    ui32 max_script_length /*= HEMLOCK_DEFAULT_MAX_SCRIPT_LENGTH*/
) {
    _Base::m_io_manager        = io_manager;
    _Base::m_registry          = registry;
    _Base::m_max_script_length = max_script_length;

    // Initialise the Lua environment.
    m_state = luaL_newstate();
    luaL_openlibs(m_state);

    // TODO(Matthew): Error handler.

    // Create a table for storing registered functions.
    lua_newtable(m_state);
    lua_setfield(m_state, LUA_REGISTRYINDEX, HEMLOCK_LUA_SCRIPT_FUNCTION_TABLE);

    // Set global namespace.
    set_global_namespace();

    // Expose Lua function registration to environment.
    add_c_function(
        "register_function",
        &hscript::lua::register_lua_function<HasRPCManager, CallBufferSize>,
        this
    );

    if constexpr (HasRPCManager) {
        _Base::rpc.init(this);

        set_namespaces("foreign");
        add_c_function("call", &call_foreign<CallBufferSize>, this);
        add_c_function("query", &query_foreign_call<CallBufferSize>, this);
        add_c_function("get_results", &get_foreign_call_results<CallBufferSize>, this);

        add_c_function(
            "set_manual_command_buffer_pump",
            &set_manual_command_buffer_pump<CallBufferSize>,
            this
        );
        add_c_function(
            "pump_command_buffer", &pump_command_buffer<CallBufferSize>, this
        );
        set_global_namespace();
    }

    // TODO(Matthew): Do stuff like event subscription.
}

template <bool HasRPCManager, size_t CallBufferSize>
void hscript::lua::Environment<HasRPCManager, CallBufferSize>::init(
    EnvironmentBase<
        Environment<HasRPCManager, CallBufferSize>,
        HasRPCManager,
        CallBufferSize>*               parent,
    hio::IOManagerBase*                io_manager,
    EnvironmentRegistry<_Environment>* registry /*= nullptr*/,
    ui32 max_script_length /*= HEMLOCK_DEFAULT_MAX_SCRIPT_LENGTH*/
) {
    _Base::m_io_manager        = io_manager;
    _Base::m_registry          = registry;
    _Base::m_max_script_length = max_script_length;

    // Cast to Lua environment.
    Environment* lua_parent = reinterpret_cast<Environment*>(parent);

    // Initialise the Lua environment.
    m_state = lua_newthread(lua_parent->m_state);

    // Set parent.
    m_parent = lua_parent;

    // Set global namespace.
    set_global_namespace();

    if constexpr (HasRPCManager) {
        _Base::rpc.init(this);
    }

    // TODO(Matthew): Anything else?
}

template <bool HasRPCManager, size_t CallBufferSize>
void hscript::lua::Environment<HasRPCManager, CallBufferSize>::dispose() {
    if (m_state) {
        lua_close(m_state);

        m_state = nullptr;
    }
}

template <bool HasRPCManager, size_t CallBufferSize>
bool hscript::lua::Environment<HasRPCManager, CallBufferSize>::load(
    const hio::fs::path& filepath
) {
    ui32        length = std::numeric_limits<ui32>::max();
    const char* script = _Base::m_io_manager->read_file_to_string(filepath, &length);

    if (!script || length > _Base::m_max_script_length) {
        return false;
    }

    bool success = luaL_loadstring(m_state, script) == 0;
    delete[] script;

    return success;
}

template <bool HasRPCManager, size_t CallBufferSize>
bool hscript::lua::Environment<HasRPCManager, CallBufferSize>::load(
    const std::string& script
) {
    // Load script, returning success state.
    return luaL_loadstring(m_state, script.c_str()) == 0;
}

template <bool HasRPCManager, size_t CallBufferSize>
bool hscript::lua::Environment<HasRPCManager, CallBufferSize>::run() {
    // Run Lua function currently on top of Lua state stack, returning success state.
    return lua_pcall(m_state, 0, LUA_MULTRET, 0) == 0;
}

template <bool HasRPCManager, size_t CallBufferSize>
bool hscript::lua::Environment<HasRPCManager, CallBufferSize>::run(
    const hio::fs::path& filepath
) {
    ui32        length = std::numeric_limits<ui32>::max();
    const char* script = _Base::m_io_manager->read_file_to_string(filepath, &length);

    if (!script || length > _Base::m_max_script_length) {
        return false;
    }

    bool success = luaL_dostring(m_state, script) == 0;
    delete[] script;

    return success;
}

template <bool HasRPCManager, size_t CallBufferSize>
bool hscript::lua::Environment<HasRPCManager, CallBufferSize>::run(
    const std::string& script
) {
    // Load and run script, and return success state.
    return luaL_dostring(m_state, script.c_str()) == 0;
}

template <bool HasRPCManager, size_t CallBufferSize>
void hscript::lua::Environment<HasRPCManager, CallBufferSize>::set_global_namespace() {
    lua_pop(m_state, m_namespace_depth);

    lua_pushglobaltable(m_state);

    m_namespace_depth = 1;
}

template <bool HasRPCManager, size_t CallBufferSize>
void hscript::lua::Environment<HasRPCManager, CallBufferSize>::push_namespace(
    const std::string& _namespace
) {
    // See if a field already exists with the key of this namespace.
    lua_getfield(m_state, -1, _namespace.c_str());
    // If it doesn't, we can add it.
    if (lua_isnil(m_state, -1)) {
        // Pop the result of looking for the namespace.
        lua_pop(m_state, 1);
        // Add a clean table to the stack.
        lua_newtable(m_state);
        // Add this clean table to the previous table as a field with key as the
        // namespace.
        lua_setfield(m_state, -2, _namespace.c_str());
        // Get the new namespace back on top of the stack (lua_setfield pops the top
        // value on the stack).
        lua_getfield(m_state, -1, _namespace.c_str());
    }

    ++m_namespace_depth;
}

template <bool HasRPCManager, size_t CallBufferSize>
bool hscript::lua::Environment<HasRPCManager, CallBufferSize>::register_lua_function(
    std::string&& name, OUT LuaFunctionState* state /* = nullptr*/
) {
    if (name.length() == 0) return false;

    auto& cache = m_lua_functions;
    if (m_parent) cache = m_parent->m_lua_functions;

    // Check cache for whether this function is already registered.
    if (cache.contains(name)) {
        if (state) *state = m_lua_functions[name];

        return true;
    }

    /*
     * The general idea here is that we register a lua function by
     * creating an entry for it in the script function table we
     * attached to the Lua registry. This table is only ever appended
     * to via `luaL_ref` in order that Lua can handle unique
     * identifiers for all registered functions. To avoid creating
     * duplicate registry entries for the same function, and to
     * speed up access to Lua functions we register, we therefore
     * also keep a cache of the information necessary to create
     * a C-side callable to invoke registered Lua functions.
     */

    i32 prior_index = lua_gettop(m_state);

    // Get our script function table in the Lua registry and place
    // it on the Lua stack.
    lua_getfield(m_state, LUA_REGISTRYINDEX, HEMLOCK_LUA_SCRIPT_FUNCTION_TABLE);

    // Now put global namespace on the stack, we will be searching
    // for the Lua function to register from here.
    lua_pushglobaltable(m_state);

    // Iterate through namesapces needed to get to the function we
    // are registering.
    size_t last_idx = 0;
    size_t next_idx = name.find('.');
    while (true) {
        // Determine the next token in the string naming the function.
        std::string_view token
            = std::string_view(name).substr(last_idx, next_idx - last_idx);

        // Push token onto stack, naming the entry in the current table
        // on the stack that should lead to the function we are registering.
        lua_pushlstring(m_state, token.data(), token.size());
        // Grab the next namespace (or function) from the previous' table.
        lua_gettable(m_state, -2);
        // Check that the object we just retrieved exists.
        if (lua_isnil(m_state, -1)) return false;

        // Break out if we just handled the last token in the function
        // name.
        if (next_idx == std::string::npos) break;

        last_idx = next_idx + 1;
        next_idx = name.find('.', last_idx);
    }

    // Check that the last object added to the stack
    // was a valid function, if it wasn't then we have
    // failed.
    if (!lua_isfunction(m_state, -1)) return false;

    // Place a reference to the function in the script
    // function table of the Lua registry, and cache
    // this.
    if (state) {
        *state = { .state = m_state, .index = luaL_ref(m_state, prior_index + 1) };
        cache[std::move(name)] = *state;
    } else {
        cache[std::move(name)]
            = { .state = m_state, .index = luaL_ref(m_state, prior_index + 1) };
    }

    // Return stack to state prior to function registration.
    i32 final_index = lua_gettop(m_state);
    lua_pop(m_state, final_index - prior_index);

    return true;
}

template <bool HasRPCManager, size_t CallBufferSize>
bool hscript::lua::Environment<HasRPCManager, CallBufferSize>::
    register_continuable_lua_function(
        std::string&& name, OUT LuaFunctionState* state /*= nullptr*/
    ) {
    if (name.length() == 0) return false;

    auto& cache = m_lua_continuable_functions;
    if (m_parent) cache = m_parent->m_lua_continuable_functions;

    // Check cache for whether this function is already registered.
    if (cache.contains(name)) {
        if (state) *state = m_lua_continuable_functions[name];

        return true;
    }

    i32 prior_index = lua_gettop(m_state);

    // Get our script function table in the Lua registry and place
    // it on the Lua stack.
    lua_getfield(
        m_state, LUA_REGISTRYINDEX, HEMLOCK_LUA_CONTINUABLE_SCRIPT_FUNCTION_TABLE
    );

    LuaHandle        continuable_thread = lua_newthread(m_state);
    LuaFunctionState continuable_func_state
        = { .state = continuable_thread, .index = luaL_ref(m_state, prior_index + 1) };

    // Now put global namespace on the stack, we will be searching
    // for the Lua function to register from here.
    lua_pushglobaltable(m_state);

    // Iterate through namesapces needed to get to the function we
    // are registering.
    size_t last_idx = 0;
    size_t next_idx = name.find('.');
    while (true) {
        // Determine the next token in the string naming the function.
        std::string_view token
            = std::string_view(name).substr(last_idx, next_idx - last_idx);

        // Push token onto stack, naming the entry in the current table
        // on the stack that should lead to the function we are registering.
        lua_pushlstring(m_state, token.data(), token.size());
        // Grab the next namespace (or function) from the previous' table.
        lua_gettable(m_state, -2);
        // Check that the object we just retrieved exists.
        if (lua_isnil(m_state, -1)) return false;

        // Break out if we just handled the last token in the function
        // name.
        if (next_idx == std::string::npos) break;

        last_idx = next_idx + 1;
        next_idx = name.find('.', last_idx);
    }

    // Check that the last object added to the stack
    // was a valid function, if it wasn't then we have
    // failed.
    if (!lua_isfunction(m_state, -1)) return false;

    // Move function over to new thread.
    lua_xmove(m_state, continuable_thread, 1);

    // Place a reference to the function in the script
    // function table of the Lua registry, and cache
    // this.
    if (state) {
        *state                 = continuable_func_state;
        cache[std::move(name)] = *state;
    } else {
        cache[std::move(name)] = continuable_func_state;
    }

    // Return stack to state prior to function registration.
    i32 final_index = lua_gettop(m_state);
    lua_pop(m_state, final_index - prior_index);

    return true;
}

template <bool HasRPCManager, size_t CallBufferSize>
template <typename... Strings>
void hscript::lua::Environment<HasRPCManager, CallBufferSize>::set_namespaces(
    Strings... namespaces
) {
    set_global_namespace();

    for (const auto& _namespace : { namespaces... }) {
        push_namespace(_namespace);
    }
}

template <bool HasRPCManager, size_t CallBufferSize>
template <typename... Strings>
void hscript::lua::Environment<HasRPCManager, CallBufferSize>::enter_namespaces(
    Strings... namespaces
) {
    for (const auto& _namespace : { namespaces... }) {
        push_namespace(_namespace);
    }
}

template <bool HasRPCManager, size_t CallBufferSize>
template <typename Type>
void hscript::lua::Environment<HasRPCManager, CallBufferSize>::add_value(
    std::string_view name, Type val
) {
    if (m_parent) return m_parent->add_value(name, val);

    const i32 value_count = static_cast<i32>(LuaValue<Type>::value_count());

    // TODO(Matthew): Do we want to support some degree of greater
    //                flexibility?
    //                  E.g. support putting PODs in with some template
    //                  magic.
    //                For now we allow this to be done explicitly with
    //                use of enter_namespaces, set_namespaces.

    if constexpr (value_count == 1) {
        // In case of adding one value only, just push it onto the
        // Lua stack and then set it as a value under the field below
        // it on the stack, the namespace it will sit in.
        lua_pushlstring(m_state, name.data(), name.size());
        LuaValue<Type>::push(m_state, val);
        lua_settable(m_state, -3);
    } else {
        // In case of adding multiple values, we first push the name
        // to give the value as a namespace, then push the value onto
        // the stack as multiple elements. For each element we attach
        // it with a numeric index to the namespace we just created.
        //   For example, if the value was a i32v3, and the name was
        //   to be "position", then under whatever namespace was set
        //   on calling add_value, a table would be added with form:
        //     <current_namespace>
        //       position
        //         0 = val.x
        //         1 = val.y
        //         2 = val.z
        push_namespace(name);
        LuaValue<Type>::push(m_state, val);
        for (i32 idx = value_count; idx > 0; --idx) {
            lua_setfield(m_state, -(idx + 1), std::to_string(idx).c_str());
        }
        // Make sure to pop the namespace we pushed, to avoid side
        // effects on the Lua stack.
        lua_pop(m_state, 1);
        --m_namespace_depth;
    }
}

template <bool HasRPCManager, size_t CallBufferSize>
template <typename ReturnType, typename... Parameters>
void hscript::lua::Environment<HasRPCManager, CallBufferSize>::add_c_delegate(
    std::string_view name, Delegate<ReturnType, Parameters...>* delegate
) {
    if (m_parent) return m_parent->add_c_delegate(name, delegate);

    lua_pushlstring(m_state, name.data(), name.size());

    LuaValue<void*>::push(m_state, reinterpret_cast<void*>(delegate));

    lua_pushcclosure(m_state, invoke_delegate<ReturnType, Parameters...>, 1);

    lua_settable(m_state, -3);
}

template <bool HasRPCManager, size_t CallBufferSize>
template <typename ReturnType, typename... Parameters>
void hscript::lua::Environment<HasRPCManager, CallBufferSize>::add_c_function(
    std::string_view name, ReturnType (*func)(Parameters...)
) {
    if (m_parent) return m_parent->add_c_function(name, func);

    lua_pushlstring(m_state, name.data(), name.size());

    LuaValue<void*>::push(m_state, reinterpret_cast<void*>(func));

    lua_pushcclosure(m_state, invoke_function<ReturnType, Parameters...>, 1);

    lua_settable(m_state, -3);
}

template <bool HasRPCManager, size_t CallBufferSize>
template <typename... Upvalues>
void hscript::lua::Environment<HasRPCManager, CallBufferSize>::add_c_function(
    std::string_view name, i32 (*func)(LuaHandle), Upvalues... upvalues
) {
    if (m_parent)
        return m_parent->add_c_function(
            name, func, std::forward<Upvalues>(upvalues)...
        );

    lua_pushlstring(m_state, name.data(), name.size());

    (LuaValue<Upvalues>::push(m_state, upvalues), ...);
    LuaValue<void*>::push(m_state, reinterpret_cast<void*>(func));

    i32 value_count = 1 + total_value_count<Upvalues...>();
    lua_pushcclosure(m_state, func, value_count);

    lua_settable(m_state, -3);
}

template <bool HasRPCManager, size_t CallBufferSize>
template <std::invocable Closure, typename ReturnType, typename... Parameters>
void hscript::lua::Environment<HasRPCManager, CallBufferSize>::add_c_closure(
    std::string_view name, Closure* closure, ReturnType (Closure::*func)(Parameters...)
) {
    if (m_parent) return m_parent->add_c_closure(name, closure, func);

    lua_pushlstring(m_state, name.data(), name.size());

    LuaValue<void*>::push(m_state, reinterpret_cast<void*>(closure));
    LuaValue<decltype(func)>::push(m_state, func);

    lua_pushcclosure(m_state, invoke_closure<Closure, ReturnType, Parameters...>, 2);

    lua_settable(m_state, -3);
}

template <bool HasRPCManager, size_t CallBufferSize>
template <typename ReturnType, typename... Parameters>
bool hscript::lua::Environment<HasRPCManager, CallBufferSize>::get_script_function(
    std::string&& name, OUT ScriptDelegate<ReturnType, Parameters...>& delegate
) {
    // Try to obtain the named Lua function, registering it
    // if it was not already registered. If we could not
    // obtain it, report failure.
    LuaFunctionState lua_func_state;
    if (!register_lua_function(std::move(name), &lua_func_state)) {
        return false;
    }

    // Call correct delegate builder depending on if we ware allowing arbitrary
    // parameters or return types.
    if constexpr (std::is_same_v<ReturnType, CallParameters> && std::equal_to<size_t>()(sizeof...(Parameters), 1ul) && (std::is_same_v<Parameters, CallParameters> || ...))
    {
        delegate = make_arbitrary_scalars_lua_delegate(
            lua_func_state, HEMLOCK_MAX_ARBITRARY_LUA_RETURNS
        );
    } else {
        delegate = make_lua_delegate<ReturnType, Parameters...>(lua_func_state);
    }

    return true;
}

template <bool HasRPCManager, size_t CallBufferSize>
template <typename NewCallSignature, typename ContinuationCallSignature>
bool hscript::lua::Environment<HasRPCManager, CallBufferSize>::
    get_continuable_script_function(
        std::string&& name,
        OUT           ContinuableFunction<NewCallSignature, ContinuationCallSignature>&
                      continuable_function
    ) {
    // Try to obtain the named Lua function, registering it
    // if it was not already registered. If we could not
    // obtain it, report failure.
    LuaFunctionState lua_func_state;
    if (!register_continuable_lua_function(std::move(name), &lua_func_state)) {
        return false;
    }

    continuable_function
        = make_continuable_function<NewCallSignature, ContinuationCallSignature>(
            lua_func_state
        );

    return true;
}
