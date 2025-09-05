#pragma once

#include <QPlainTextEdit>
#include <QKeyEvent>
#include <QStringList>

class Terminal : public QPlainTextEdit
{
    Q_OBJECT

public:
    explicit Terminal(QWidget* parent = nullptr);

    void setPrompt(const QString& prompt);

public slots: 
    void appendOutput(const QString& text);

signals:
    void commandEntered(const QString& command);

protected:
    void keyPressEvent(QKeyEvent* e) override;
    void contextMenuEvent(QContextMenuEvent* e) override;

private:
    void onCommandEntered();
    void insertPrompt(bool forceNewLine = false);
    void scrollDown();
    void historyBack();
    void historyForward();
    void copySelection();

    QString m_prompt;
    int m_inputStartPosition;
    QStringList m_history;
    int m_historyPos;
};