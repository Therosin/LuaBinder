#include "Global.h"

bool enable_debug = false;

lua_State *L = luaL_newstate();
LuaScript *Script;

int add(int a, int b)
{
    return a + b;
};

void log(std::string message)
{
    std::cout << message << std::endl;
}

int main(int argc, char *argv[])
{
    system("cls");

    Script = new LuaScript(L);

    // register Lua functions
    LuaFunction<int(int, int)> luaAdd(add);
    luaAdd.Register(Script->State(), "add");

    LuaFunction<void(std::string)> luaLog(log);
    luaLog.Register(Script->State(), "Log");

    if (!Script->runString("testList = { 'Item1', 'Item2', 'Item3' }; testMap = { key1 = 'Value1', key2 = 'Value2', key3 = 'Value3' }; Log('hello world Result: ' .. add(10, 20))"))
    {
        std::cout << "failed to load string" << std::endl;
        std::string error = Script->GetError();
        if (!error.empty())
        {
            std::cout << error << std::endl;
        }
        system("pause");
    }

    std::vector<std::string> testItems = Script->GetList<std::string>("testList");
    std::cout << "Test List: " << std::endl;
    for (auto &item : testItems)
    {
        std::cout << "  " << item << std::endl;
    }

    testItems[2] = "Item3-2";

    Script->SetList<std::string>("testList", testItems);

    std::cout << "Test List: " << std::endl;
    std::vector<std::string> testItems2 = Script->GetList<std::string>("testList");
    for (auto &item : testItems2)
    {
        std::cout << "  " << item << std::endl;
    }

    std::map<std::string, std::string> testMap = Script->GetMap<std::string, std::string>("testMap");
    std::cout << "Test Map: " << std::endl;
    for (auto &item : testMap)
    {
        std::cout << "  " << item.first << " = " << item.second << std::endl;
    }

    testMap["key3"] = "Value3-2";
    Script->SetMap<std::string, std::string>("testMap", testMap);

    std::cout << "Test Map: " << std::endl;
    std::map<std::string, std::string> testMap2 = Script->GetMap<std::string, std::string>("testMap");
    for (auto &item : testMap2)
    {
        std::cout << "  " << item.first << " = " << item.second << std::endl;
    }

    system("pause");
    return 0;
}
