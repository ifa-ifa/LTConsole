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

    if (GetKeyState(VK_CAPITAL) & 0x0001) {
        m_capsToggled = true;
    }

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

void Terminal::focusOutEvent(QFocusEvent* e)
{
    m_shiftHeld = false;
    m_ctrlHeld = false;
    QPlainTextEdit::focusOutEvent(e);
}

void Terminal::keyReleaseEvent(QKeyEvent* e)
{
    if (e->key() == Qt::Key_Shift)   m_shiftHeld = false;
    if (e->key() == Qt::Key_Control) m_ctrlHeld = false;

    QPlainTextEdit::keyReleaseEvent(e);
}

void Terminal::keyPressEvent(QKeyEvent* e)
{

    if (e->key() == Qt::Key_Shift) {
        m_shiftHeld = true; return;
    }
    if (e->key() == Qt::Key_Control) {
        m_ctrlHeld = true; return;
    }
    if (e->key() == Qt::Key_CapsLock) {
        m_capsToggled = !m_capsToggled; return;
    }

    if (m_ctrlHeld)
    {
        switch (e->key()) {
        case Qt::Key_A:
        {

            QTextCursor cursor = textCursor();
            cursor.setPosition(m_inputStartPosition);
            cursor.movePosition(QTextCursor::End, QTextCursor::KeepAnchor);
            setTextCursor(cursor);
            return;
        }
        case Qt::Key_C:
            copySelection();
            return;
        case Qt::Key_X:
            if (!isReadOnly() && textCursor().hasSelection() && textCursor().selectionStart() >= m_inputStartPosition) {
                cut();
            }
            return;
        case Qt::Key_V:
            if (!isReadOnly()) paste();
            return;

        case Qt::Key_Left:
        case Qt::Key_Right:
        case Qt::Key_Backspace:
        case Qt::Key_Delete:
        case Qt::Key_Home:
        case Qt::Key_End:
            break;

        default:
            return;
        }
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
    bool isEditingKey = (e->key() == Qt::Key_Backspace || e->key() == Qt::Key_Delete);
    bool isNavKey = (e->key() == Qt::Key_Left || e->key() == Qt::Key_Right ||
        e->key() == Qt::Key_Home || e->key() == Qt::Key_End ||
        e->key() == Qt::Key_PageUp || e->key() == Qt::Key_PageDown);


    if (isEditingKey && cursor.hasSelection())
    {
        if (cursor.selectionStart() < m_inputStartPosition)
        {
            if (cursor.selectionEnd() <= m_inputStartPosition) {
                return;
            }


            cursor.setPosition(m_inputStartPosition);
            cursor.setPosition(textCursor().selectionEnd(), QTextCursor::KeepAnchor);
            setTextCursor(cursor);

            QPlainTextEdit::keyPressEvent(e);
            return;
        }
    }

    if (cursor.position() < m_inputStartPosition)
    {
        if (isNavKey || e->matches(QKeySequence::Copy)) {
            QPlainTextEdit::keyPressEvent(e);
        }
        return;
    }

    if (e->key() == Qt::Key_Backspace && !cursor.hasSelection() && cursor.position() == m_inputStartPosition) {
        return; 
    }

    if (e->key() == Qt::Key_Delete || isNavKey || isEditingKey) {
        QPlainTextEdit::keyPressEvent(e);
        return;
    }


    QChar inputChar;
    int key = e->key();

    if (key >= Qt::Key_A && key <= Qt::Key_Z) {
        bool isUpper = m_shiftHeld ^ m_capsToggled;
        inputChar = isUpper ? QChar(key) : QChar(key).toLower();
    }
    else if (m_shiftHeld && key >= Qt::Key_0 && key <= Qt::Key_9) {
        const char* symbols = ")!@#$%^&*(";
        if (key - Qt::Key_0 < 10) inputChar = QChar(symbols[key - Qt::Key_0]);
    }
    else if (key == Qt::Key_Space) {
        inputChar = ' ';
    }
    else {
        if (!e->text().isEmpty()) {
            inputChar = e->text().at(0);
            if (inputChar.isLetter()) {
                bool isUpper = m_shiftHeld ^ m_capsToggled;
                inputChar = isUpper ? inputChar.toUpper() : inputChar.toLower();
            }
        }
    }

    if (!inputChar.isNull() && inputChar.isPrint())
    {
        insertPlainText(QString(inputChar));
    }
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