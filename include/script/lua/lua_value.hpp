#ifndef __hemlock_script_lua_lua_value_hpp
#define __hemlock_script_lua_lua_value_hpp

#include "script/lua/state.hpp"

namespace hemlock {
    namespace script {
        namespace lua {
            template <typename Type>
            struct is_single_lua_type : public std::false_type { };
            template <typename Type>
                requires (
                    is_bounded_array<Type>::value
                    || is_same_template<ui8v3, Type>::value
                )
            struct is_single_lua_type : public std::true_type { };

            template <typename Type>
            struct is_multiple_lua_value : public std::false_type { };
            template <typename Type>
                requires (
                    std::is_arithmetic<Type>::value
                    || std::is_pointer<Type>::value
                    || is_same_template<std::string, Type>::value
                )
            struct is_multiple_lua_value : public std::true_type { };

            /**
             * @brief Provides an API for moving data between Lua stack and C++ side.
             * It is implemented for scalar types, pointer types, bounded array types
             * and glm vectors.
             *
             * @tparam Type The type of the data to be moved.
             */
            template <typename Type>
                requires (
                    is_single_lua_type<Type>::type
                        || is_multiple_lua_type<Type>::value
                )
            struct LuaValue {
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
                 * specified. The value is removed from
                 * the Lua stack.
                 *
                 * @param state The Lua state.
                 * @param index The index into the Lua
                 * stack to retrieve from.
                 * @return Type The value retrieved.
                 */
                static Type retrieve(LuaHandle state, i32 index);
                /**
                 * @brief Try to retrieve a value of type
                 * Type from the Lua stack at the index
                 * specified. The value is removed from
                 * the Lua stack.
                 *
                 * @param state The Lua state.
                 * @param index The index into the Lua
                 * stack to retrieve from.
                 * @param value The value to try to
                 * retrieve.
                 * @return True if the value was
                 * retrieved, false otherwise.
                 */
                static bool try_retrieve(LuaHandle state, i32 index, OUT Type& value);
            protected:
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
            template <typename ...Types>
            constexpr ui32 total_value_count();
        }
    }
}
namespace hscript = hemlock::script;

#include "lua_value.inl"

#endif // __hemlock_script_lua_lua_value_hpp
