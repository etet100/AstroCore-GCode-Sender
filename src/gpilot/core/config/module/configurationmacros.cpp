#include "configurationmacros.h"

const QMap<QString, QVariant> DEFAULTS = {
    {"macros", QVariant::fromValue(QList<MacroItem>{
        {.name = "Test macro 1", .type = 0, .enabled = true, .content = "G0 X0 Y0"},
        {.name = "Test macro 2", .type = 0, .enabled = false, .content = "G0 X0 Y0\nM2"},
    })},
};

ConfigurationMacros::ConfigurationMacros(QObject *parent) : ConfigurationModule(parent, DEFAULTS)
{
    ConfigurationRegistry::registerStruct(
        "MacroItem",
        [](const char* data) -> QVariantMap {
            MacroItem* item = (MacroItem*)data;

            return {
                {"name", item->name},
                {"type", item->type},
                {"enabled", item->enabled},
                {"content", item->content},
            };
        },
        [](QVariantMap map) -> QVariant {
            return QVariant::fromValue(MacroItem{
                .name = map["name"].toString(),
                .type = map["type"].toInt(),
                .enabled = map["enabled"].toBool(),
                .content = map["content"].toString(),
            });
        }
    );
    ConfigurationRegistry::registerStructList<MacroItem>("QList<MacroItem>", "MacroItem");
}
