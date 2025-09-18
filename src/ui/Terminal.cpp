#include "terminal.h"
#include <windows.h>
#include <winuser.h>
#include <QScrollBar>
#include <QTextCursor>
#include <QChar>
#include <QApplication>
#include <QClipboard>
#include <QMenu>
#include <QContextMenuEvent>
#include <QTextBlock>
#include <QTextDocumentFragment>
#include <iostream>


// function to manually and reliably translate a QKeyEvent to a QChar
// using the WinAPI. This is necessary becuase the game
// interferes with the keyboard input
QChar getCharFromKeyEvent(QKeyEvent* e)
{
    BYTE keyboardState[256];
    if (!GetKeyboardState(keyboardState)) {
        return QChar();
    }

    if (e->modifiers() & Qt::ShiftModifier) {
        keyboardState[VK_SHIFT] |= 0x80;
    }
    else {
        keyboardState[VK_SHIFT]  &= ~0x80;
    }
    if (e->modifiers() & Qt::ControlModifier) {
        keyboardState[VK_CONTROL] |= 0x80;
    }
    else {
        keyboardState[VK_CONTROL] &= ~0x80;
    }
    if (e->modifiers() & Qt::AltModifier) {
        keyboardState[VK_MENU] |= 0x80;
    }
    else {

        keyboardState[VK_MENU] &= ~0x80;
    }

    wchar_t buffer[2] = { 0,0 };
    int res = ToUnicode(e->nativeVirtualKey(), e->nativeScanCode(), keyboardState, buffer, 2, 0);

    if (res > 0) {
        return QChar(buffer[0]);
    }

    return QChar(); 
}


Terminal::Terminal(QWidget* parent)
    : QPlainTextEdit(parent),
    m_prompt(">> "),
    m_inputStartPosition(0),
    m_historyPos(0)
{
    setStyleSheet("QPlainTextEdit {"
        "    background-color: #282c34;"
        "    color: #abb2bf;"
        "    border: 1px solid #2c313a;"
        "    font-family: 'Consolas', 'Courier New', monospace;"
        "    font-size: 14px;"
        "    padding: 6px;"
        "}"
        "QPlainTextEdit:focus {"
        "    border: 1px solid #61afef;"
        "}");

    insertPrompt();
}

void Terminal::setPrompt(const QString& prompt)
{
    m_prompt = prompt;
}

void Terminal::appendOutput(const QString& text)
{
    setReadOnly(false);

    moveCursor(QTextCursor::End);

    textCursor().insertText("\n");

    if (!text.isEmpty()) {
        textCursor().insertText(text);
        textCursor().insertText("\n");
    }

    textCursor().insertText(m_prompt);

    m_inputStartPosition = textCursor().position();
    scrollDown();
    setFocus();
}

void Terminal::keyPressEvent(QKeyEvent* e)
{
    if (isReadOnly()) {
        if (e->matches(QKeySequence::Copy)) {
            copySelection();
        }
        return;
    }

    if (e->matches(QKeySequence::Copy)) {
        copySelection();
        return;
    }
    if (e->key() == Qt::Key_Return || e->key() == Qt::Key_Enter) {
        onCommandEntered();
        return;
    }
    if (e->key() == Qt::Key_Up) {
        historyBack();
        return;
    }
    if (e->key() == Qt::Key_Down) {
        historyForward();
        return;
    }

    QTextCursor cursor = textCursor();

    if (cursor.position() < m_inputStartPosition || (cursor.hasSelection() && cursor.anchor() < m_inputStartPosition))
    {
        if (e->text().isEmpty() && e->key() != Qt::Key_Backspace && e->key() != Qt::Key_Delete) {
            QPlainTextEdit::keyPressEvent(e);
        }
        return;
    }

    if (e->key() == Qt::Key_Backspace && !cursor.hasSelection() && cursor.position() == m_inputStartPosition) {
        return;
    }

    QChar ch = getCharFromKeyEvent(e);

    if (!ch.isNull() && ch.isPrint())
    {
        insertPlainText(QString(ch));
        return;
    }

    QPlainTextEdit::keyPressEvent(e);
}

void Terminal::contextMenuEvent(QContextMenuEvent* e)
{
    QMenu menu(this);
    QAction* copyAction = menu.addAction("Copy");
    copyAction->setEnabled(textCursor().hasSelection());
    connect(copyAction, &QAction::triggered, this, &Terminal::copySelection);

    menu.addSeparator();

    QAction* selectAllAction = menu.addAction("Select All");
    connect(selectAllAction, &QAction::triggered, this, &QPlainTextEdit::selectAll);

    menu.exec(e->globalPos());
}

void Terminal::copySelection()
{
    QTextCursor cursor = textCursor();
    if (!cursor.hasSelection()) {
        return;
    }

    QString selectedText = cursor.selection().toPlainText();
    QStringList lines = selectedText.split('\n');
    QStringList cleanedLines;

    for (const QString& line : lines) {
        if (line.startsWith(m_prompt)) {
            cleanedLines.append(line.mid(m_prompt.length()));
        }
        else {
            cleanedLines.append(line);
        }
    }

    QApplication::clipboard()->setText(cleanedLines.join('\n'));
}

void Terminal::onCommandEntered()
{
    QString command = document()->lastBlock().text().mid(m_prompt.length()).trimmed();

    if (!command.isEmpty()) {
        m_history.append(command);
        m_historyPos = m_history.size();
        emit commandEntered(command);


        setReadOnly(true);
    }
    else {
        insertPrompt(true);
    }
}
 

void Terminal::insertPrompt(bool forceNewLine)
{
    moveCursor(QTextCursor::End);
    if (forceNewLine || document()->lastBlock().length() > 0) {
        appendPlainText("");
    }

    textCursor().insertText(m_prompt);
    m_inputStartPosition = textCursor().position();
    scrollDown();
}

void Terminal::scrollDown()
{
    verticalScrollBar()->setValue(verticalScrollBar()->maximum());
}

void Terminal::historyBack()
{
    if (m_history.isEmpty() || m_historyPos == 0) return;

    --m_historyPos;
    QTextCursor cursor = textCursor();
    cursor.setPosition(m_inputStartPosition);
    cursor.movePosition(QTextCursor::End, QTextCursor::KeepAnchor);
    cursor.removeSelectedText();
    cursor.insertText(m_history.at(m_historyPos));
}

void Terminal::historyForward()
{
    if (m_history.isEmpty()) return;

    QTextCursor cursor = textCursor();
    cursor.setPosition(m_inputStartPosition);
    cursor.movePosition(QTextCursor::End, QTextCursor::KeepAnchor);
    cursor.removeSelectedText();

    if (m_historyPos < m_history.size() - 1) {
        ++m_historyPos;
        cursor.insertText(m_history.at(m_historyPos));
    }
    else {
        m_historyPos = m_history.size();
    }
}