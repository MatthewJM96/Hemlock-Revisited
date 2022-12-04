template <typename Type>
    requires (
        hscript::lua::is_single_lua_type<Type>::value
        || hscript::lua::is_multiple_lua_type<Type>::value
    )
constexpr Type hscript::lua::LuaValue<Type>::default_value() {
    return Type{};
}

template <typename Type>
    requires (
        hscript::lua::is_single_lua_type<Type>::value
        || hscript::lua::is_multiple_lua_type<Type>::value
    )
constexpr ui32 hscript::lua::LuaValue<Type>::value_count() {
    if constexpr (is_multiple_lua_type<Type>()) {
        return sizeof(Type) / sizeof(decltype(Type{}[0]));
    } else if constexpr (std::is_same<Type, void>()) {
        return 0;
    }

    return 1;
}

template <typename Type>
    requires (
        hscript::lua::is_single_lua_type<Type>::value
        || hscript::lua::is_multiple_lua_type<Type>::value
    )
ui32 hscript::lua::LuaValue<Type>::push(LuaHandle state, Type value) {
    if constexpr (is_multiple_lua_type<Type>()) {
        // For each index in type, push that value separately,
        // returning how many have been pushed.
        for (ui32 idx = 0; idx < value_count(); ++idx) {
            LuaValue<decltype(Type{}[0])>::push(state, value[idx]);
        }
        return value_count();
    }

    /********\
     * Bool *
    \********/
    if constexpr (std::is_same<Type, bool>()) {
        lua_pushboolean(state, value);

        /********\
         * Char *
        \********/
    } else if constexpr (std::is_same<Type, char>()) {
        char* tmp = new char[2];
        tmp[0]    = value;
        tmp[1]    = '\0';

        lua_pushlstring(state, tmp, 1);

        delete[] tmp;

        /***********\
         * Integer *
        \***********/
    } else if constexpr (std::is_integral<Type>()) {
        lua_pushinteger(state, static_cast<lua_Integer>(value));

        /*********\
         * Float *
        \*********/
    } else if constexpr (std::is_floating_point<Type>()) {
        lua_pushnumber(state, static_cast<lua_Number>(value));

        /**********\
         * String *
        \**********/
    } else if constexpr (std::is_same<Type, std::string>()) {
        lua_pushlstring(state, value.c_str(), value.size());

        /************\
         * C-String *
        \************/
    } else if constexpr (std::is_same<typename std::remove_const<Type>::type, char*>(
                         ))
    {
        lua_pushlstring(state, value, strlen(value));

        /********\
         * Enum *
        \********/
    } else if constexpr (std::is_enum<Type>()) {
        using Underlying = typename std::underlying_type<Type>::type;

        LuaValue<Underlying>::push(state, static_cast<Underlying>(value));

        /*********\
         * void* *
        \*********/
    } else if constexpr (std::is_same<Type, void*>()) {
        lua_pushlightuserdata(state, value);

        /******\
         * T* *
        \******/
    } else if constexpr (std::is_pointer<Type>()) {
        LuaValue<void*>::push(
            state,
            reinterpret_cast<void*>(const_cast<std::remove_const<Type>::type>(value))
        );

        /****************\
         * U(V::*)(...) *
        \****************/
    } else if constexpr (std::is_member_function_pointer<Type>()) {
        void* block = lua_newuserdata(state, sizeof(Type));

        std::memcpy(block, reinterpret_cast<void*>(&value), sizeof(Type));
    } else {
        debug_printf("Trying to push with an unsupported type.");
    }

    return 1;
}

template <typename Type>
    requires (
        hscript::lua::is_single_lua_type<Type>::value
        || hscript::lua::is_multiple_lua_type<Type>::value
    )
Type hscript::lua::LuaValue<Type>::pop(LuaHandle state) {
    return retrieve<true>(state, -1);
}

template <typename Type>
    requires (
        hscript::lua::is_single_lua_type<Type>::value
        || hscript::lua::is_multiple_lua_type<Type>::value
    ) bool
hscript::lua::LuaValue<Type>::try_pop(LuaHandle state, OUT Type& value) {
    return try_retrieve<true>(state, -1, value);
}

template <typename Type>
    requires (
        hscript::lua::is_single_lua_type<Type>::value
        || hscript::lua::is_multiple_lua_type<Type>::value
    )
template <bool RemoveValue>
Type hscript::lua::LuaValue<Type>::retrieve(LuaHandle state, i32 index) {
    return __do_retrieve<RemoveValue>(state, index);
}

template <typename Type>
    requires (
        hscript::lua::is_single_lua_type<Type>::value
        || hscript::lua::is_multiple_lua_type<Type>::value
    )
template <bool RemoveValue>
bool hscript::lua::LuaValue<Type>::try_retrieve(
    LuaHandle state, i32 index, OUT Type& value
) {
    if constexpr (is_multiple_lua_type<Type>()) {
        // For each index in type, test it has a
        // corresponding value on the Lua stack.
        for (ui32 idx = 0; idx < -value_count(); ++idx) {
            if (!test_index<decltype(Type{}[0])>(state, index - idx)) return false;
        }
        // We can pop the compound type!
        value = retrieve<RemoveValue>(state, index);
        return true;
    }

    // Test the top of the Lua stack, popping
    // only if the test succeeds.
    if (!test_index(state, index)) return false;

    value = retrieve<RemoveValue>(state, index);
    return true;
}

template <typename Type>
    requires (
        hscript::lua::is_single_lua_type<Type>::value
        || hscript::lua::is_multiple_lua_type<Type>::value
    )
Type hscript::lua::LuaValue<Type>::retrieve_upvalue(LuaHandle state, i32 index) {
    return __do_retrieve<false>(state, lua_upvalueindex(index));
}

template <typename Type>
    requires (
        hscript::lua::is_single_lua_type<Type>::value
        || hscript::lua::is_multiple_lua_type<Type>::value
    ) bool
hscript::lua::LuaValue<Type>::test_index(LuaHandle state, i32 index) {
    /********\
     * Bool *
    \********/
    if constexpr (std::is_same<Type, bool>()) {
        return lua_isboolean(state, index);

        /************\
         *     Char *
         *   String *
         * C-String *
        \************/
    } else if constexpr (std::is_same<Type, char>() || std::is_same<Type, std::string>() || std::is_same<typename std::remove_const<Type>::type, char*>())
    {
        return lua_isstring(state, index);

        /***********\
         * Integer *
         *   Float *
        \***********/
    } else if constexpr (std::is_integral<Type>() || std::is_floating_point<Type>()) {
        return lua_isnumber(state, index);

        /********\
         * Enum *
        \********/
    } else if constexpr (std::is_enum<Type>()) {
        using Underlying = typename std::underlying_type<Type>::type;

        return LuaValue<Underlying>::test_index(state, index);

        /*********\
         * void* *
        \*********/
    } else if constexpr (std::is_same<Type, void*>()) {
        return lua_islightuserdata(state, index);

        /******\
         * T* *
        \******/
    } else if constexpr (std::is_pointer<Type>()) {
        return LuaValue<void*>::test_index(state, index);

        /****************\
         * U(V::*)(...) *
        \****************/
    } else if constexpr (std::is_member_function_pointer<Type>()) {
        return lua_isuserdata(state, index);
    } else {
        debug_printf("Trying to test index with an unsupported type.");
    }
}

template <typename... Types>
constexpr ui32 hscript::lua::total_value_count() {
    return (0 + ... + LuaValue<Types>::value_count());
}
