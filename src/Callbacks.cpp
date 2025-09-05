#include "Callbacks.h"


std::queue<std::function<void()>> postLoadTaskQueue;
std::mutex postLoadQueueMutex;

std::queue<std::function<void()>> postStartTaskQueue;
std::mutex postStartQueueMutex;


void enqueuePostLoadTask(std::function<void()> task) {
    std::lock_guard<std::mutex> lock(postLoadQueueMutex);
    postLoadTaskQueue.push(std::move(task));
}

void enqueuePostStartTask(std::function<void()> task) {
    std::lock_guard<std::mutex> lock(postStartQueueMutex);
    postStartTaskQueue.push(std::move(task));
}

std::map<CallbackId, std::function<void()>> postLoadCallbacks;
std::mutex postLoadCallbacksMutex;
std::atomic<CallbackId> nextPostLoadCallbackId = 1;

std::map<CallbackId, std::function<void()>> postStartCallbacks;
std::mutex postStartCallbacksMutex;
std::atomic<CallbackId> nextPostStartCallbackId = 1;


CallbackId registerPostLoadCallback(std::function<void()> callback) {
    std::lock_guard<std::mutex> lock(postLoadCallbacksMutex);
    CallbackId id = nextPostLoadCallbackId.fetch_add(1);
    postLoadCallbacks[id] = std::move(callback);
    return id;
}

void unregisterPostLoadCallback(CallbackId id) {
    std::lock_guard<std::mutex> lock(postLoadCallbacksMutex);
    postLoadCallbacks.erase(id);
}

CallbackId registerPostStartCallback(std::function<void()> callback) {
    std::lock_guard<std::mutex> lock(postStartCallbacksMutex);
    CallbackId id = nextPostStartCallbackId.fetch_add(1);
    postStartCallbacks[id] = std::move(callback);
    return id;
}

void unregisterPostStartCallback(CallbackId id) {
    std::lock_guard<std::mutex> lock(postStartCallbacksMutex);
    postStartCallbacks.erase(id);
}