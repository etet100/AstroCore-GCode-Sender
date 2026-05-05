#ifndef CORE_H
#define CORE_H

#include <QObject>
#include <optional>
#include "core/macro/macros.h"
#include "core/utils/filesmanager.h"
#include "core/config/configuration.h"
#include "core/gcode/gcode.h"
#include "core/heightmap/heightmap.h"
#include "core/gcode/parser/gcodeviewparser.h"
#include "core/utils/timer.h"
#include "core/utils/programtimeestimator.h"

class ConnectionManager;
class Communicator;
class AbstractConnection;

// Core owns the persistent domain state that is not tied to any UI widget:
// configuration, macros, G-code program, heightmap, view parsers, the time
// estimator and the communicator. Lifetime is tied to the Meyers singleton —
// constructed on first access (after QApplication is created) and destroyed
// at program exit.
class Core : public QObject
{
    Q_OBJECT

    public:
        static Core& instance() {
            static Core instance;

            return instance;
        }

        Core(const Core&) = delete;
        Core& operator=(const Core&) = delete;

        Configuration& configuration() { return m_configuration; }

        Macros& macros() { return m_macros; }
        const Macros& macros() const { return m_macros; }

        FilesManager& filesManager() { return FilesManager::instance(); }

        // Stable references — drawers / behaviors / models may hold their
        // address for the full application lifetime.
        GCode& program() { return m_program; }
        Heightmap& heightmap() { return m_heightmap; }
        GCodeViewParser& viewParser() { return m_viewParser; }
        GCodeViewParser& probeParser() { return m_probeParser; }
        Timer& timer() { return m_timer; }
        ProgramTimeEstimator& timeEstimator() { return m_timeEstimator; }

        // Lazy — created on first access. Safe to call before configuration
        // is fully initialised because ConnectionManager only stores a
        // reference to ConfigurationConnection and dereferences it later.
        ConnectionManager& connectionManager();

        // Lazy core object owned by Core. UI code must not be its QObject
        // parent, otherwise Qt will destroy it with the window.
        Communicator* communicator();

        // Current active connection. Core owns the pointer — setConnection()
        // deletes any previous one.
        AbstractConnection* connection() const { return m_connection; }
        void setConnection(AbstractConnection* c);

        // Destructor must be public so the Meyers-singleton cleanup code
        // (emitted in translation units that include this header) can
        // destroy the static instance.
        ~Core();

        // Routes a raw console line through a small chain of handlers:
        // internal command (`:` prefix) → macro match → scanned g-code/grbl.
        // Each step can stop processing or rewrite the command before the
        // remaining steps and the final send via Communicator::sendCommand.
        void handleConsoleCommand(QString command);

        // Recent files list mutation. Each call updates ConfigurationUI,
        // persists the change and emits recentFilesChanged() so that
        // interested UI parts (menus) can refresh themselves.
        void addRecentFile(QString fileName);
        void addRecentHeightmap(QString fileName);
        void clearRecentFiles(bool heightmapMode);

    signals:
        void log(QString message);
        void openFileRequested();
        void recentFilesChanged();

    private:
        Core();

        // Outcome of a single console-command handler step.
        // `rewritten` replaces the command for the next steps when present.
        struct ConsoleCommandResult {
            enum class Action { Stop, Forward };
            Action action = Action::Forward;
            std::optional<QString> rewritten;

            static ConsoleCommandResult stop() { return {Action::Stop, std::nullopt}; }
            static ConsoleCommandResult forward() { return {Action::Forward, std::nullopt}; }
            static ConsoleCommandResult forwardAs(QString s) { return {Action::Forward, std::move(s)}; }
        };

        void reloadMacros();

        ConsoleCommandResult tryHandleInternalCommand(const QString& command);
        ConsoleCommandResult tryHandleMacro(const QString& command);
        ConsoleCommandResult tryHandleScanned(const QString& command);

        Configuration m_configuration;
        Macros m_macros;
        GCode m_program;
        Heightmap m_heightmap;
        GCodeViewParser m_viewParser;
        GCodeViewParser m_probeParser;
        Timer m_timer;
        ProgramTimeEstimator m_timeEstimator;
        ConnectionManager* m_connectionManager = nullptr;
        Communicator* m_communicator = nullptr;
        AbstractConnection* m_connection = nullptr;
};

#endif // CORE_H
