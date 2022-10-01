#ifndef __hemlock_script_lua_lua_value_hpp
#define __hemlock_script_lua_lua_value_hpp

#include "script/lua/state.hpp"

namespace hemlock {
    namespace script {
        namespace lua {
            /**
             * @brief Provides an API for moving data between Lua stack and C++ side.
             * It is implemented for scalar types, pointer types, bounded array types
             * and glm vectors.
             *
             * @tparam Type The type of the data to be moved.
             */
            template <typename Type>
                requires (
                    !std::is_compound<Type>::value
                        || std::is_pointer<Type>::value
                        || std::is_bounded_array<Type>::value
                        || is_same_template<ui8v3, Type>::value
                )
            struct LuaValue {
                static constexpr Type default_value();
                static constexpr ui32 value_count();
                static ui32 push(LuaHandle state, Type value);
                static Type pop(LuaHandle state);
                static bool try_pop(LuaHandle state, OUT Type& value);
                static Type retrieve(LuaHandle state, i32 index);
                static bool try_retrieve(LuaHandle state, i32 index, OUT Type& value);
            protected:
                static bool test_index(LuaHandle state, i32 index);
            };

            template <typename ...Types>
            constexpr ui32 total_value_count();
        }
    }
}
namespace hscript = hemlock::script;

#include "lua_value.inl"

#endif // __hemlock_script_lua_lua_value_hpp
