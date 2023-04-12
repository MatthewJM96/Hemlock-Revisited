#ifndef __hemlock_script_lua_lua_value_hpp
#define __hemlock_script_lua_lua_value_hpp

#include "script/lua/state.hpp"

namespace hemlock {
    namespace script {
        namespace lua {
            template <typename, typename = void>
            struct is_single_lua_type : public std::false_type { };

            template <typename Type>
            struct is_single_lua_type<
                Type,
                typename std::enable_if_t<
                    std::is_arithmetic<Type>::value || std::is_enum<Type>::value
                    || std::is_pointer<Type>::value
                    || std::is_member_function_pointer<Type>::value
                    || is_same_template<std::string, Type>::value>> :
                public std::true_type { };

            template <typename, typename = void>
            struct is_multiple_lua_type : public std::false_type { };

            template <typename Type>
            struct is_multiple_lua_type<
                Type,
                typename std::enable_if_t<
                    std::is_bounded_array<Type>::value
                    || is_same_template<ui8v3, Type>::value>> :
                public std::true_type { };

            template <typename Type>
            struct LuaValue<
                Type,
                typename std::enable_if_t<std::is_same<Type, void>::value>> {
                static constexpr ui32 value_count() { return 0; }
            };

            template <typename Type>
            struct LuaValue<
                Type,
                typename std::enable_if_t<
                    is_single_lua_type<Type>::value
                    || is_multiple_lua_type<Type>::value>> {
                /**
                 * @brief Provides the default value for
                 * type Type.
                 *
                 * @return constexpr Type The default value.
                 */
                static constexpr Type default_value();
                /**
                 * @brief Provides the number of values
                 * actually pushed or popped for type
                 * Type.
                 *
                 * @return constexpr ui32 The number of
                 * values.
                 */
                static constexpr ui32 value_count();
                /**
                 * @brief Push a value of type Type onto
                 * the Lua stack.
                 *
                 * @param state The Lua state.
                 * @param value The value to push.
                 * @return ui32 The number of items pushed
                 * onto the Lua stack.
                 */
                static ui32 push(LuaHandle state, Type value);
                /**
                 * @brief Pop a value of type Type from
                 * the Lua stack.
                 *
                 * @param state The Lua state.
                 * @return Type The value popped from
                 * the Lua stack.
                 */
                static Type pop(LuaHandle state);
                /**
                 * @brief Try to pop a value of type Type
                 * from the Lua stack.
                 *
                 * @param state The Lua state.
                 * @param value The value to try to pop.
                 * @return True if the value was popped,
                 * false otherwise.
                 */
                static bool try_pop(LuaHandle state, OUT Type& value);
                /**
                 * @brief Retrieve a value of type Type
                 * from the Lua stack at the index
                 * specified.
                 *
                 * @tparam bool Whether to remove the retrieved
                 * value from the stack or not.
                 * @param state The Lua state.
                 * @param index The index into the Lua
                 * stack to retrieve from.
                 * @return Type The value retrieved.
                 */
                template <bool RemoveValue>
                static Type retrieve(LuaHandle state, i32 index);
                /**
                 * @brief Try to retrieve a value of type
                 * Type from the Lua stack at the index
                 * specified. The value is removed from
                 * the Lua stack.
                 *
                 * @tparam bool Whether to remove the retrieved
                 * value from the stack or not.
                 * @param state The Lua state.
                 * @param index The index into the Lua
                 * stack to retrieve from.
                 * @param value The value to try to
                 * retrieve.
                 * @return True if the value was
                 * retrieved, false otherwise.
                 */
                template <bool RemoveValue>
                static bool try_retrieve(LuaHandle state, i32 index, OUT Type& value);
                /**
                 * @brief Retrieve a value of type Type
                 * from the Lua stack at the upvalue
                 * index specified. Upvalues are handled
                 * by Lua and are not really on the
                 * stack.
                 *
                 * @param state The Lua state.
                 * @param index The upvalue index into
                 * the Lua stack to retrieve from.
                 * @return Type The value retrieved.
                 */
                static Type retrieve_upvalue(LuaHandle state, i32 index);
                /**
                 * @brief Test the given index in the
                 * Lua stack for being of type Type.
                 *
                 * @param state The Lua state.
                 * @param index The index to test.
                 * @return True if the value at
                 * the given index is of type
                 * Type, false otherwise.
                 */
                static bool test_index(LuaHandle state, i32 index);

                /**
                 * @brief Retrieve a value of type Type
                 * from the Lua stack at the index
                 * specified. The value is removed from
                 * the Lua stack.
                 *
                 * @tparam RemoveValue Whether to remove
                 * the value retrieved.
                 * @param state The Lua state.
                 * @param index The index into the Lua
                 * stack to retrieve from.
                 * @return Type The value retrieved.
                 */
                template <bool RemoveValue>
                static Type __do_retrieve(LuaHandle state, i32 index) {
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
            };

            /**
             * @brief Calculates the total number
             * of values across the set of types.
             *
             * @tparam Types The types to get the
             * value count for.
             * @return constexpr ui32 The total
             * count of values that would
             * represent these types on the Lua
             * stack.
             */
            template <typename... Types>
            constexpr ui32 total_value_count();
        }  // namespace lua
    }      // namespace script
}  // namespace hemlock
namespace hscript = hemlock::script;

#include "lua_value.inl"

#endif  // __hemlock_script_lua_lua_value_hpp
