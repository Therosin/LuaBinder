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
