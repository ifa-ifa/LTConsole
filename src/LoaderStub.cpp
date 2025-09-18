#include <Windows.h>
#include <LunarTear.h>
#include <string>
#include <stdexcept>

typedef void (*LunarTearPluginInitFunc)(const LunarTearAPI* api, LT_PluginHandle handle);

inline std::wstring to_wstring(const std::string& s) {
	if (s.empty()) return {};
	int size = MultiByteToWideChar(CP_UTF8, 0, s.c_str(), -1, nullptr, 0);
	if (size == 0) throw std::runtime_error("MultiByteToWideChar failed");
	std::wstring ws(size - 1, L'\0'); 
	MultiByteToWideChar(CP_UTF8, 0, s.c_str(), -1, ws.data(), size);
	return ws;
}


extern "C" __declspec(dllexport)
void LunarTearPluginInit(const LunarTearAPI* api, LT_PluginHandle handle)
{

	api->Log(handle, LT_LOG_VERBOSE, "Stub start");

	std::string buffer(1024, '\0');
	int outsize = api->GetModDirectory(handle, "LTCon", buffer.data(), buffer.size());
	buffer.resize(outsize);

	buffer += "/bin/";

	SetDefaultDllDirectories(LOAD_LIBRARY_SEARCH_DEFAULT_DIRS | LOAD_LIBRARY_SEARCH_USER_DIRS);
	DLL_DIRECTORY_COOKIE cookie = AddDllDirectory(to_wstring(buffer).c_str());



	buffer += "LTConsole.dll";
	HMODULE mainlib = LoadLibrary(buffer.c_str());

	RemoveDllDirectory(cookie);

	if (mainlib == 0) {
		api->Log(handle, LT_LOG_INFO, "Stub could not load main library, LoadLibrary returned Null");
		return;
	}

	LunarTearPluginInitFunc func = (LunarTearPluginInitFunc)GetProcAddress(mainlib, "LunarTearPluginInit");

	func(api, handle);

	api->Log(handle, LT_LOG_VERBOSE, "Stub end");


}
