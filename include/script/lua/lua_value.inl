template <typename Type>
constexpr Type hscript::lua::LuaValue<
    Type,
    typename std::enable_if_t<
        hscript::lua::is_single_lua_type<Type>::value
        || hscript::lua::is_multiple_lua_type<Type>::value>>::default_value() {
    return Type{};
}

template <typename Type>
constexpr ui32 hscript::lua::LuaValue<
    Type,
    typename std::enable_if_t<
        hscript::lua::is_single_lua_type<Type>::value
        || hscript::lua::is_multiple_lua_type<Type>::value>>::value_count() {
    if constexpr (is_multiple_lua_type<Type>()) {
        return sizeof(Type) / sizeof(decltype(Type{}[0]));
    } else if constexpr (std::is_same<Type, void>()) {
        return 0;
    }

    return 1;
}

template <typename Type>
ui32 hscript::lua::LuaValue<
    Type,
    typename std::enable_if_t<
        hscript::lua::is_single_lua_type<Type>::value
        || hscript::lua::is_multiple_lua_type<Type>::value>>::
    push(LuaHandle state, Type value) {
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
    } else if constexpr (std::is_same<typename std::remove_const<Type>::type, char*>())
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
            reinterpret_cast<void*>(
                const_cast<typename std::remove_const<Type>::type>(value)
            )
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
Type hscript::lua::LuaValue<
    Type,
    typename std::enable_if_t<
        hscript::lua::is_single_lua_type<Type>::value
        || hscript::lua::is_multiple_lua_type<Type>::value>>::pop(LuaHandle state) {
    return retrieve<true>(state, -1);
}

template <typename Type>
bool hscript::lua::LuaValue<
    Type,
    typename std::enable_if_t<
        hscript::lua::is_single_lua_type<Type>::value
        || hscript::lua::is_multiple_lua_type<Type>::value>>::
    try_pop(LuaHandle state, OUT Type& value) {
    return try_retrieve<true>(state, -1, value);
}

template <typename Type>
template <bool RemoveValue>
Type hscript::lua::LuaValue<
    Type,
    typename std::enable_if_t<
        hscript::lua::is_single_lua_type<Type>::value
        || hscript::lua::is_multiple_lua_type<Type>::value>>::
    retrieve(LuaHandle state, i32 index) {
    return __do_retrieve<RemoveValue>(state, index);
}

template <typename Type>
template <bool RemoveValue>
bool hscript::lua::LuaValue<
    Type,
    typename std::enable_if_t<
        hscript::lua::is_single_lua_type<Type>::value
        || hscript::lua::is_multiple_lua_type<Type>::value>>::
    try_retrieve(LuaHandle state, i32 index, OUT Type& value) {
    if constexpr (is_multiple_lua_type<Type>()) {
        // For each index in type, test it has a
        // corresponding value on the Lua stack.
        for (ui32 idx = 0; idx < -value_count(); ++idx) {
            if (!LuaValue<decltype(Type{}[0])>::test_index(state, index - idx))
                return false;
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
Type hscript::lua::LuaValue<
    Type,
    typename std::enable_if_t<
        hscript::lua::is_single_lua_type<Type>::value
        || hscript::lua::is_multiple_lua_type<Type>::value>>::
    retrieve_upvalue(LuaHandle state, i32 index) {
    return __do_retrieve<false>(state, lua_upvalueindex(index));
}

template <typename Type>
bool hscript::lua::LuaValue<
    Type,
    typename std::enable_if_t<
        hscript::lua::is_single_lua_type<Type>::value
        || hscript::lua::is_multiple_lua_type<Type>::value>>::
    test_index(LuaHandle state, i32 index) {
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

template <typename Type>
template <bool RemoveValue>
Type hscript::lua::LuaValue<
    Type,
    typename std::enable_if_t<
        hscript::lua::is_single_lua_type<Type>::value
        || hscript::lua::is_multiple_lua_type<Type>::value>>::
    __do_retrieve(LuaHandle state, i32 index) {
    if constexpr (is_multiple_lua_type<Type>()) {
        Type tmp;
        // For each index in type, in reverse order, pop
        // that element and store in tmp for return.
        for (ui32 idx = value_count(); idx != 0; --idx) {
            tmp[idx - 1]
                = LuaValue<decltype(Type{}[0])>::template __do_retrieve<
                    RemoveValue>(
                    state, index + static_cast<i32>(idx) - value_count()
                );
        }
        return tmp;
    }

    // Note that with lua_tostring, we are getting
    // a pointer to the string on the Lua stack, we
    // must therefore immediately make a copy to
    // avoid GC killing the value, hence the use of
    // std::string in all cases.

    Type value = default_value();

    /********\
     * Bool *
    \********/
    if constexpr (std::is_same<Type, bool>()) {
        // Pops bool as integer and converts.
        value = lua_toboolean(state, index) != 0;

        /********\
         * Char *
        \********/
    } else if constexpr (std::is_same<Type, char>()) {
        value = lua_tostring(state, index)[0];

        /***********\
         * Integer *
        \***********/
    } else if constexpr (std::is_integral<Type>()) {
        value = static_cast<Type>(lua_tointeger(state, index));

        /*********\
         * Float *
        \*********/
    } else if constexpr (std::is_floating_point<Type>()) {
        value = static_cast<Type>(lua_tonumber(state, index));

        /**********\
         * String *
        \**********/
    } else if constexpr (std::is_same<Type, std::string>()) {
        value = std::string(
            lua_tostring(state, index), lua_strlen(state, index)
        );

        /************\
         * C-String *
        \************/
    } else if constexpr (std::is_same<
                                typename std::remove_const<Type>::type,
                                char*>())
    {
        auto len = lua_strlen(state, index);
        if (len != 0) {
            value = new char[len];
            std::memcpy(lua_tostring(state, index), value, len);
        }

        /********\
         * Enum *
        \********/
    } else if constexpr (std::is_enum<Type>()) {
        using Underlying = typename std::underlying_type<Type>::type;

        value = static_cast<Type>(
            LuaValue<Underlying>::template __do_retrieve<false>(
                state, index
            )
        );

        /*********\
         * void* *
        \*********/
    } else if constexpr (std::is_same<Type, void*>()) {
        value = lua_touserdata(state, index);

        /******\
         * T* *
        \******/
    } else if constexpr (std::is_pointer<Type>()) {
        value = reinterpret_cast<Type>(
            LuaValue<void*>::template __do_retrieve<false>(state, index)
        );

        /****************\
         * U(V::*)(...) *
        \****************/
    } else if constexpr (std::is_member_function_pointer<Type>()) {
        value = *reinterpret_cast<Type*>(lua_touserdata(state, index));
    } else {
        debug_printf("Trying to retrieve with an unsupported type.");
    }

    if constexpr (RemoveValue && !is_multiple_lua_type<Type>())
        lua_remove(state, index);

    return value;
}

template <typename... Types>
constexpr ui32 hscript::lua::total_value_count() {
    return (0 + ... + LuaValue<Types>::value_count());
}
