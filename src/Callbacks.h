#pragma once
#include <queue>
#include <vector>
#include <map>
#include <functional>
#include <mutex>
#include <atomic>

extern std::queue<std::function<void()>> postStartTaskQueue;
extern std::mutex postStartQueueMutex;

extern std::queue<std::function<void()>> postLoadTaskQueue;
extern std::mutex postLoadQueueMutex;

void enqueuePostLoadTask(std::function<void()> task);
void enqueuePostStartTask(std::function<void()> task);

using CallbackId = uint64_t;

extern std::map<CallbackId, std::function<void()>> postLoadCallbacks;
extern std::mutex postLoadCallbacksMutex;

extern std::map<CallbackId, std::function<void()>> postStartCallbacks;
extern std::mutex postStartCallbacksMutex;

CallbackId registerPostLoadCallback(std::function<void()> callback);
void unregisterPostLoadCallback(CallbackId id);

CallbackId registerPostStartCallback(std::function<void()> callback);
void unregisterPostStartCallback(CallbackId id);