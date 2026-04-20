#ifndef CORE_H
#define CORE_H

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
class Core
{
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

        // Communicator is created explicitly by the UI so it can pass itself
        // (or any QObject) as parent. The pointer is owned by Core.
        Communicator* communicator() const { return m_communicator; }
        Communicator* createCommunicator(QObject* parent);

        // Current active connection. Core owns the pointer — setConnection()
        // deletes any previous one.
        AbstractConnection* connection() const { return m_connection; }
        void setConnection(AbstractConnection* c);

        // Destructor must be public so the Meyers-singleton cleanup code
        // (emitted in translation units that include this header) can
        // destroy the static instance.
        ~Core();

    private:
        Core();

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
