#include <Windows.h>
#include <LunarTear++.h>
#include <QApplication>
#include <thread>
#include "Bindings.h"
#include "ui/MainWindow.h"

const LunarTearAPI* g_api;
LT_PluginHandle g_handle;
QString g_modBasePath;

std::thread g_uiThread;
std::unique_ptr<QApplication> g_app;

std::atomic<bool> g_running = false;

void StartUI(bool runInThread)
{
    auto guiFunc = [] {
        g_running = true;
        int argc = 0;
        g_app = std::make_unique<QApplication>(argc, nullptr);

        MainWindow window;
        //window.show();

        g_app->exec();      
        g_app.reset();
        g_running = false;
        };

    if (runInThread) {
        g_uiThread = std::thread(guiFunc);
    }
    else {
        guiFunc();
    }
}

extern "C" __declspec(dllexport)
void LunarTearPluginInit(const LunarTearAPI* api, LT_PluginHandle handle)
{
    g_api = api;
    g_handle = handle;

    LunarTear::Init(api, handle);


    LT_LuaCFunc fLoadString = (LT_LuaCFunc)(LunarTear::Get().Game().GetProcessBaseAddress() + 0x3d92b0); // luaB_loadstring
    LunarTear::Get().RegisterLuaCFunc("_ifaifa_LTCon_loadstring", fLoadString);
    LT_LuaCFunc fPcall = (LT_LuaCFunc)(LunarTear::Get().Game().GetProcessBaseAddress() + 0x3d94f0); // luaB_pcall
    LunarTear::Get().RegisterLuaCFunc("_ifaifa_LTCon_pcall", fPcall);
    LT_LuaCFunc fToString = (LT_LuaCFunc)(LunarTear::Get().Game().GetProcessBaseAddress() + 0x3d95c0); // luaB_tostring
    LunarTear::Get().RegisterLuaCFunc("_ifaifa_LTCon_tostring", fToString);
    LT_LuaCFunc fXpcall = (LT_LuaCFunc)(LunarTear::Get().Game().GetProcessBaseAddress() + 0x3d9550); // luaB_xpcall
    LunarTear::Get().RegisterLuaCFunc("_ifaifa_LTCon_xpcall", fXpcall);
    LT_LuaCFunc fType = (LT_LuaCFunc)(LunarTear::Get().Game().GetProcessBaseAddress() + 0x3d90c0); // luaB_type
    LunarTear::Get().RegisterLuaCFunc("_ifaifa_LTCon_type", fType);
    LT_LuaCFunc fTable_getn = (LT_LuaCFunc)(LunarTear::Get().Game().GetProcessBaseAddress() + 0x3eb5d0); // table.getn
    LunarTear::Get().RegisterLuaCFunc("_ifaifa_LTCon_table_getn", fTable_getn);


    LunarTear::Get().RegisterLuaCFunc("_ifaifa_LTCon_GetCommand", Binding_GetCommand);
    LunarTear::Get().RegisterLuaCFunc("_ifaifa_LTCon_SetCommandResult", Binding_SetCommandResult);
    LunarTear::Get().RegisterLuaCFunc("_ifaifa_LTCon_PostStartMessage", Binding_PostStartMessage);
    LunarTear::Get().RegisterLuaCFunc("_ifaifa_LTCon_PostLoadMessage", Binding_PostLoadMessage);



    StartUI(true);
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
    if (ul_reason_for_call == DLL_PROCESS_ATTACH) {
        DisableThreadLibraryCalls(hModule);
    }

    else if (ul_reason_for_call == DLL_PROCESS_DETACH) {
        if (g_running) { 
            // thread-safe way to ask the QApplication to quit
            QMetaObject::invokeMethod(g_app.get(), "quit", Qt::QueuedConnection);
        }

        if (g_uiThread.joinable()) {
            // We cannot join the thread here so detatch it
            g_uiThread.detach();
        }
    }

    return TRUE;
}

int main(int argc, char** argv) {
    StartUI(false);

    return 0;
}