#pragma once
#include <functional>
#include <string>
#include <iostream>
#include <tuple>
#include <map>
#include <cstring>

extern "C"
{
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
}

// print the global table
void LuaPrintGlobals(lua_State *L)
{
    std::cout << "Globals: " << std::endl;
    lua_getglobal(L, "_G");
    lua_pushnil(L);
    while (lua_next(L, -2))
    {
        std::cout << "  " << lua_tostring(L, -2) << " : " << lua_typename(L, lua_type(L, -1)) << std::endl;
        lua_pop(L, 1);
    }
}

void LuaPrintStack(lua_State *L, int start = 0, int end = 0)
{
    int level;
    int top = lua_gettop(L);
    if (end == 0)
    {
        end = -top;
    }
    std::cout << "Lua Stack (" << top << ") Entries" << std::endl;
    std::cout << "Lua Stack Start: " << start << " End: " << end << std::endl;
    // we are counting -1, -2, -3, -4, -5, ...
    for (level = start; level >= end; level--)
    {
        std::cout << "  [" << level << "]";
        int t = lua_type(L, level);
        switch(t)
         {
            case LUA_TSTRING:
                std::cout << " type: string     value: " << lua_tostring(L, level) << std::endl;
                break;
            case LUA_TBOOLEAN:
                std::cout << " type: boolean    value: " << lua_tostring(L, level) << std::endl;
                break;
            case LUA_TNUMBER:
                std::cout << " type: number     value: " << lua_tostring(L, level) << std::endl;
                break;
            case LUA_TTABLE:
                std::cout << " type: table" << std::endl;
                break;
            case LUA_TFUNCTION:
                std::cout << " type: function" << std::endl;
                break;
            case LUA_TUSERDATA:
                std::cout << " type: userdata" << std::endl;
                break;
            default:
                std::cout << " type: unknown" << std::endl;
        }
    }
}

// ─── LuaPush ──────────────────────────────────────────────────────────────────

// lua_push
// push values to the lua stack

template <typename T>
bool lua_pushdefault(lua_State *L, T &value)
{
    std::cout << "lua_push: not implemented for type: " << typeid(T).name() << std::endl;
    return false;
}

template <typename T>
bool lua_push(lua_State *L, T &&value)
{
    // if value is a pointer, push it as light userdata
    if (std::is_pointer<T &&>::value)
    {
        std::cout << "lua_push: got type: " << typeid(T).name() << ", pushing function as userdata" << std::endl;
        lua_pushlightuserdata(L, (void *)&&value);
        return true;
    }
    return lua_pushdefault(L, value);
}

template <typename T>
bool lua_push(lua_State *L, T &value)
{
    if (std::is_pointer<T &>::value)
    {
        std::cout << "lua_push: got type: " << typeid(T).name() << ", pushing function as userdata" << std::endl;
        lua_pushlightuserdata(L, (void *)&value);
        return true;
    }
    return lua_pushdefault(L, value);
}

template <typename T>
bool lua_push(lua_State *L, T *value)
{
    if (std::is_pointer<T *>::value)
    {
        std::cout << "lua_push: got type: " << typeid(T).name() << ", pushing function as userdata" << std::endl;
        lua_pushlightuserdata(L, (void *)value);
        return true;
    }
    return lua_pushdefault(L, value);
}

// ─── LuaCall ──────────────────────────────────────────────────────────────────
// Used to call a function in a lua script

template <typename T>
T LuaCall(lua_State *L, const std::string &name, const std::vector<T> &args)
{
    lua_getglobal(L, name.c_str());
    for (const auto &arg : args)
        lua_push<T>(L, arg);
    int e = lua_pcall(L, args.size(), 1, 0);
    if (e)
    {
        std::cout << "Error: " << lua_tostring(L, -1) << std::endl;
        lua_pop(L, 1);
        return T();
    }
    T ret;
    lua_pop(L, ret);
    return ret;
}

// ─── LuaStack ────────────────────────────────────────────────────────────────

template<typename T>
struct LuaStack;

//------------------------------------------------------------------------------
/**
  Receive the lua_State* as an argument.
  */
template <>
struct LuaStack <lua_State*>
{
    static lua_State* get (lua_State* L, int)
    {
        return L;
    }
};

//------------------------------------------------------------------------------
/**
  Push a lua_CFunction.
  */
template <>
struct LuaStack <lua_CFunction>
{
    static void push (lua_State* L, lua_CFunction f)
    {
        lua_pushcfunction (L, f);
    }

    static lua_CFunction get (lua_State* L, int index)
    {
        return lua_tocfunction (L, index);
    }
};

//------------------------------------------------------------------------------
/**
  LuaStack specialization for `int`.
  */
template <>
struct LuaStack <int>
{
    static inline void push (lua_State* L, int value)
    {
        lua_pushinteger (L, static_cast <lua_Integer> (value));
    }

    static inline int get (lua_State* L, int index)
    {
        return static_cast <int> (luaL_checkinteger (L, index));
    }
};

template <>
struct LuaStack <int const&>
{
    static inline void push (lua_State* L, int value)
    {
        lua_pushnumber (L, static_cast <lua_Number> (value));
    }

    static inline int get (lua_State* L, int index)
    {
        return static_cast <int > (luaL_checknumber (L, index));
    }
};
//------------------------------------------------------------------------------
/**
  LuaStack specialization for `unsigned int`.
  */
template <>
struct LuaStack <unsigned int>
{
    static inline void push (lua_State* L, unsigned int value)
    {
        lua_pushinteger (L, static_cast <lua_Integer> (value));
    }

    static inline unsigned int get (lua_State* L, int index)
    {
        return static_cast <unsigned int> (luaL_checkinteger (L, index));
    }
};

template <>
struct LuaStack <unsigned int const&>
{
    static inline void push (lua_State* L, unsigned int value)
    {
        lua_pushnumber (L, static_cast <lua_Number> (value));
    }

    static inline unsigned int get (lua_State* L, int index)
    {
        return static_cast <unsigned int > (luaL_checknumber (L, index));
    }
};

//------------------------------------------------------------------------------
/**
  LuaStack specialization for `unsigned char`.
  */
template <>
struct LuaStack <unsigned char>
{
    static inline void push (lua_State* L, unsigned char value)
    {
        lua_pushinteger (L, static_cast <lua_Integer> (value));
    }

    static inline unsigned char get (lua_State* L, int index)
    {
        return static_cast <unsigned char> (luaL_checkinteger (L, index));
    }
};

template <>
struct LuaStack <unsigned char const&>
{
    static inline void push (lua_State* L, unsigned char value)
    {
        lua_pushnumber (L, static_cast <lua_Number> (value));
    }

    static inline unsigned char get (lua_State* L, int index)
    {
        return static_cast <unsigned char> (luaL_checknumber (L, index));
    }
};

//------------------------------------------------------------------------------
/**
  LuaStack specialization for `short`.
  */
template <>
struct LuaStack <short>
{
    static inline void push (lua_State* L, short value)
    {
        lua_pushinteger (L, static_cast <lua_Integer> (value));
    }

    static inline short get (lua_State* L, int index)
    {
        return static_cast <short> (luaL_checkinteger (L, index));
    }
};

template <>
struct LuaStack <short const&>
{
    static inline void push (lua_State* L, short value)
    {
        lua_pushnumber (L, static_cast <lua_Number> (value));
    }

    static inline short get (lua_State* L, int index)
    {
        return static_cast <short> (luaL_checknumber (L, index));
    }
};

//------------------------------------------------------------------------------
/**
  LuaStack specialization for `unsigned short`.
  */
template <>
struct LuaStack <unsigned short>
{
    static inline void push (lua_State* L, unsigned short value)
    {
        lua_pushinteger (L, static_cast <lua_Integer> (value));
    }

    static inline unsigned short get (lua_State* L, int index)
    {
        return static_cast <unsigned short> (luaL_checkinteger (L, index));
    }
};

template <>
struct LuaStack <unsigned short const&>
{
    static inline void push (lua_State* L, unsigned short value)
    {
        lua_pushnumber (L, static_cast <lua_Number> (value));
    }

    static inline unsigned short get (lua_State* L, int index)
    {
        return static_cast <unsigned short> (luaL_checknumber (L, index));
    }
};

//------------------------------------------------------------------------------
/**
  LuaStack specialization for `long`.
  */
template <>
struct LuaStack <long>
{
    static inline void push (lua_State* L, long value)
    {
        lua_pushinteger (L, static_cast <lua_Integer> (value));
    }

    static inline long get (lua_State* L, int index)
    {
        return static_cast <long> (luaL_checkinteger (L, index));
    }
};

template <>
struct LuaStack <long const&>
{
    static inline void push (lua_State* L, long value)
    {
        lua_pushnumber (L, static_cast <lua_Number> (value));
    }

    static inline long get (lua_State* L, int index)
    {
        return static_cast <long> (luaL_checknumber (L, index));
    }
};

//------------------------------------------------------------------------------
/**
  LuaStack specialization for `unsigned long`.
  */
template <>
struct LuaStack <unsigned long>
{
    static inline void push (lua_State* L, unsigned long value)
    {
        lua_pushinteger (L, static_cast <lua_Integer> (value));
    }

    static inline unsigned long get (lua_State* L, int index)
    {
        return static_cast <unsigned long> (luaL_checkinteger (L, index));
    }
};

template <>
struct LuaStack <unsigned long const&>
{
    static inline void push (lua_State* L, unsigned long value)
    {
        lua_pushnumber (L, static_cast <lua_Number> (value));
    }

    static inline unsigned long get (lua_State* L, int index)
    {
        return static_cast <unsigned long> (luaL_checknumber (L, index));
    }
};

//------------------------------------------------------------------------------
/**
  LuaStack specialization for `float`.
  */
template <>
struct LuaStack <float>
{
    static inline void push (lua_State* L, float value)
    {
        lua_pushnumber (L, static_cast <lua_Number> (value));
    }

    static inline float get (lua_State* L, int index)
    {
        return static_cast <float> (luaL_checknumber (L, index));
    }
};

template <>
struct LuaStack <float const&>
{
    static inline void push (lua_State* L, float value)
    {
        lua_pushnumber (L, static_cast <lua_Number> (value));
    }

    static inline float get (lua_State* L, int index)
    {
        return static_cast <float> (luaL_checknumber (L, index));
    }
};

//------------------------------------------------------------------------------
/**
  LuaStack specialization for `double`.
  */
template <> struct LuaStack <double>
{
    static inline void push (lua_State* L, double value)
    {
        lua_pushnumber (L, static_cast <lua_Number> (value));
    }

    static inline double get (lua_State* L, int index)
    {
        return static_cast <double> (luaL_checknumber (L, index));
    }
};

template <> struct LuaStack <double const&>
{
    static inline void push (lua_State* L, double value)
    {
        lua_pushnumber (L, static_cast <lua_Number> (value));
    }

    static inline double get (lua_State* L, int index)
    {
        return static_cast <double> (luaL_checknumber (L, index));
    }
};

//------------------------------------------------------------------------------
/**
  LuaStack specialization for `bool`.
  */
template <>
struct LuaStack <bool> {
    static inline void push (lua_State* L, bool value)
    {
        lua_pushboolean (L, value ? 1 : 0);
    }

    static inline bool get (lua_State* L, int index)
    {
        return lua_toboolean (L, index) ? true : false;
    }
};

template <>
struct LuaStack <bool const&> {
    static inline void push (lua_State* L, bool value)
    {
        lua_pushboolean (L, value ? 1 : 0);
    }

    static inline bool get (lua_State* L, int index)
    {
        return lua_toboolean (L, index) ? true : false;
    }
};

//------------------------------------------------------------------------------
/**
  LuaStack specialization for `char`.
  */
template <>
struct LuaStack <char>
{
    static inline void push (lua_State* L, char value)
    {
        char str [2] = { value, 0 };
        lua_pushstring (L, str);
    }

    static inline char get (lua_State* L, int index)
    {
        return luaL_checkstring (L, index) [0];
    }
};

template <>
struct LuaStack <char const&>
{
    static inline void push (lua_State* L, char value)
    {
        char str [2] = { value, 0 };
        lua_pushstring (L, str);
    }

    static inline char get (lua_State* L, int index)
    {
        return luaL_checkstring (L, index) [0];
    }
};

//------------------------------------------------------------------------------
/**
  LuaStack specialization for `float`.
  */
template <>
struct LuaStack <char const*>
{
    static inline void push (lua_State* L, char const* str)
    {
        if (str != 0)
            lua_pushstring (L, str);
        else
            lua_pushnil (L);
    }

    static inline char const* get (lua_State* L, int index)
    {
        return lua_isnil (L, index) ? 0 : luaL_checkstring (L, index);
    }
};

//------------------------------------------------------------------------------
/**
  LuaStack specialization for `std::string`.
  */
template <>
struct LuaStack <std::string>
{
    static inline void push (lua_State* L, std::string const& str)
    {
        lua_pushlstring (L, str.c_str (), str.size());
    }

    static inline std::string get (lua_State* L, int index)
    {
        size_t len;
        const char *str = luaL_checklstring(L, index, &len);
        return std::string (str, len);
    }
};

//------------------------------------------------------------------------------
/**
  LuaStack specialization for `std::string const&`.
  */
template <>
struct LuaStack <std::string const&>
{
    static inline void push (lua_State* L, std::string const& str)
    {
        lua_pushlstring (L, str.c_str(), str.size());
    }

    static inline std::string get (lua_State* L, int index)
    {
        size_t len;
        const char *str = luaL_checklstring(L, index, &len);
        return std::string (str, len);
    }
};


// ─── LuaGet ────────────────────────────────────────────────────────────────────

// lua_get
// get values from the lua stack
template <typename T>
T lua_get(lua_State* L, int level = 0)
{
    return LuaStack<T>::get(L, level);
}

// lua_set
// set a value on the lua stack
template <typename T>
bool lua_set(lua_State* L, T value, int level = 0)
{
    LuaStack<T>::push(L, value);
    if (level != 0)
        lua_replace(L, level);
    return true;
}

// ─── LuaTuple ──────────────────────────────────────────────────────────────────

template <typename... Args>
std::tuple<Args...> LuaTuple(lua_State *L, int level)
{
    return std::make_tuple(lua_get<Args>(L, level)...);
}

template <typename... Args>
bool LuaTuple(lua_State *L, int level, std::tuple<Args...> &&value)
{
    int i = level;
    (void)std::initializer_list<int>{(lua_set(L, i++, std::get<Args>(value)), 0)...};
    return true;
}
