// This file is a part of "G-Pilot GCode Sender" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich
// Copyright 2024 BTS

#ifndef BEHAVIORS_H
#define BEHAVIORS_H

// Include base class
#include "statebehavior.h"

// Behavior implementations
#include "idlebehavior.h"
#include "initializationbehavior.h"
#include "connectingbehavior.h"
#include "reconnectingbehavior.h"
#include "runningbehavior.h"
#include "joggingbehavior.h"
#include "homingbehavior.h"
#include "errorbehavior.h"
#include "probingbehavior.h"
#include "pausebehavior.h"
#include "alarmbehavior.h"
#include "checkmodebehavior.h"
#include "holdbehavior.h"
#include "toolchangebehavior.h"
#include "resetbehavior.h"
#include "gotobehavior.h"

#endif // BEHAVIORS_H
