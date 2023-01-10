#include "Global.h"

bool enable_debug = false;

lua_State *L = luaL_newstate();
LuaScript *Script;

int add(int a, int b)
{
    return a + b;
};

int main(int argc, char *argv[])
{
    system("cls");

    Script = new LuaScript(L);

    // register Lua function
    LuaFunction<int(int, int)> luaAdd(add);
    luaAdd.Register(Script->State(), "add");
    if (!Script->runString("print(add(1,2))"))
    {
        std::cout << "failed to load string" << std::endl;
        std::string error = Script->GetError();
        if (!error.empty())
        {
            std::cout << error << std::endl;
        }
        system("pause");
    }

    system("pause");
    return 0;
}
