// This file is a part of "G-Pilot (formerly Candle)" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich
// Copyright 2025 BTS

#ifndef TOOLCHANGEBEHAVIOR_H
#define TOOLCHANGEBEHAVIOR_H

#include "statebehavior.h"

class ToolChangeBehavior : public StateBehavior
{
    public:
        explicit ToolChangeBehavior(QObject *parent);
        QString name() override { return "Tool Change"; }

        enum ToolChangeSource {
            Program,
            User
        };
};

#endif // TOOLCHANGEBEHAVIOR_H
