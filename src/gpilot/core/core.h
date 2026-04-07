#ifndef CORE_H
#define CORE_H

#include "core/macro/macros.h"
#include "core/utils/filesmanager.h"
#include "core/config/configuration.h"

class Core
{
    public:
        static Core& instance() {
            static Core instance;

            return instance;
        }

        Core(const Core&) = delete;
        Core& operator=(const Core&) = delete;

        // bool loadConfiguration(const QString& configType);
        Configuration& configuration() { return m_configuration; }

        Macros& macros() { return m_macros; }
        const Macros& macros() const { return m_macros; }

        FilesManager& filesManager() { return FilesManager::instance(); }

    private:
        Core();

        Configuration m_configuration;
        Macros m_macros;
};

#endif // CORE_H
