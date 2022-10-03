#include "c_function.hpp"
#include "lua_value.hpp"

template <typename ...Strings>
void hscript::lua::Environment::set_namespaces(Strings... namespaces) {
    set_global_namespace();

    for (const auto& _namespace : {namespaces...}) {
        push_namespace(_namespace);
    }
}

template <typename ...Strings>
void hscript::lua::Environment::enter_namespaces(Strings... namespaces) {
    for (const auto& _namespace : {namespaces...}) {
        push_namespace(_namespace);
    }
}

template <typename Type>
void hscript::lua::Environment::add_value(const std::string& name, Type val) {

}

template <typename ReturnType, typename ...Parameters>
void hscript::lua::Environment::add_c_delegate(const std::string& name, Delegate<ReturnType, Parameters...>* delegate) {
    if (m_parent) return m_parent->add_c_delegate(name, delegate);

    LuaValue<void*>::push(m_state, reinterpret_cast<void*>(delegate));

    lua_pushcclosure(m_state, invoke_delegate<ReturnType, Parameters...>, 1);
    lua_setfield(m_state, -2, name.c_str());
}

template <typename ReturnType, typename ...Parameters>
void hscript::lua::Environment::add_c_function(const std::string& name, ReturnType(*func)(Parameters...)) {
    if (m_parent) return m_parent->add_c_function(name, func);

    LuaValue<void*>::push(m_state, reinterpret_cast<void*>(func));

    lua_pushcclosure(m_state, invoke_function<ReturnType, Parameters...>, 1);
    lua_setfield(m_state, -2, name.c_str());
}

template <typename ...Upvalues>
void hscript::lua::Environment::add_c_function(const std::string& name, i32(*func)(LuaHandle), Upvalues... upvalues) {
    if (m_parent) return m_parent->add_c_function(name, func, std::forward<Upvalues>(upvalues)...);

    (LuaValue<Upvalues>::push(m_state, upvalues), ...);
    LuaValue<void*>::push(m_state, reinterpret_cast<void*>(func));

    i32 value_count = 1 + total_value_count<Upvalues...>();

    lua_pushcclosure(m_state, func, value_count);
    lua_setfield(m_state, -2, name.c_str());
}

template <std::invocable Closure, typename ReturnType, typename ...Parameters>
void hscript::lua::Environment::add_c_closure(const std::string& name, Closure* closure, ReturnType(Closure::*func)(Parameters...)) {
    if (m_parent) return m_parent->add_c_closure(name, closure, func);

    LuaValue<void*>::push(m_state, reinterpret_cast<void*>(closure));
    LuaValue<void*>::push(m_state, reinterpret_cast<void*>(func));

    lua_pushcclosure(m_state, invoke_closure<Closure, ReturnType, Parameters...>, 2);
    lua_setfield(m_state, -2, name.c_str());
}

template <typename ReturnType, typename ...Parameters>
bool hscript::lua::Environment::get_script_function(const std::string& name, OUT ScriptDelegate<ReturnType, Parameters...>& delegate) {
    // Try to obtain the named Lua function, registering it
    // if it was not already registered. If we could not
    // obtain it, report failure.
    LuaFunctionState lua_func_state;
    if (!register_lua_function(name, &lua_func_state)) {
        return false;
    }

    delegate = make_lua_delegate<ReturnType, Parameters...>(lua_func_state);

    return true;
}
