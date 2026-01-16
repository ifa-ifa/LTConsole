#include <Windows.h>
#include <LunarTear.h>
#include <string>
#include <vector>
#include <stdexcept>
#include <filesystem> 

typedef void (*LunarTearPluginInitFunc)(const LunarTearAPI* api, LT_PluginHandle handle);

extern "C" __declspec(dllexport)
void LunarTearPluginInit(const LunarTearAPI* api, LT_PluginHandle handle)
{
    api->Log(handle, LT_LOG_VERBOSE, "Stub start");

    char buffer[1024];
    int outsize = api->GetModDirectory(handle, "LTCon", buffer, sizeof(buffer));

    if (outsize <= 0) {
        api->Log(handle, LT_LOG_ERROR, "Stub failed: Could not get mod directory.");
        return;
    }

    std::filesystem::path modDir(buffer);
    std::filesystem::path dllPath = modDir / "bin" / "LTConsole.dll";


    HMODULE mainlib = LoadLibraryExW(dllPath.c_str(), NULL, LOAD_WITH_ALTERED_SEARCH_PATH);

    if (mainlib == NULL) {
        DWORD err = GetLastError();
        std::string errStr = "Stub could not load main library. Error: " + std::to_string(err);
        api->Log(handle, LT_LOG_ERROR, errStr.c_str());
        return;
    }

    LunarTearPluginInitFunc func = (LunarTearPluginInitFunc)GetProcAddress(mainlib, "LunarTearPluginInit");

    if (func) {
        func(api, handle);
    }
    else {
        api->Log(handle, LT_LOG_ERROR, "Stub failed: Could not find 'LunarTearPluginInit' in target DLL.");
    }

    api->Log(handle, LT_LOG_VERBOSE, "Stub end");
}