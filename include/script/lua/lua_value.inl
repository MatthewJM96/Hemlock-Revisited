template <typename Type>
constexpr Type hscript::lua::LuaValue<Type>::default_value() {
    return Type{};
}

template <typename Type>
constexpr ui32 hscript::lua::LuaValue<Type>::value_count() {
    if constexpr (std::is_scalar<Type>() || std::is_pointer<Type>()) {
        return 1;
    } else if constexpr (is_same_template<ui8v3, Type>::value || std::is_bounded_array<Type>()) {
        return sizeof(Type) / sizeof(decltype(Type{}[0]));
    }
}

template <typename Type>
ui32 hscript::lua::LuaValue<Type>::push(LuaHandle state, Type value) {
    if constexpr (is_same_template<ui8v3, Type>::value || std::is_bounded_array<Type>()) {
        // For each index in type, push that value separately,
        // returning how many have been pushed.
        for (ui32 idx = 0; idx < value_count(); ++idx) {
            LuaValue<decltype(Type{}[0])>::push(state, value[idx]);
        }
        return value_count();
    }

    if constexpr (std::is_same<Type, bool>()) {
        lua_pushboolean(state, value);
    } else if constexpr (std::is_same<Type, char>()) {
        char* tmp = new char[2];
        tmp[0] = value;
        tmp[1] = '\0';

        lua_pushstring(state, tmp);

        delete[] tmp;
    } else if constexpr (std::is_integral<Type>()) {
        lua_pushinteger(state, static_cast<lua_Integer>(value));
    } else if constexpr (std::is_floating_point<Type>()) {
        lua_pushnumber(state, static_cast<lua_Number>(value));
    } else if constexpr (std::is_same<Type, std::string>()) {
        lua_pushstring(state, value.c_str());
    } else if constexpr (std::is_same<typename std::remove_const<Type>::type, char*>()) {
        lua_pushstring(state, value);
    } else if constexpr (std::is_enum<Type>()) {
        using Underlying = typename std::underlying_type<Type>::type;

        LuaValue<Underlying>::push(state, static_cast<Underlying>(value));
    } else if constexpr (std::is_same<Type, void*>()) {
        lua_pushlightuserdata(state, value);
    } else if constexpr (std::is_pointer<Type>()) {
        LuaValue<void*>::push(
            state,
            reinterpret_cast<void*>(
                const_cast<std::remove_const<Type>::type>(value)
            )
        );
    }

    return 1;
}

template <typename Type>
Type hscript::lua::LuaValue<Type>::pop(LuaHandle state) {
    return retrieve(state, -1);
}

template <typename Type>
bool hscript::lua::LuaValue<Type>::try_pop(LuaHandle state, OUT Type& value) {
    return try_retrieve(state, -1, value);
}

template <typename Type>
Type hscript::lua::LuaValue<Type>::retrieve(LuaHandle state, i32 index) {
    if constexpr (is_same_template<ui8v3, Type>::value || std::is_bounded_array<Type>()) {
        Type tmp;
        // For each index in type, in reverse order, pop
        // that element and store in tmp for return.
        for (ui32 idx = value_count(); idx != 0; --idx) {
            tmp[idx - 1] = LuaValue<decltype(Type{}[0])>::retrieve(state, index + static_cast<i32>(idx) - value_count());
        }
        return tmp;
    }

    // Note that with lua_tostring, we are getting
    // a pointer to the string on the Lua stack, we
    // must therefore immediately make a copy to
    // avoid GC killing the value, hence the use of
    // std::string in all cases.

    Type value = default_value();
    if constexpr (std::is_same<Type, bool>()) {
        // Pops bool as integer and converts.
        value = lua_toboolean(state, index) != 0;
    } else if constexpr (std::is_same<Type, char>()) {
        // Pops char as string, return character if
        // it exists, else default value.
        std::string tmp = lua_tostring(state, index);

        if (tmp.length() > 0) value = tmp.c_str()[0];
    } else if constexpr (std::is_integral<Type>()) {
        value = static_cast<Type>(lua_tointeger(state, index));
    } else if constexpr (std::is_floating_point<Type>()) {
        value = static_cast<Type>(lua_tonumber(state, index));
    } else if constexpr (std::is_same<Type, std::string>()) {
        value = lua_tostring(state, index);
    } else if constexpr (std::is_same<typename std::remove_const<Type>::type, char*>()) {
        std::string tmp = lua_tostring(state, index);
        
        if (tmp.length() > 0) value = const_cast<Type>(tmp.c_str());
    } else if constexpr (std::is_enum<Type>()) {
        using Underlying = typename std::underlying_type<Type>::type;

        value = static_cast<Type>(LuaValue<Underlying>::pop(state));
    } else if constexpr (std::is_same<Type, void*>()) {
        value = lua_touserdata(state, index);
    } else if constexpr (std::is_pointer<Type>()) {
        value = reinterpret_cast<Type>(LuaValue<void*>::pop(state));
    }

    lua_remove(state, index);

    return value;
}

template <typename Type>
bool hscript::lua::LuaValue<Type>::try_retrieve(LuaHandle state, i32 index, OUT Type& value) {
    if constexpr (is_same_template<ui8v3, Type>::value || std::is_bounded_array<Type>()) {
        // For each index in type, test it has a
        // corresponding value on the Lua stack.
        for (ui32 idx = 0; idx < -value_count(); ++idx) {
            if (!test_index<decltype(Type{}[0])>(state, index - idx)) return false;
        }
        // We can pop the compound type!
        value = retrieve(state, index);
        return true;
    }

    // Test the top of the Lua stack, popping
    // only if the test succeeds.
    if (!test_index(state, index)) return false;

    value = retrieve(state, index);
    return true;
}

template <typename Type>
bool hscript::lua::LuaValue<Type>::test_index(LuaHandle state, i32 index) {
    if constexpr (std::is_same<Type, bool>()) {
        return lua_isboolean(state, index);
    } else if constexpr (std::is_same<Type, char>()
                        || std::is_same<Type, std::string>()
                        || std::is_same<typename std::remove_const<Type>::type, char*>()
    ) {
        return lua_isstring(state, index);
    } else if constexpr (std::is_integral<Type>()
                        || std::is_floating_point<Type>()) {
        return lua_isnumber(state, index);
    } else if constexpr (std::is_enum<Type>()) {
        using Underlying = typename std::underlying_type<Type>::type;

        return LuaValue<Underlying>::test_index(state, index);
    } else if constexpr (std::is_same<Type, void*>()) {
        return lua_islightuserdata(state, index);
    } else if constexpr (std::is_pointer<Type>()) {
        return LuaValue<void*>::test_index(state, index);
    }
}

template <typename ...Types>
constexpr ui32 hscript::lua::total_value_count() {
    ui32 total_count = 0;
    for (ui32 count : { 0, LuaValue<Types>::value_count()... }) {
        total_count += count;
    }
    return total_count;
}
