// This file is a part of "G-Pilot GCode Sender" application.

#ifndef UIPERMISSIONS_H
#define UIPERMISSIONS_H

#include <QHash>
#include <QSet>
#include "core/state_behavior/abstractstatebehavior.h"

enum class UiPermission {
    EditGCode,
    OpenFile,
    CloseFile,
    ChangeSettings,
};

namespace UiPermissions {

using T = AbstractStateBehavior::Type;
using P = UiPermission;
using PSet = QSet<UiPermission>;

static const PSet All = { P::EditGCode, P::OpenFile, P::CloseFile, P::ChangeSettings };

// States not listed here have no restrictions.
static const QHash<T, PSet> forbidden {
    { T::Running,   All },
    { T::Homing,    All },
    { T::ScanTable, All },
    { T::CheckMode, { P::EditGCode, P::OpenFile, P::ChangeSettings } },
    { T::Probing,   { P::EditGCode, P::OpenFile } },
    { T::Jogging,   { P::ChangeSettings } },
};

inline bool isAllowed(UiPermission p, AbstractStateBehavior::Type t)
{
    return !forbidden.value(t).contains(p);
}

} // namespace UiPermissions

#endif // UIPERMISSIONS_H
