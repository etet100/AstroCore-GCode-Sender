#pragma once

#include <QObject>
#include <functional>
#include "core/globals.h"

class AbstractConnection;

// Called when a command gets a final response (ok/error) from the machine.
// Return true  = behavior processed it (Ok).
// Return false = behavior wants the command returned to the front of the buffer.
using CommandResponseHandler = std::function<bool(
    const QString& command,
    CommandAttributes& attributes,
    const CmdStatus& status,
    const QString& data,
    const QStringList& lines
)>;

// Called by CommandBuffer to drain queued commands through the normal send path.
// This allows Communicator::sendCommand() to keep its special-case logic (M2/M30 etc.)
// while the queue drain happens inside CommandBuffer.
using QueuedCommandSender = std::function<SendCommandResult(
    CommandSource source,
    const QString& commandLine,
    int tableIndex,
    CommandCallback callback
)>;

// Manages the two-level GRBL command buffer:
//   m_commands — commands currently in the 127-byte GRBL serial buffer, awaiting response.
//   m_queue    — software wait queue for commands that cannot be sent yet.
class CommandBuffer : public QObject
{
    Q_OBJECT

public:
    explicit CommandBuffer(AbstractConnection *connection);
    void setConnection(AbstractConnection *connection);

    void setResponseHandler(CommandResponseHandler handler);
    void setQueuedCommandSender(QueuedCommandSender sender);

    // Place command into the buffer or queue.
    // If wait=true or the GRBL buffer is full, it goes to m_queue.
    SendCommandResult enqueue(CommandSource source, const QString& commandLine,
                              int tableIndex, bool wait,
                              CommandCallback callback = nullptr);

    void sendRealtime(const QString& command);
    void sendRealtime(int command);

    void clear();
    void clearQueue();

    int  bufferLength() const;
    bool willOverflow(const QString& command) const;

    bool isEmpty()      const { return m_commands.isEmpty(); }
    bool isQueueEmpty() const { return m_queue.isEmpty(); }

    // Direct access needed by CommunicatorApi and onConnectionLineReceived checks.
    QList<CommandAttributes>& commands() { return m_commands; }

    // Called when a line arrives from the machine that may be a command response.
    // Accumulates multi-line responses, then processes the final ok/error line.
    // Returns true if the behavior handler returned Ok (state transition may be pending).
    bool processResponse(const QString& data);

signals:
    void commandSent(CommandAttributes attributes);
    // Emitted after a command response is fully processed.
    // Communicator uses this to handle special commands ($#) and forward commandResponseReceived.
    void commandCompleted(CommandAttributes attributes, CmdStatus status, QStringList lines);

private:
    static const int BUFFER_SIZE = 127;

    AbstractConnection *m_connection;
    QList<CommandAttributes> m_commands;
    QList<CommandQueue> m_queue;
    int m_commandIndex = 0;

    CommandResponseHandler m_responseHandler;
    QueuedCommandSender m_queuedCommandSender;

    // Accumulates lines of a multi-line response (e.g., $#, $$) until ok/error arrives.
    QString m_responseAccumulator;
    QStringList m_responseLines;

    void drainQueue();
};
