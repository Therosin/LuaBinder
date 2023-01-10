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
#include "Lua.hpp"

// ─── LuaFunction ─────────────────────────────────────────────────────────────
template <typename Sig>
struct LuaFunction;

template <typename Ret, typename... Args>
struct LuaFunction<Ret(Args...)>
{
  std::function<Ret(Args...)> func;

  template <typename F>
  LuaFunction(F &&f) : func(std::forward<F>(f)) {}
  LuaFunction(Ret (*f)(Args...)) : func(f) {}

  Ret operator()(Args... args)
  {
    return func(args...);
  }

  void Register(lua_State *L, const char *name)
  {
    // check if lua_State pointer is valid
    if (!L)
    {
      std::cerr << "Error: Invalid lua_State pointer" << std::endl;
      return;
    }
    // check if name is valid
    if (!name || name[0] == '\0')
    {
      std::cerr << "Error: Invalid function name" << std::endl;
      return;
    }
    // check if the function object is valid
    if (!func)
    {
      std::cerr << "Error: Invalid function object" << std::endl;
      return;
    }
    // push a new userdata to the stack
    LuaFunction<Ret(Args...)> *f = static_cast<LuaFunction<Ret(Args...)> *>(lua_newuserdata(L, sizeof(LuaFunction<Ret(Args...)>)));
    // initialize the userdata
    new (f) LuaFunction<Ret(Args...)>(func);
    // push the function to the stack
    lua_pushcclosure(
        L, [](lua_State *L) -> int
        {
            // get the function object from upvalue
            LuaFunction<Ret(Args...)> &f = *static_cast<LuaFunction<Ret(Args...)> *>(lua_touserdata(L, lua_upvalueindex(1)));
            // check number of arguments
            if (lua_gettop(L) != sizeof...(Args))
            {
                std::cerr << "Error: Invalid number of arguments expected " << sizeof...(Args) << ", got " << lua_gettop(L) << std::endl;
                return 0;
            }
            // get the function name
            std::string name = lua_tostring(L, lua_upvalueindex(2));
            // check if the function name is valid
            if (name.empty())
            {
                std::cerr << "Error: Invalid/Missing function name" << std::endl;
                return 0;
            }
            // get the arguments from the stack
            std::tuple<Args...> args;
            int i = 1;
            std::apply([&](auto &... args) { ((args = LuaGet<Args>(L, -++i)), ...); }, args);
            if constexpr (!std::is_void<Ret>::value)
            {
              // call the function
              Ret ret = std::apply([&](auto &&... args){
                  return f.func(args...);
              }, args);
              // push the return value to the stack
              LuaSet(L, -1, ret);
              // return the number of return values
              return 1;
            } else {
              // call the function
              std::apply([&](auto &&... args){
                  return f.func(args...);
              }, args);
              // push nil to the stack
              lua_pushnil(L);
              // return the number of return values
              return 0;
            }},
        2);
    // set the function name as the key
    lua_setglobal(L, name);
    // pop the userdata from the stack
    lua_pop(L, 1);
    // push the function to the stack
    lua_getglobal(L, name);
    // push the function object to the stack
    LuaPush(L, this);
    // set the upvalue
    lua_setupvalue(L, -2, 1);
    // push the function name to the stack
    lua_pushstring(L, name);
    // set the upvalue
    lua_setupvalue(L, -2, 2);
    // pop the function from the stack
    lua_pop(L, 1);
  }
};

// Construct a LuaFunction from any callable
template <typename F>
LuaFunction(F &&f) -> LuaFunction<decltype(std::forward<F>(f))>;

// Construct a LuaFunction from a raw function pointer
template <typename Ret, typename... Args>
LuaFunction(Ret (*)(Args...)) -> LuaFunction<Ret(Args...)>;

// ─── LuaScript ───────────────────────────────────────────────────────────────
class LuaScript
{
public:
  LuaScript(lua_State *L);
  LuaScript(const std::string &filename);
  ~LuaScript();
  void printError(const std::string &variableName, const std::string &reason);

  /** Get the current lua_State
   * @return The current lua_State
   */
  lua_State *State() { return this->L; }

  // runString
  // Runs the lua code stored in the string
  bool runString(const std::string &str)
  {
    // Attempt to execute the string as Lua code
    if (luaL_loadstring(this->L, str.c_str()) || lua_pcall(this->L, 0, 0, 0))
    {
      // Print an error message if the string could not be executed
      std::cout << "Error: failed to load string ::\r\n " << str << std::endl;
      return false;
    }
    return true;
  }

  // runFile
  // Runs the lua code stored in the file
  bool runFile(const std::string &filename)
  {
    if (luaL_loadfile(this->L, filename.c_str()) || lua_pcall(this->L, 0, 0, 0))
    {
      std::cout << "Error: failed to load file :: '" << filename << "'" << std::endl;
      return false;
    }
    return true;
  }

  inline void clean()
  {
    int n = lua_gettop(L);
    lua_pop(L, n);
  }

  bool lua_gettostack(const std::string &variableName)
  {
    level = 0;
    std::string var = "";
    for (unsigned int i = 0; i < variableName.size(); i++)
    {
      if (variableName.at(i) == '.')
      {
        if (level == 0)
        {
          lua_getglobal(L, var.c_str());
        }
        else
        {
          lua_getfield(L, -1, var.c_str());
        }

        if (lua_isnil(L, -1))
        {
          printError(variableName, var + " is not defined");
          return false;
        }
        else
        {
          var = "";
          level++;
        }
      }
      else
      {
        var += variableName.at(i);
      }
    }
    if (level == 0)
    {
      lua_getglobal(L, var.c_str());
    }
    else
    {
      lua_getfield(L, -1, var.c_str());
    }
    if (lua_isnil(L, -1))
    {
      printError(variableName, var + " is not defined");
      return false;
    }

    return true;
  }

  template <typename T>
  bool lua_is(lua_State *L, int index)
  {
    switch (lua_type(L, index))
    {
    case LUA_TSTRING:
      return std::is_same<T, std::string>::value;
    case LUA_TBOOLEAN:
      return std::is_same<T, bool>::value;
    case LUA_TNUMBER:
      return std::is_same<T, int>::value || std::is_same<T, double>::value;
    case LUA_TTABLE:
      return std::is_same<T, std::vector<std::string>>::value || std::is_same<T, std::vector<int>>::value || std::is_same<T, std::vector<double>>::value || std::is_same<T, std::vector<bool>>::value;
    case LUA_TFUNCTION:
      return std::is_same<T, lua_CFunction>::value;
    case LUA_TUSERDATA:
      return std::is_same<T, void *>::value;
    default:
      return false;
    }
  }

  // Generic get
  template <typename T>
  T lua_get(const std::string &variableName)
  {
    return 0;
  }

  template <typename T>
  T lua_getdefault()
  {
    return 0;
  }

  template <typename T>
  T get(const std::string &variableName)
  {
    if (!L)
    {
      printError(variableName, "Script is not loaded");
      return lua_getdefault<T>();
    }

    T result;
    if (lua_gettostack(variableName))
    { // variable succesfully on top of stack
      result = lua_get<T>(variableName);
    }
    else
    {
      result = lua_getdefault<T>();
    }

    clean();
    return result;
  }

  std::vector<int> getIntVector(const std::string &name);
  std::vector<std::string> getTableKeys(const std::string &name);

  template <typename T>
  std::vector<T> GetList(const std::string &name);

  template <typename T>
  void SetList(const std::string &name, const std::vector<T> &list);

  template <typename T, typename U>
  std::map<T, U> GetMap(const std::string &name);

  template <typename T, typename U>
  void SetMap(const std::string &name, const std::map<T, U> &map);

  // Get the last error from lua
  std::string GetError()
  {
    return lua_tostring(L, -1);
  }

private:
  lua_State *L;
  std::string filename;
  int level;
};

LuaScript::LuaScript(const std::string &filename)
{
  L = luaL_newstate();
  if (luaL_loadfile(L, filename.c_str()) || lua_pcall(L, 0, 0, 0))
  {
    std::cout << "Error: failed to load (" << filename << ")" << std::endl;
    L = 0;
    return;
  }

  if (L)
    luaL_openlibs(L);
}

LuaScript::LuaScript(lua_State *L)
{
  this->L = L;
  luaL_openlibs(L);
}

LuaScript::~LuaScript()
{
  if (L)
    lua_close(L);
}

void LuaScript::printError(const std::string &variableName, const std::string &reason)
{
  std::cout << "Error: can't get [" << variableName << "]. " << reason << std::endl;
}

std::vector<int> LuaScript::getIntVector(const std::string &name)
{
  std::vector<int> v;
  lua_gettostack(name.c_str());
  if (lua_isnil(L, -1))
  { // array is not found
    return std::vector<int>();
  }
  lua_pushnil(L);
  while (lua_next(L, -2))
  {
    v.push_back((int)lua_tonumber(L, -1));
    lua_pop(L, 1);
  }
  clean();
  return v;
}

std::vector<std::string> LuaScript::getTableKeys(const std::string &name)
{
  std::string code =
      "function getKeys(name) "
      "s = \"\""
      "for k, v in pairs(_G[name]) do "
      "    s = s..k..\",\" "
      "    end "
      "return s "
      "end"; // function for getting table keys
  luaL_loadstring(L,
                  code.c_str()); // execute code
  lua_pcall(L, 0, 0, 0);
  lua_getglobal(L, "getKeys"); // get function
  lua_pushstring(L, name.c_str());
  lua_pcall(L, 1, 1, 0); // execute function
  std::string test = lua_tostring(L, -1);
  std::vector<std::string> strings;
  std::string temp = "";
  for (unsigned int i = 0; i < test.size(); i++)
  {
    if (test.at(i) != ',')
    {
      temp += test.at(i);
    }
    else
    {
      strings.push_back(temp);
      temp = "";
    }
  }
  clean();
  return strings;
}

template <>
inline std::string LuaScript::lua_getdefault<std::string>()
{
  return "null";
}

// lua_get for string
template <>
inline std::string LuaScript::lua_get<std::string>(const std::string &variableName)
{
  std::string s = "null";
  if (lua_isstring(L, -1))
  {
    s = std::string(lua_tostring(L, -1));
  }
  else
  {
    printError(variableName, "Not a string");
  }
  return s;
}

// lua_get for int
template <>
inline int LuaScript::lua_get<int>(const std::string &variableName)
{
  if (!lua_isnumber(L, -1))
  {
    printError(variableName, "Not a number");
  }
  return (int)lua_tonumber(L, -1);
}

// lua_get for float
template <>
inline float LuaScript::lua_get<float>(const std::string &variableName)
{
  if (!lua_isnumber(L, -1))
  {
    printError(variableName, "Not a number");
  }
  return (float)lua_tonumber(L, -1);
}

// lua_get for bool
template <>
inline bool LuaScript::lua_get<bool>(const std::string &variableName)
{
  return (bool)lua_toboolean(L, -1);
}

// template GetList
// Gets a List of values of type T in the lua state
template <typename T>
std::vector<T> LuaScript::GetList(const std::string &name)
{
  std::vector<T> result;
  if (!L)
  {
    printError(name, "No State");
    return result;
  }

  if (lua_gettostack(name))
  { // variable succesfully on top of stack
    if (lua_istable(L, -1))
    {                 // table on top of stack
      lua_pushnil(L); // nil key on top of stack
      while (lua_next(L, -2) != 0)
      { // key and value on top of stack
        if (lua_is<T>(L, -1))
        { // value is of type T
          result.push_back(lua_get<T>(name));
        }
        else
        {
          // skip invalid value
          break;
        }
        lua_pop(L, 1); // remove value, keep key for next iteration
      }
    }
    else
    {
      printError(name, "is not a table");
    }
  }
  clean();
  return result;
}

// template SetList
// Sets a List of values of type T in the lua state
template <typename T>
void LuaScript::SetList(const std::string &name, const std::vector<T> &list)
{
  if (!L)
  {
    printError(name, "No State");
    return;
  }

  lua_newtable(L);
  for (size_t i = 0; i < list.size(); i++)
  {
    lua_pushnumber(L, (int)i + 1);
    lua_push(L,list[i]);
    lua_settable(L, -3);
  }
  lua_setglobal(L, name.c_str());
}

// template GetMap
// Gets a Map of values of type T in the lua state
template <typename T, typename U>
std::map<T, U> LuaScript::GetMap(const std::string &name)
{
  std::map<T, U> result;
  if (!L)
  {
    printError(name, "No State");
    return result;
  }

  if (lua_gettostack(name))
  { // variable succesfully on top of stack
    if (lua_istable(L, -1))
    {                 // table on top of stack
      lua_pushnil(L); // nil key on top of stack
      while (lua_next(L, -2) != 0)
      { // key and value on top of stack
        if (lua_is<T>(L, -2) && lua_is<U>(L, -1))
        { // value is of type T
          result[LuaGet<T>(L, -3)] = LuaGet<U>(L, -2);
        }
        else
        {
          // skip invalid value
          break;
        }
        lua_pop(L, 1); // remove value, keep key for next iteration
      }
    }
    else
    {
      printError(name, "is not a table");
    }
  }
  clean();
  return result;
}

// template SetMap
// Sets a Map of values of type T in the lua state
template <typename T, typename U>
void LuaScript::SetMap(const std::string &name, const std::map<T, U> &map)
{
  if (!L)
  {
    printError(name, "No State");
    return;
  }

  lua_newtable(L);
  for (auto it = map.begin(); it != map.end(); it++)
  {
    lua_push(L, it->first);
    lua_push(L, it->second);
    lua_settable(L, -3);
  }
  lua_setglobal(L, name.c_str());
}

// ─── LuaTable ────────────────────────────────────────────────────────────────

// class LuaTable
// {
//   LuaTable(LuaScript *luaScript, const std::string &name);

//   bool isValid() const;

//   std::vector<std::string> getKeys() const;

//   template <typename T>
//   T get(const std::string &key) const;
// };

// LuaTable::LuaTable(LuaScript *luaScript, const std::string &tableName)
// {
//   this->luaScript = luaScript;
//   this->name = tableName;
// }
