#include "Bindings.h"
#include <LunarTear++.h>
#include <LuaConsoleManager.h>
#include <GameData.h>
#include <mutex>
#include "Callbacks.h"

static std::string g_currentCommand;
static std::string g_currentResult;
static std::mutex g_commandMutex;


void _GetCommand(ScriptState* state) {

	std::string cmd;
	uint64_t terminalId = 0;
	bool had = LuaConsoleManager::instance().popNextCommand(cmd, terminalId);

	if (!had) {
		LunarTear::Get().Game().SetArgumentString(state->returnBuffer, "");
		state->returnArgCount = 1;
		return;
	}

	LuaConsoleManager::instance().setCurrentExecutingId(terminalId);

	LunarTear::Get().Game().SetArgumentString(state->returnBuffer, cmd.c_str());
	state->returnArgCount = 1;

}

void _SetCommandResult(ScriptState* state) {

	void* pArg = LunarTear::Get().Game().GetArgumentPointer(state->argBuffer, 0);
	const char* arg = LunarTear::Get().Game().GetArgumentString(pArg);

	QString result = arg ? QString::fromUtf8(arg) : QString();

	LuaConsoleManager::instance().postResultForCurrentExecuting(result);

}



void _PostStartMessage(ScriptState* state) {
	while (true) {
		std::function<void()> task;
		{
			std::lock_guard<std::mutex> lock(postStartQueueMutex);
			if (postStartTaskQueue.empty()) break;
			task = std::move(postStartTaskQueue.front());
			postStartTaskQueue.pop();
		}
		task();
	}

	std::vector<std::function<void()>> callbacksToRun;
	{
		std::lock_guard<std::mutex> lock(postStartCallbacksMutex);
		for (const auto& pair : postStartCallbacks) {
			callbacksToRun.push_back(pair.second);
		}
	}
	for (const auto& func : callbacksToRun) {
		func();
	}
}


void _PostLoadMessage(ScriptState* state) {
	while (true) {
		std::function<void()> task;
		{
			std::lock_guard<std::mutex> lock(postLoadQueueMutex);
			if (postLoadTaskQueue.empty()) break;
			task = std::move(postLoadTaskQueue.front());
			postLoadTaskQueue.pop();
		}
		task();
	}

	std::vector<std::function<void()>> callbacksToRun;
	{
		std::lock_guard<std::mutex> lock(postLoadCallbacksMutex);
		for (const auto& pair : postLoadCallbacks) {
			callbacksToRun.push_back(pair.second);
		}
	}
	for (const auto& func : callbacksToRun) {
		func();
	}
}


void Binding_GetCommand(void* L) {
	LunarTear::Get().Game().PhaseBindingDispatcher(L, _GetCommand);
}
void Binding_SetCommandResult(void* L) {
	LunarTear::Get().Game().PhaseBindingDispatcher(L, _SetCommandResult);
}
void Binding_PostStartMessage(void* L) {
	LunarTear::Get().Game().PhaseBindingDispatcher(L, _PostStartMessage);
}
void Binding_PostLoadMessage(void* L) {
	LunarTear::Get().Game().PhaseBindingDispatcher(L, _PostLoadMessage);
}