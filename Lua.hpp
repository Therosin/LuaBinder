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

// ─── LuaGet ────────────────────────────────────────────────────────────────────

// lua_get
// get values from the lua stack
template <typename T>
T lua_get(lua_State *L, int index);

// lua_get for int
template <>
int lua_get<int>(lua_State *L, int index)
{
    lua_Integer i = luaL_checkinteger(L, index + 1);
    return static_cast<int>(i);
}

// lua_get for double
template <>
double lua_get<double>(lua_State *L, int index)
{
    return luaL_checknumber(L, index + 1);
}

// lua_get for bool
template <>
bool lua_get<bool>(lua_State *L, int index)
{
    return lua_toboolean(L, index + 1);
}

// lua_get for std::string
template <>
std::string lua_get<std::string>(lua_State *L, int index)
{
    return std::string(luaL_checkstring(L, index + 1));
}

// lua_get for std::vector
template <typename T>
std::vector<T> lua_get(lua_State *L, int index)
{
    std::vector<T> ret;
    if (!lua_istable(L, index + 1))
    {
        std::cout << "Error: Not a table" << std::endl;
        return ret;
    }
    lua_pushnil(L);
    while (lua_next(L, index + 1))
    {
        ret.push_back(lua_get<T>(L, -1));
        lua_pop(L, 1);
    }
    return ret;
}

// lua_get for std::map
template <typename T, typename U>
std::map<T, U> lua_get(lua_State *L, int index)
{
    std::map<T, U> ret;
    if (!lua_istable(L, index + 1))
    {
        std::cout << "Error: Not a table" << std::endl;
        return ret;
    }
    lua_pushnil(L);
    while (lua_next(L, index + 1))
    {
        ret[lua_get<T>(L, -2)] = lua_get<U>(L, -1);
        lua_pop(L, 1);
    }
    return ret;
}

// ─── LuaPush ──────────────────────────────────────────────────────────────────

// lua_push
// push values to the lua stack
template <typename T>
void lua_push(lua_State *L, const T &value);

// lua_push for int
template <>
void lua_push<int>(lua_State *L, const int &value)
{
    lua_pushinteger(L, value);
}

// lua_push for double
template <>
void lua_push<double>(lua_State *L, const double &value)
{
    lua_pushnumber(L, value);
}

// lua_push for bool
template <>
void lua_push<bool>(lua_State *L, const bool &value)
{
    lua_pushboolean(L, value);
}

// lua_push for std::string
template <>
void lua_push<std::string>(lua_State *L, const std::string &value)
{
    lua_pushstring(L, value.c_str());
}

// lua_push for char&
template <>
void lua_push(lua_State *L, const char &value)
{
    char buffer[2];
    buffer[0] = value;
    buffer[1] = '\0';
    lua_pushstring(L, buffer);
}

// lua_push for std::vector
template <typename T>
void lua_push(lua_State *L, const std::vector<T> &value)
{
    lua_newtable(L);
    for (int i = 0; i < value.size(); i++)
    {
        lua_push<T>(L, value[i]);
        lua_rawseti(L, -2, i + 1);
    }
}

// lua_push for std::map
template <typename T, typename U>
void lua_push(lua_State *L, const std::map<T, U> &value)
{
    lua_newtable(L);
    for (auto &i : value)
    {
        lua_push<T>(L, i.first);
        lua_push<U>(L, i.second);
        lua_settable(L, -3);
    }
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

// ─── LuaGet ──────────────────────────────────────────────────────────────────
template <typename T>
T LuaGet(lua_State *L, int index);

template <>
int LuaGet<int>(lua_State *L, int index)
{
    lua_Integer i = luaL_checkinteger(L, index + 1);
    return static_cast<int>(i);
}

template <>
double LuaGet<double>(lua_State *L, int index)
{
    return luaL_checknumber(L, index + 1);
}

template <>
std::string LuaGet<std::string>(lua_State *L, int index)
{
    return std::string(luaL_checkstring(L, index + 1));
}

template <>
bool LuaGet<bool>(lua_State *L, int index)
{
    return lua_toboolean(L, index + 1) != 0;
}

template <>
char LuaGet<char>(lua_State *L, int index)
{
    const char *str = luaL_checkstring(L, index + 1);
    if (std::strlen(str) > 1)
        luaL_error(L, "Expected a single character but got a string");
    return str[0];
}

template <>
std::vector<int> LuaGet<std::vector<int>>(lua_State *L, int index)
{
    std::vector<int> vec;
    luaL_checktype(L, index + 1, LUA_TTABLE);
    lua_pushnil(L);
    while (lua_next(L, index + 1))
    {
        vec.push_back(LuaGet<int>(L, -1));
        lua_pop(L, 1);
    }
    return vec;
}

// ─── LuaSet ──────────────────────────────────────────────────────────────────
template <typename T>
bool LuaSet(lua_State *L, int index, T &&value);

template <>
bool LuaSet<int>(lua_State *L, int index, int &&value)
{
    lua_pushinteger(L, value);
    return true;
}

// LuaSet<int &>(struct lua_State *,int,int &)
template <>
bool LuaSet<int &>(lua_State *L, int index, int &value)
{
    lua_pushinteger(L, value);
    return true;
}

template <>
bool LuaSet<double>(lua_State *L, int index, double &&value)
{
    lua_pushnumber(L, value);
    return true;
}

template <>
bool LuaSet<std::string>(lua_State *L, int index, std::string &&value)
{
    lua_pushstring(L, value.c_str());
    return true;
}

template <>
bool LuaSet<bool>(lua_State *L, int index, bool &&value)
{
    lua_pushboolean(L, value);
    return true;
}

template <>
bool LuaSet<char>(lua_State *L, int index, char &&value)
{
    lua_pushstring(L, &value);
    return true;
}

template <>
bool LuaSet<std::vector<int>>(lua_State *L, int index, std::vector<int> &&vec)
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

// ─── LuaTuple ──────────────────────────────────────────────────────────────────

template <typename... Args>
std::tuple<Args...> LuaTuple(lua_State *L, int index)
{
    return std::make_tuple(LuaGet<Args>(L, index)...);
}

template <typename... Args>
bool LuaTuple(lua_State *L, int index, std::tuple<Args...> &&value)
{
    int i = index;
    (void)std::initializer_list<int>{(LuaSet(L, i++, std::get<Args>(value)), 0)...};
    return true;
}

// LuaPush
template <typename T>
void LuaPush(lua_State *L, T &&t)
{
    lua_pushlightuserdata(L, (void *)&&t);
}

template <typename T>
void LuaPush(lua_State *L, T &t)
{
    lua_pushlightuserdata(L, (void *)&t);
}

template <typename T>
void LuaPush(lua_State *L, T *t)
{
    lua_pushlightuserdata(L, (void *)t);
}

template <>
void LuaPush<int>(lua_State *L, int &&t)
{
    lua_pushinteger(L, t);
}

template <>

void LuaPush<double>(lua_State *L, double &&t)
{
    lua_pushnumber(L, t);
}

template <>
void LuaPush<std::string>(lua_State *L, std::string &&t)
{
    lua_pushstring(L, t.c_str());
}

template <>
void LuaPush<bool>(lua_State *L, bool &&t)
{
    lua_pushboolean(L, t);
}

template <>
void LuaPush<char>(lua_State *L, char &&t)
{
    lua_pushstring(L, &t);
}

template <>
void LuaPush<std::vector<int>>(lua_State *L, std::vector<int> &&t)
{
    lua_newtable(L);
    for (int i = 0; i < t.size(); i++)
    {
        lua_pushinteger(L, i + 1);
        lua_pushinteger(L, t[i]);
        lua_settable(L, -3);
    }
}
