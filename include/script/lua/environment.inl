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
    LuaValue<void*>::push(m_state, reinterpret_cast<void*>(delegate));

    lua_pushcclosure(m_state, invoke_delegate<ReturnType, Parameters...>, 1);
    lua_setfield(m_state, -2, name.c_str());
}

template <typename ReturnType, typename ...Parameters>
void hscript::lua::Environment::add_c_function(const std::string& name, ReturnType(*func)(Parameters...)) {
    LuaValue<void*>::push(m_state, reinterpret_cast<void*>(func));

    lua_pushcclosure(m_state, invoke_function<ReturnType, Parameters...>, 1);
    lua_setfield(m_state, -2, name.c_str());
}

template <std::invocable Closure, typename ReturnType, typename ...Parameters>
void hscript::lua::Environment::add_c_closure(const std::string& name, Closure* closure, ReturnType(Closure::*func)(Parameters...)) {
    LuaValue<void*>::push(m_state, reinterpret_cast<void*>(closure));
    LuaValue<void*>::push(m_state, reinterpret_cast<void*>(func));

    lua_pushcclosure(m_state, invoke_closure<Closure, ReturnType, Parameters...>, 2);
    lua_setfield(m_state, -2, name.c_str());
}

template <typename ReturnType, typename ...Parameters>
hemlock::Delegate<ReturnType, Parameters...> hscript::lua::Environment::get_script_function(const std::string& name, bool do_register/* = true*/) {

}
