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
        LuaValue<Type>::push(m_state, val);
        lua_setfield(m_state, -1, name.c_str());
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
    }
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
    // TODO(Matthew): We can't do this... it will break as sizeof(func) here is not == sizeof(void*).
    //                  Can use lua_newuserdata to achieve what we want, but need some Typeless trick.
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
