// Copyright (C) 2023 Theros < MisModding | SvalTek >
//
// This file is part of EzConsole.
//
// EzConsole is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// EzConsole is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with EzConsole.  If not, see <http://www.gnu.org/licenses/>.
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
        end = top;
    }
    std::cout << "Lua Stack (" << -top << ") Entries" << std::endl;
    std::cout << "Lua Stack Start: " << start << " End: " << end << std::endl;
    // we are counting -1, -2, -3, -4, -5, ...
    for (level = start; level >= end; level--)
    {
        int t = lua_type(L, level);
        std::string lua_type_name;
        std::string lua_value;
        // if t within 0-9
        if (t >= 0 && t <= 9)
        {
            lua_type_name = lua_typename(L, t);
            lua_value = lua_tostring(L, level);
        }
        else
        {
            lua_type_name = "unknown";
            lua_value = "unknown";
        }
        std::cout << "  [" << level << "]"
                  << " type: " << lua_type_name << " value: " << lua_value << std::endl;
    }
}

// ─── LuaGet ────────────────────────────────────────────────────────────────────

// lua_get
// get values from the lua stack
template <typename T>
T lua_get(lua_State *L, int level = -1);

// lua_get for std::string
template <>
std::string lua_get<std::string>(lua_State *L, int level)
{
    std::string s = "null";
    if (!lua_isstring(L, level))
    {
        std::cout << "lua_get<std::string>[" << level << "]: not a string" << std::endl;
    }
    return std::string(lua_tostring(L, level));
}

// lua_get for int
template <>
int lua_get<int>(lua_State *L, int level)
{
    if (!lua_isnumber(L, level))
    {
        std::cout << "lua_get<int>[" << level << "]: not a number" << std::endl;
        return 0;
    }
    return static_cast<int>(lua_tonumber(L, level));
}

// lua_get for double
template <>
double lua_get<double>(lua_State *L, int level)
{
    if (!lua_isnumber(L, level))
    {
        std::cout << "lua_get<double>[" << level << "]: not a number" << std::endl;
        return 0.0;
    }
    return static_cast<double>(lua_tonumber(L, level));
}

// lua_get for float
template <>
float lua_get<float>(lua_State *L, int level)
{
    if (!lua_isnumber(L, level))
    {
        std::cout << "lua_get<float>[" << level << "]: not a number" << std::endl;
        return 0.0f;
    }
    return static_cast<float>(lua_tonumber(L, level));
}

// lua_get for bool
template <>
bool lua_get<bool>(lua_State *L, int level)
{
    if (!lua_isboolean(L, level))
    {
        std::cout << "lua_get<bool>[" << level << "]: not a bool" << std::endl;
        return false;
    }
    return lua_toboolean(L, level);
}

// lua_get for char
template <>
char lua_get<char>(lua_State *L, int level)
{
    char c = 0;
    if (lua_isstring(L, level))
    {
        std::string s = std::string(lua_tostring(L, level));
        c = s[0];
    }
    else
    {
        std::cout << "lua_get<char>[" << level << "]: not a string" << std::endl;
    }
    return c;
}

// ─── lua_set ──────────────────────────────────────────────────────────────────

template <typename T>
bool lua_set(lua_State *L, int level, T &&value);

template <typename T>
bool lua_set(lua_State *L, int level, T &&value)
{
    std::cout << "lua_set: not implemented for type: " << typeid(T).name() << std::endl;
    return false;
}

template <>
bool lua_set<int>(lua_State *L, int level, int &&value)
{
    lua_pushinteger(L, value);
    return true;
}

// lua_set<int &>(struct lua_State *,int,int &)
template <>
bool lua_set<int &>(lua_State *L, int level, int &value)
{
    lua_pushinteger(L, value);
    return true;
}

template <>
bool lua_set<double>(lua_State *L, int level, double &&value)
{
    lua_pushnumber(L, value);
    return true;
}

template <>
bool lua_set<std::string>(lua_State *L, int level, std::string &&value)
{
    lua_pushstring(L, value.c_str());
    return true;
}

template <>
bool lua_set<bool>(lua_State *L, int level, bool &&value)
{
    lua_pushboolean(L, value);
    return true;
}

template <>
bool lua_set<char>(lua_State *L, int level, char &&value)
{
    lua_pushstring(L, &value);
    return true;
}

template <>
bool lua_set<std::vector<int>>(lua_State *L, int level, std::vector<int> &&vec)
{
    lua_newtable(L);
    for (int i = 0; i < vec.size(); i++)
    {
        lua_pushinteger(L, i + 1);
        lua_pushinteger(L, vec[i]);
        lua_settable(L, -3);
    }
    return true;
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

// lua_push for int
template <>
bool lua_push<int>(lua_State *L, int &value)
{
    lua_pushinteger(L, value);
    return true;
}

// lua_push for double
template <>
bool lua_push<double>(lua_State *L, double &value)
{
    lua_pushnumber(L, value);
    return true;
}

// lua_push for bool
template <>
bool lua_push<bool>(lua_State *L, bool &value)
{
    lua_pushboolean(L, value);
    return true;
}

// lua_push for std::string
template <>
bool lua_push<std::string>(lua_State *L, std::string &value)
{
    lua_pushstring(L, value.c_str());

    return true;
}

// lua_push for char&
template <>
bool lua_push<char>(lua_State *L, char &value)
{
    char buffer[2];
    buffer[0] = value;
    buffer[1] = '\0';
    lua_pushstring(L, buffer);

    return true;
}

// lua_push for std::vector
template <typename T>
bool lua_push(lua_State *L, std::vector<T> &value)
{
    lua_newtable(L);
    for (int i = 0; i < value.size(); i++)
    {
        lua_push<T>(L, value[i]);
        lua_rawseti(L, -2, i + 1);
    }

    return true;
}

// lua_push for std::map
template <typename T, typename U>
bool lua_push(lua_State *L, std::map<T, U> &value)
{
    lua_newtable(L);
    for (auto &i : value)
    {
        lua_push<T>(L, i.first);
        lua_push<U>(L, i.second);
        lua_settable(L, -3);
    }

    return true;
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

// ─── LuaTuple ──────────────────────────────────────────────────────────────────

template <typename... Args>
std::tuple<Args...> LuaTuple(lua_State *L, int level)
{
    return std::make_tuple(LuaGet<Args>(L, level)...);
}

template <typename... Args>
bool LuaTuple(lua_State *L, int level, std::tuple<Args...> &&value)
{
    int i = level;
    (void)std::initializer_list<int>{(lua_set(L, i++, std::get<Args>(value)), 0)...};
    return true;
}
