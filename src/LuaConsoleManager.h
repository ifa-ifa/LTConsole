#pragma once
#include <mutex>
#include <queue>
#include <unordered_map>
#include <atomic>
#include <QTimer>
#include <optional>
#include <QString>
#include <QPointer>

class Terminal;

class LuaConsoleManager : public QObject {
    Q_OBJECT
public:
    static LuaConsoleManager& instance();

    uint64_t registerTerminal(Terminal* term);
    void unregisterTerminal(uint64_t terminalId);

    void enqueueCommand(uint64_t terminalId, const QString& command);


    bool popNextCommand(std::string& outCommand, uint64_t& outTerminalId);

    // Called from bindings when Lua produced a result
    void postResultForCurrentExecuting(const QString& result);

    void setCurrentExecutingId(uint64_t id) {
        m_currentExecutingId.store(id, std::memory_order_relaxed);
    }

private slots:
    void reapTimedOutCommands();

private:
    LuaConsoleManager();
    ~LuaConsoleManager();

    LuaConsoleManager(const LuaConsoleManager&) = delete;
    LuaConsoleManager& operator=(const LuaConsoleManager&) = delete;

    struct PendingCmd {
        uint64_t terminalId;
        std::string cmd;
        std::chrono::steady_clock::time_point creationTime; 
    };


    QTimer m_reaperTimer;


    std::mutex m_queueMutex;
    std::queue<PendingCmd> m_queue;

    std::mutex m_termMutex;
    std::unordered_map<uint64_t, QPointer<Terminal>> m_terminals;

    std::atomic<uint64_t> m_nextId{ 1 }; // 0 reserved for "none"
    std::atomic<uint64_t> m_currentExecutingId{ 0 }; // 0 == none
};
