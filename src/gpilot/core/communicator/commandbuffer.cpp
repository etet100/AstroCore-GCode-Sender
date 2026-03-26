#include "commandbuffer.h"
#include "io/connection/connection.h"
#include "core/gcode/parser/gcodepreprocessorutils.h"
#include <QDebug>
#include <QRegularExpression>
#include <cassert>

// ---- file-scope helpers (GRBL response classification) ----

static bool dataIsEnd(const QString& data)
{
    return data.startsWith("ok")
        || data.startsWith("error")
        || data.startsWith(">:ok")
        || data.startsWith(">:error");
}

static bool dataIsReset(const QString& data)
{
    static QRegularExpression re(
        "^(GRBL|GCARVIN)\\s\\d\\.\\d.",
        QRegularExpression::CaseInsensitiveOption
    );

    return re.match(data).hasMatch();
}

// -----------------------------------------------------------

CommandBuffer::CommandBuffer(Connection *connection)
    : QObject(nullptr)
    , m_connection(connection)
{
}

void CommandBuffer::setConnection(Connection *connection)
{
    m_connection = connection;
}

void CommandBuffer::setResponseHandler(CommandResponseHandler handler)
{
    m_responseHandler = std::move(handler);
}

void CommandBuffer::setQueuedCommandSender(QueuedCommandSender sender)
{
    m_queuedCommandSender = std::move(sender);
}

SendCommandResult CommandBuffer::enqueue(
    CommandSource source,
    const QString& commandLine,
    int tableIndex,
    bool wait,
    CommandCallback callback)
{
    if (wait || willOverflow(commandLine)) {
        m_queue.append(CommandQueue(source, commandLine, tableIndex, callback));
        return SendCommandResult::Queue;
    }

    CommandAttributes attrs(source, m_commandIndex++, tableIndex, commandLine, callback);
    m_commands.append(attrs);
    m_connection->sendLine(commandLine);

    emit commandSent(attrs);

    return SendCommandResult::Done;
}

void CommandBuffer::sendRealtime(const QString& command)
{
    if (command.length() != 1) return;
    m_connection->sendByteArray(QByteArray(command.toLatin1(), 1));
}

void CommandBuffer::sendRealtime(int command)
{
    QByteArray data;
    data.append(char(command));
    m_connection->sendByteArray(data);
}

void CommandBuffer::clear()
{
    qDebug() << "[CommandBuffer] Clearing commands and queue";
    m_commands.clear();
    clearQueue();
}

void CommandBuffer::clearQueue()
{
    qDebug() << "[CommandBuffer] Clearing queue";
    m_queue.clear();
}

int CommandBuffer::bufferLength() const
{
    int length = 0;
    for (const CommandAttributes& attrs : m_commands) {
        length += attrs.length;
    }

    return length;
}

bool CommandBuffer::willOverflow(const QString& command) const
{
    return (bufferLength() + command.length() + 1) > BUFFER_SIZE;
}

bool CommandBuffer::processResponse(const QString& data)
{
    assert(!m_commands.isEmpty());

    const QString firstCommand = m_commands[0].commandLine;
    const bool isCtrlX = (firstCommand == "[CTRL+X]");

    // Accumulate lines of a multi-line response until the final ok/error arrives.
    const bool waitingForEnd   = !isCtrlX && !dataIsEnd(data);
    const bool waitingForReset =  isCtrlX && !dataIsReset(data);

    if (waitingForEnd || waitingForReset) {
        m_responseAccumulator.append(data + "; ");
        m_responseLines.append(data);

        return false;
    }

    m_responseAccumulator.append(data);

    // Parse the final response line.
    CmdStatus status { .ok = false, .errorCode = 0 };
    if (data == "ok" || data == ">:ok") {
        status.ok = true;
    } else if (data.startsWith("error:")) {
        status.errorCode = data.mid(6).toInt();
        qDebug() << "[CommandBuffer] error" << status.errorCode;
    } else if (data.startsWith(">:error:")) {
        status.errorCode = data.mid(8).toInt();
        qDebug() << "[CommandBuffer] error" << status.errorCode;
    } else {
        qDebug() << "[CommandBuffer] unknown response status:" << data;
        assert(false);
    }

    CommandAttributes attrs = m_commands.takeFirst();
    const QString command = GcodePreprocessorUtils::removeComment(attrs.commandLine).toUpper();

    bool behaviorHandledIt = false;

    if (m_responseHandler) {
        bool ok = m_responseHandler(command, attrs, status, data, m_responseLines);
        if (!ok) {
            // Behavior wants to process this command again on the next response.
            qDebug() << "[CommandBuffer] Returning command to front:" << attrs.commandLine;
            m_commands.prepend(attrs);
        } else {
            behaviorHandledIt = true;
        }
    }

    attrs.response = m_responseAccumulator;
    const QStringList completedLines = m_responseLines;

    // Clear accumulators for the next command.
    m_responseAccumulator.clear();
    m_responseLines.clear();

    emit commandCompleted(attrs, status, completedLines);

    drainQueue();

    return behaviorHandledIt;
}

void CommandBuffer::drainQueue()
{
    if (!m_queuedCommandSender || m_queue.isEmpty()) return;

    // Guard against re-entrance (drainQueue can be triggered recursively
    // when the queued command sender calls back into enqueue).
    static bool draining = false;
    if (draining) return;

    draining = true;

    while (!m_queue.isEmpty()) {
        CommandQueue queued = m_queue.takeFirst();
        SendCommandResult r = m_queuedCommandSender(
            queued.source, queued.commandLine, queued.tableIndex, queued.callback
        );

        if (r == SendCommandResult::Done) {
            // One command sent — stop until next response arrives.
            break;
        }

        if (r == SendCommandResult::Queue) {
            // Buffer full — the sender re-added it to m_queue as the last element.
            // Move it back to the front to preserve order.
            if (!m_queue.isEmpty()) {
                m_queue.prepend(m_queue.takeLast());
            }
            break;
        }
    }

    draining = false;
}
