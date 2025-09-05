#pragma once
#define NOMINMAX
#include <Windows.h> 
#include <LunarTear.h>
#include <string>
#include <vector>
#include <functional>
#include <sstream>
#include <stdexcept>

class LunarTear;

class LunarTearUninitializedError : public std::exception {
public:
    explicit LunarTearUninitializedError(const std::string& msg =
        "LunarTear API not initialized")
        : m_msg(msg) {
    }

    const char* what() const noexcept override {
        return m_msg.c_str();
    }

private:
    std::string m_msg;
};

class LuaResult {
public:
    LuaResult(const LT_LuaResult& c_result) : m_result(c_result) {}

    LT_LuaResultType GetType() const { return m_result.type; }

    // Helper to quickly check if the script execution resulted in an error.
    bool IsError() const {
        return m_result.type == LT_LUA_RESULT_ERROR_SYNTAX ||
            m_result.type == LT_LUA_RESULT_ERROR_RUNTIME ||
            m_result.type == LT_LUA_RESULT_ERROR_UNSUPPORTED_TYPE;
    }

    // Helper to check if a number value has no fractional part.
    bool IsInteger() const {
        if (m_result.type != LT_LUA_RESULT_NUMBER) {
            return false;
        }
        return m_result.value.numberValue == std::floor(m_result.value.numberValue);
    }

    // Safely get the result as a double.
    // Returns the provided default value if the result was not a number
    double AsDouble(double defaultValue = 0.0) const {
        if (m_result.type == LT_LUA_RESULT_NUMBER) {
            return m_result.value.numberValue;
        }
        return defaultValue;
    }

    // Safely get the result as an integer.
    // Returns the provided default value if the result was not a number
    long long AsInteger(long long defaultValue = 0) const {
        if (m_result.type == LT_LUA_RESULT_NUMBER) {
            return static_cast<long long>(m_result.value.numberValue);
        }
        return defaultValue;
    }

    // Safely get the result as a string.
    // Returns the provided default value if the result was not a string or error.
    std::string AsString(const std::string& defaultValue = "") const {
        if (m_result.type == LT_LUA_RESULT_STRING || IsError()) {
            return m_result.value.stringValue ? m_result.value.stringValue : "";
        }
        return defaultValue;
    }

private:
    const LT_LuaResult& m_result;
};


class LunarTearGameAPI {
public:
    LunarTearGameAPI(LT_GameAPI* gameApi) : m_gameApi(gameApi) {
        if (!m_gameApi) {
            throw std::runtime_error("LT_GameAPI pointer is null during initialization.");
        }
    }

    int LuaBindingDispatcher(void* luaState, void* CFunc) { return m_gameApi->luaBindingDispatcher(luaState, CFunc); }
    void* GetArgumentPointer(void* argBuffer, int index) { return m_gameApi->GetArgumentPointer(argBuffer, index); }
    const char* GetArgumentString(void* arg) { return m_gameApi->GetArgumentString(arg); }
    int GetArgumentInt(void* arg) { return m_gameApi->GetArgumentInt(arg); }
    float GetArgumentFloat(void* arg) { return m_gameApi->GetArgumentFloat(arg); }
    void* SetArgumentFloat(void* returnBuffer, float value) { return m_gameApi->SetArgumentFloat(returnBuffer, value); }
    void* SetArgumentInt(void* returnBuffer, int value) { return m_gameApi->SetArgumentInt(returnBuffer, value); }
    void* SetArgumentString(void* returnBuffer, const char* value) { return m_gameApi->SetArgumentString(returnBuffer, value); }

    bool AnyEndingSeen() { return m_gameApi->AnyEndingSeen(GetEndingsData()); }
    bool EndingESeen() { return m_gameApi->EndingESeen(GetEndingsData()); }
    bool CheckRouteEActive() { return m_gameApi->CheckRouteEActive(GetEndingsData()); }
    std::string GetLocalizedString(int strId) { return m_gameApi->GetLocalizedString(GetLocaleData(), strId); }
    int GetCurrentLevel() { return m_gameApi->getCurrentLevel(); }
    uint64_t GetPlayerItemCount(unsigned int itemId) { return m_gameApi->getPlayerItemCount(GetPlayerSaveData(), itemId); }
    void SetCharacterLevelInSaveData(unsigned int charIndex, int newLevel) { m_gameApi->setCharacterLevelInSaveData(GetPlayerSaveData(), charIndex, newLevel); }
    void SetPlayerItemCount(unsigned int itemId, char count) { m_gameApi->setPlayerItemCount(GetPlayerSaveData(), itemId, count); }
    void SpawnEnemyGroup(int EnemyGroupID, unsigned char param_2, float param_3) { m_gameApi->SpawnEnemyGroup(EnemyGroupID, param_2, param_3); }

    void GameFlagOn(unsigned int flagId) { m_gameApi->GameFlagOn(nullptr, flagId); }
    void GameFlagOff(unsigned int flagId) { m_gameApi->GameFlagOff(nullptr, flagId); }
    bool IsGameFlag(unsigned int flagId) { return m_gameApi->IsGameFlag(nullptr, flagId) != 0; }
    void SnowGameFlagOn(unsigned int flagId) { m_gameApi->SnowGameFlagOn(nullptr, flagId); }
    void SnowGameFlagOff(unsigned int flagId) { m_gameApi->SnowGameFlagOff(nullptr, flagId); }
    bool IsSnowGameFlag(unsigned int flagId) { return m_gameApi->IsSnowGameFlag(nullptr, flagId) != 0; }

    PlayableManager* GetPlayableManagerInstance() { return m_gameApi->GetPlayableManager(); }
    ActorPlayable* GetActorPlayable() {

        PlayableManager* pm = GetPlayableManagerInstance();
        return pm ? m_gameApi->GetActorPlayable(pm) : nullptr;
    }

    uintptr_t GetProcessBaseAddress() const { return m_gameApi->processBaseAddress; }
    PhaseScriptManager* GetPhaseScriptManager() const { return m_gameApi->phaseScriptManager; }
    RootScriptManager* GetRootScriptManager() const { return m_gameApi->rootScriptManager; }
    GameScriptManager* GetGameScriptManager() const { return m_gameApi->gameScriptManager; }
    PlayerSaveData* GetPlayerSaveData() const { return m_gameApi->playerSaveData; }
    EndingsData* GetEndingsData() const { return m_gameApi->endingsData; }
    void* GetLocaleData() const { return m_gameApi->localeData; }
    CPlayerParam* GetPlayerParam() const { return m_gameApi->playerParam; }

private:
    LT_GameAPI* m_gameApi;
};

class LunarTear {
public:
    class LogStream {
    public:
        LogStream(LT_LogLevel level, const LunarTearAPI* api, LT_PluginHandle handle)
            : m_level(level), m_api(api), m_handle(handle) {
        }

        ~LogStream() {
            if (m_api && m_handle && !m_buffer.str().empty()) {
                m_api->Log(m_handle, m_level, m_buffer.str().c_str());
            }
        }

        template <typename T>
        LogStream& operator<<(const T& value) {
            m_buffer << value;
            return *this;
        }

    private:
        std::stringstream m_buffer;
        LT_LogLevel m_level;
        const LunarTearAPI* m_api;
        LT_PluginHandle m_handle;
    };

  

public:

    using ScriptCallback = std::function<void(const LuaResult&)>;

    static void Init(const LunarTearAPI* api, LT_PluginHandle handle) {
        if (!s_instance) {
            s_instance = new LunarTear(api, handle);
        }
    }

    static LunarTear& Get() {
        if (!s_instance) {
            throw LunarTearUninitializedError("Lunar Tear API not initialised");
        }
        return *s_instance;
    }

    uint32_t GetVersion() const {
        return m_api->api_version;
    }

    LogStream Log(LT_LogLevel level) {
        return LogStream(level, m_api, m_handle);
    }

    std::string GetConfigString(const std::string& section, const std::string& key, const std::string& defaultValue) {
        std::vector<char> buffer(256);
        int size = m_api->Config_GetString(m_handle, section.c_str(), key.c_str(), defaultValue.c_str(), buffer.data(), static_cast<uint32_t>(buffer.size()));
        if (size > static_cast<int>(buffer.size() - 1)) {
            buffer.resize(size + 1);
            size = m_api->Config_GetString(m_handle, section.c_str(), key.c_str(), defaultValue.c_str(), buffer.data(), static_cast<uint32_t>(buffer.size()));
        }
        return std::string(buffer.data(), size);
    }

    long GetConfigInt(const std::string& section, const std::string& key, long defaultValue) {
        return m_api->Config_GetInteger(m_handle, section.c_str(), key.c_str(), defaultValue);
    }

    double GetConfigReal(const std::string& section, const std::string& key, double defaultValue) {
        return m_api->Config_GetReal(m_handle, section.c_str(), key.c_str(), defaultValue);
    }

    bool GetConfigBool(const std::string& section, const std::string& key, bool defaultValue) {
        return m_api->Config_GetBoolean(m_handle, section.c_str(), key.c_str(), defaultValue);
    }

    LT_HookStatus CreateHook(void* pTarget, void* pDetour, void** ppOriginal) {
        return m_api->Hook_Create(m_handle, pTarget, pDetour, ppOriginal);
    }

    LT_HookStatus EnableHook(void* pTarget) {
        return m_api->Hook_Enable(m_handle, pTarget);
    }

    bool RegisterLuaCFunc(const std::string& functionName, LT_LuaCFunc pFunc) {
        return m_api->Lua_RegisterCFunc(m_handle, functionName.c_str(), pFunc);
    }

    void QueuePhaseScriptCall(const std::string& functionName) {
        m_api->Lua_QueuePhaseScriptCall(m_handle, functionName.c_str());
    }

    void QueuePhaseUpdateCallback(std::function<void()> func) {
        if (!func) return;
        // The wrapper will be responsible for calling it and then deleting it
        auto* p_func = new std::function<void()>(std::move(func));
        m_api->QueuePhaseUpdateCallback(m_handle, &LambdaWrapper, p_func);
    }


    void QueuePhaseScriptExecution(const std::string& script, ScriptCallback callback = nullptr) {
        if (!callback) {
            // Fire and forget call with no callback needed
            m_api->Lua_QueuePhaseScriptExecution(m_handle, script.c_str(), nullptr, nullptr);
        }
        else {

            auto* p_func = new ScriptCallback(std::move(callback));
            m_api->Lua_QueuePhaseScriptExecution(m_handle, script.c_str(), &ScriptLambdaWrapper, p_func);
        }
    }

    bool IsModActive(const std::string& modName) {
        return m_api->IsModActive(m_handle, modName.c_str());
    }

    bool IsPluginActive(const std::string& pluginName) {
        return m_api->IsPluginActive(m_handle, pluginName.c_str());
    }

    std::string GetModDirectory(const std::string& modName) {
        std::vector<char> buffer(MAX_PATH);
        int size = m_api->GetModDirectory(m_handle, modName.c_str(), buffer.data(), static_cast<uint32_t>(buffer.size()));
        if (size < 0) return "";

        if (size > static_cast<int>(buffer.size() - 1)) {
            buffer.resize(size + 1);
            size = m_api->GetModDirectory(m_handle, modName.c_str(), buffer.data(), static_cast<uint32_t>(buffer.size()));
        }
        return std::string(buffer.data(), size);
    }



    LunarTearGameAPI& Game() {
        return *m_game;
    }

    const LunarTearAPI* GetRawAPI() const {
        return m_api;
    }



private:
    LunarTear(const LunarTearAPI* api, LT_PluginHandle handle)
        : m_api(api), m_handle(handle) {
        if (!api || !api->game) {
            throw std::runtime_error("LunarTear API pointers are null during initialization.");
        }
        m_game = new LunarTearGameAPI(api->game);
    }

    ~LunarTear() {
        delete m_game;
    }


    static void ScriptLambdaWrapper(const LT_LuaResult* result, void* userData) {
        if (!userData || !result) return;

        auto* p_func = static_cast<ScriptCallback*>(userData);

        LuaResult cpp_result(*result);

        (*p_func)(cpp_result);
        delete p_func;
    }


    static void LambdaWrapper(void* userData) {
        if (!userData) return;
        auto* p_func = static_cast<std::function<void()>*>(userData);
        (*p_func)(); 
        delete p_func; 
    }

    const LunarTearAPI* m_api;
    LT_PluginHandle m_handle;
    LunarTearGameAPI* m_game;

    inline static LunarTear* s_instance = nullptr;
};

