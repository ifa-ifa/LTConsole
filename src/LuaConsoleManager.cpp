#include "LuaConsoleManager.h"
#include <LunarTear++.h>
#include <QMetaObject>
#include "ui/Terminal.h"


constexpr int COMMAND_TIMEOUT_SECONDS = 3;

LuaConsoleManager& LuaConsoleManager::instance() {
    static LuaConsoleManager s;
    return s;
}

LuaConsoleManager::LuaConsoleManager() {

    m_reaperTimer.setInterval(1000);    
    QObject::connect(&m_reaperTimer, &QTimer::timeout, this, &LuaConsoleManager::reapTimedOutCommands);
    m_reaperTimer.start();

};
LuaConsoleManager::~LuaConsoleManager() = default;

uint64_t LuaConsoleManager::registerTerminal(Terminal* term) {
    if (!term) return 0;
    uint64_t id = m_nextId.fetch_add(1, std::memory_order_relaxed);
    {
        std::lock_guard<std::mutex> lk(m_termMutex);
        m_terminals.emplace(id, QPointer<Terminal>(term));
    }
    return id;
}

void LuaConsoleManager::unregisterTerminal(uint64_t terminalId) {
    std::lock_guard<std::mutex> lk(m_termMutex);
    m_terminals.erase(terminalId);
    uint64_t cur = m_currentExecutingId.load(std::memory_order_relaxed);
    if (cur == terminalId) m_currentExecutingId.store(0, std::memory_order_relaxed);
}

void LuaConsoleManager::enqueueCommand(uint64_t terminalId, const QString& command) {
    if (terminalId == 0) return;
    {
        std::lock_guard<std::mutex> lk(m_queueMutex);
        m_queue.push(PendingCmd{ terminalId, command.toUtf8().constData(), std::chrono::steady_clock::now() });
    }

    try {
        LunarTear::Get().QueuePhaseScriptCall("ifaifa_LTCon_ExecuteCommand");
    }
    catch (const LunarTearUninitializedError& e) {
        QString errorMsg = QString("LTCON Error: Could not send command: %1").arg(e.what());


        QPointer<Terminal> term;
        {
            std::lock_guard<std::mutex> lk(m_termMutex);
            auto it = m_terminals.find(terminalId);
            if (it != m_terminals.end()) term = it->second;
        }

        if (term) {
            QMetaObject::invokeMethod(
                term,
                "appendOutput",
                Qt::QueuedConnection,
                Q_ARG(QString, errorMsg)
            );
        }
        // The command will sit in the queue, but the UI is now unlocked and the user
        // knows something went wrong. The next successful command will process the old one
    }
}


bool LuaConsoleManager::popNextCommand(std::string& outCommand, uint64_t& outTerminalId) {
    std::lock_guard<std::mutex> lk(m_queueMutex);
    if (m_queue.empty()) {
        outCommand.clear();
        outTerminalId = 0;
        return false;
    }
    auto pc = m_queue.front();
    m_queue.pop();
    outCommand = std::move(pc.cmd);
    outTerminalId = pc.terminalId;

    m_currentExecutingId.store(outTerminalId, std::memory_order_relaxed);
    return true;
}


void LuaConsoleManager::postResultForCurrentExecuting(const QString& result) {
    uint64_t terminalId = m_currentExecutingId.load(std::memory_order_relaxed);
    if (terminalId == 0) return;


    QPointer<Terminal> term;
    {
        std::lock_guard<std::mutex> lk(m_termMutex);
        auto it = m_terminals.find(terminalId);
        if (it != m_terminals.end()) term = it->second;
    }

    if (term) {

        QMetaObject::invokeMethod(
            term,
            "appendOutput",
            Qt::QueuedConnection,
            Q_ARG(QString, result)
        );
    }

    m_currentExecutingId.store(0, std::memory_order_relaxed);
}

void LuaConsoleManager::reapTimedOutCommands() {
    const auto now = std::chrono::steady_clock::now();
    const auto timeout = std::chrono::seconds(COMMAND_TIMEOUT_SECONDS);
    QString errorMsg = QString("LTCON Error: Command timed out");

    std::lock_guard<std::mutex> lk(m_queueMutex);

    while (!m_queue.empty() && (now - m_queue.front().creationTime > timeout)) {
        PendingCmd timedOutCmd = m_queue.front();
        m_queue.pop(); 

        QPointer<Terminal> term;
        {
            std::lock_guard<std::mutex> term_lk(m_termMutex);
            auto it = m_terminals.find(timedOutCmd.terminalId);
            if (it != m_terminals.end()) term = it->second;
        }
        if (term) {
            QMetaObject::invokeMethod(term, "appendOutput", Qt::QueuedConnection, Q_ARG(QString, errorMsg));
        }
    }
}