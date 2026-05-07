
#include <QRegularExpression>
#include "core/globals.h"
#include "core/communicator/communicator.h"
#include "core/machine/modalstateparser.h"
#include "core/gcode/parser/gcodepreprocessorutils.h"
#include "core/machine/physicalmachineconfigurationparser.h"
#include "statusreportprocessor.h"
#include <QMessageBox>
#include <QThread>
#include <QCoreApplication>

static bool dataIsStartupMessage(const QString& data)
{
    static QRegularExpression re(
        "^(GRBL|GCARVIN)\\s\\d\\.\\d.",
        QRegularExpression::CaseInsensitiveOption
    );

    return re.match(data).hasMatch();
}

void Communicator::onConnectionLineReceived(QString data)
{
    qDebug() << "[Communicator][Resp][All] " << data;

    assert(QThread::currentThread() == QCoreApplication::instance()->thread());
    //The longest line i have seen is 94 characters:
    //"<Run|MPos:-10.780,-9.740,3.000|Bf:0,932|DTG:-20.215,-18.260,0.000|FS:673,1000|WCO:0.000,0.000,0.000>"
    assert(data.length() < 150);

    if (data.startsWith("[MSG:")) {
        qDebug() << "[Communicator][Resp][Msg] " << data;
        processMessage(data);
        processStateBehaviorTransition();

        return;
    }

    if (data.startsWith("ALARM:")) {
        qDebug() << "[Communicator][Resp][Alarm] " << data;

        processAlarm(data);
        processStateBehaviorTransition();

        return;
    }

    // if (m_reseting) {
    //     if (!dataIsStartupMessage(data)) return;
    //     m_reseting = false;
    //     stopUpdatingState();
    //     startUpdatingState(m_configuration->connectionModule().queryStateInterval());
    // }

    if (data.isEmpty()) {
        // blank response
        return;
    }

    // Status response
    if (data.startsWith('<')) {
        qDebug() << "[Communicator][Resp][Status] " << data;
        processStatus(data);
        processStateBehaviorTransition();

        return;
    }

    // qDebug() << "<" << data;

    // if (dataIsStartupMessage(data)) {
    //     qDebug() << "< RST <" << data;
    // }

    qDebug() << "[Communicator][Resp2] " << data;

    if (m_sbManager.hasCurrent()) {
        AbstractStateBehavior* sb = m_sbManager.current();
        if (sb->onRawResponse(data) == AbstractStateBehavior::Result::Ok) {
            processStateBehaviorTransition();

            return;
        }
    }

    if (!m_commandBuffer->isEmpty() && !dataIsStartupMessage(data)
        && !(m_commandBuffer->commands()[0].commandLine != "[CTRL+X]" && dataIsStartupMessage(data)))
    {
        if (m_commandBuffer->processResponse(data)) {
            processStateBehaviorTransition();

            return;
        }
    }

    // Unprocessed responses
    // Handle hardware reset
    processUnhandledResponse(data);
    processStateBehaviorTransition();
}

void Communicator::processStatus(QString line)
{
    //qDebug() << "[Communicator] Processing status line:" << line;

    // MachineState state = MachineState::Unknown;
    // Remove < and >, split by |
    // <Run|MPos:-10.780,-9.740,3.000|Bf:0,932|DTG:-20.215,-18.260,0.000|FS:673,1000|WCO:0.000,0.000,0.000>
    m_statusReceived = true;

    auto report = StatusReportProcessor::parse(line);
    if (!report) {
        return;
    }

    processMachineState(report->state);

    // WCO is sent only occasionally (every few status messages) — fall back to the sticky
    // value cached in PositionTracker. Then derive whichever of MPos/WPos was missing.
    const QVector3D wco = report->hasWorkOffset ? report->workOffset
                                                : m_posTracker->workOffset();

    if (report->hasMachinePos && !report->hasWorkPos) {
        report->workPos = report->machinePos - wco;
        report->hasWorkPos = true;
    } else if (report->hasWorkPos && !report->hasMachinePos) {
        report->machinePos = report->workPos + wco;
        report->hasMachinePos = true;
    }
    if (!report->hasWorkOffset && (report->hasMachinePos || report->hasWorkPos)) {
        report->workOffset = wco;
        report->hasWorkOffset = true;
    }

    if (report->hasMachinePos) {
        m_posTracker->processMachinePosition(report->machinePos);
    }
    if (report->hasWorkOffset) {
        m_posTracker->processWorkOffset(report->workOffset);
    }

    if (report->hasOverrides) {
        emit overridesReceived(report->feedOverride, report->spindleOverride, report->rapidOverride);
    }
    if (report->hasFeedSpindleSpeed) {
        emit feedSpindleSpeedReceived(report->feedRate, report->spindleSpeed);
    }
    if (report->hasBufferStatus) {
        emit buffersStatusReceived(report->bufferAvailable, report->bufferSize);
    }
    if (report->hasPinStates) {
        emit pinStateReceived(report->pinStates);
        qDebug() << "[Communicator] Pin state:" << report->pinStates.toString();
    }
    if (report->hasAccessoryState) {
        m_spindleCW = report->spindleCW;
        emit spindleStateReceived(report->spindleEnabled);
        emit floodStateReceived(report->floodEnabled);
    }

    m_deviceContext.setLastStatusReport(*report);

    emit machineStatusReportReceived(*report);
    emit workPosChanged(m_posTracker->workPos());

    m_posTracker->processNewToolPosition(
        m_deviceContext.machineState() == MachineState::Check,
        true
    );

    // Emit status signal
    emit statusReceived(line);
}

void Communicator::processMachineState(MachineState state)
{
    static MachineState lastState = MachineState::Unknown;
    if (state != lastState) {
        qDebug() << "[Communicator][Status][Change] Machine state changed:"
                 << m_machineStateDictionary.value(state, "Unknown");
        lastState = state;
    }

    qDebug() << "[Communicator][MachineState] Machine state:"
             << m_machineStateDictionary.value(state, "Unknown");

    // Update status
    AbstractStateBehavior* sb = m_sbManager.current();
    if (state != m_deviceContext.machineState()) {
        sb->onMachineStateChanged(state);
        emit machineStateChanged(state);
    }
    sb->onMachineState(state);

    emit machineStateReceived(state);

    // Update controls
    // moved to deviceStateReceived handler
    // ui->cmdCheck->setEnabled(state != DeviceRun && (m_senderState == SenderStopped));
    // ui->cmdCheck->setChecked(state == DeviceCheck);
    // ui->cmdHold->setChecked(state == DeviceHold0 || state == DeviceHold1 || state == DeviceQueue);
    // ui->cmdSpindle->setEnabled(state == DeviceHold0 || ((m_senderState != SenderTransferring) &&
    //                                                     (m_senderState != SenderStopping)));

    // // Update "elapsed time" timer
    // // moved to deviceStateReceived handler
    // if ((m_senderState == SenderTransferring) || (m_senderState == SenderStopping)) {
    //     QTime time(0, 0, 0);
    //     int elapsed = m_startTime.elapsed();
    //     ui->glwVisualizer->setSpendTime(time.addMSecs(elapsed));
    // }

    // Abort
    static double x = sNan;
    static double y = sNan;
    static double z = sNan;

    // if (m_aborting) {
    //     switch (state) {
    //         case MachineState::Idle: // Idle
    //             if ((m_senderState == SenderState::Stopped) && m_resetCompleted) {
    //                 m_aborting = false;
    //                 restoreParserState();
    //                 m_posTracker->restoreOffsets(m_machineConfiguration);
    //                 return;
    //             }
    //             break;
    //         case MachineState::Hold0: // Hold
    //         case MachineState::Hold1:
    //         case MachineState::Queue:
    //             if (!m_reseting && m_posTracker->compareCoordinates(x, y, z)) {
    //                 x = sNan;
    //                 y = sNan;
    //                 z = sNan;
    //                 reset();
    //             } else {
    //                 const QVector3D pos = m_posTracker->machinePos();
    //                 x = pos.x();
    //                 y = pos.y();
    //                 z = pos.z();
    //             }
    //             break;
    //         case MachineState::Unknown:
    //         case MachineState::Alarm:
    //         case MachineState::Run:
    //         case MachineState::Home:
    //         case MachineState::Check:
    //         case MachineState::Door0:
    //         case MachineState::Door1:
    //         case MachineState::Door2:
    //         case MachineState::Door3:
    //         case MachineState::Jog:
    //         case MachineState::Sleep:
    //             break;
    //     }
    // }

}

void Communicator::processDeviceConfiguration(QStringList response)
{
    PhysicalMachineConfigurationParser::setConfiguration(&m_configuration->machineModule());
    m_deviceContext.setPhysicalConfig(PhysicalMachineConfigurationParser::parse(response));

    emit machineConfigurationReceived(m_deviceContext.physicalConfig());


    // static QRegularExpression gs("^\\$(\\d+)\\=([^;]+)$");

    // QMap<int, double> rawMachineConfiguration;

    // for (QString line : response) {
    //     QRegularExpressionMatch match = gs.match(line);
    //     if (match.hasMatch()) {
    //         rawMachineConfiguration[match.captured(1).toInt()] = match.captured(2).toDouble();
    //     }
    // }

    // MachineConfiguration *machineConfiguration = m_machineConfiguration = new MachineConfiguration(
    //     rawMachineConfiguration,
    //     m_configuration->machineModule()
    // );



    // if (commandAttributes.callback != nullptr) {
    //     commandAttributes.callback(machineConfiguration);
    // }


    // Command sent after reset
    // if (ca.tableIndex == -2) {
    //     QList<int> keys = rawMachineConfiguration.keys();
    //     if (keys.contains(13)) m_settings->setUnits(rawMachineConfiguration[13]);
    //     {...}

    //     //moved to settingsReceived signal handler
    //     //setupCoordsTextboxes();
    // }

    qDebug() << "[Communicator] Device configuration processed.";
}

void Communicator::processGCodeParserState(CommandAttributes commandAttributes, QString response)
{
    static QRegularExpression g("G5[4-9]");

    QRegularExpressionMatch match = g.match(response);
    if (match.hasMatch()) {
        m_posTracker->coordinateCache().setCS(match.captured(0));
        // @TODO how to update drawer? signal? timer?
        // m_form->machineBoundsDrawer().setOffset(
        //     QPointF(
        //         toMetric(m_storedVars.x()),
        //         toMetric(m_storedVars.y())
        //     ) + QPointF(
        //         toMetric(m_storedVars.G92x()),
        //         toMetric(m_storedVars.G92y()
        //     )
        // ));
    }

    static QRegularExpression t("T(\\d+)(?!\\d)");

    match = t.match(response);
    if (match.hasMatch()) {
        m_posTracker->coordinateCache().setTool(match.captured(1).toInt());
    }

    // TODO: Store firmware version, features, buffer size on $I command
    // [VER:1.1d.20161014:Some string]
    // [OPT:VL,15,128]

    // Restore absolute/relative coordinate system after jog
    // if (commandAttributes.tableIndex == TABLE_INDEX_UTIL1) {
    //     // @TODO how to handle keyboard control?? not like this!
    //     // if (ui->chkKeyboardControl->isChecked()) m_form->absoluteCoordinates() = response.contains("G90");
    //     // else if (response.contains("G90")) sendCommand(CommandSource::System, "G90", COMMAND_TI_UI);
    //     // if (response.contains("G90")) sendCommand(CommandSource::System, "G90", TABLE_INDEX_UI);
    // }

    // Process GCore parser state
    // if (commandAttributes.tableIndex == TABLE_INDEX_UTIL2) {
        auto modal = ModalStateParser::parse(response);
        if (modal) {
            m_deviceContext.setModalState(*modal);
            emit log(modal->toString());

            emit modalStateChanged(*modal);

            // Update status in visualizer window
            emit parserStateReceived(modal->raw);

            if (modal->spindleSpeed != -1) {
                emit spindleSpeedReceived(modal->spindleSpeed);
            }

            // Spindle speed
            // @TODO what is the difference between this and processFeedSpindleSpeed??
            // static QRegularExpression rx(".*S([\\d\\.]+)");

            // match = rx.match(response);
            // if (match.hasMatch()) {
            //     double spindleSpeed = match.captured(1).toDouble();
            //     emit spindleSpeedReceived(spindleSpeed);
            // }
        }

        m_updateParserState = true;
    // }
}

// processCommandResponse was moved to CommandBuffer::processResponse()

#if 0
bool Communicator::processCommandResponse(QString data)
{
    bool result = false;

    static QString response;
    static QStringList lines;

    // qDebug() << "< CMD <" << data;

    assert(m_commands.length() > 0);

    // This part is used to collect multi-line responses, e.g. $$
    // Was opposite: if ((m_commands[0].commandLine != "[CTRL+X]" && dataIsEnd(data)) || (m_commands[0].commandLine == "[CTRL+X]" && dataIsStartupMessage(data))) {
    QString firstCommand = m_commands[0].commandLine;
    if ((firstCommand == "[CTRL+X]" || !dataIsEnd(data)) && (firstCommand != "[CTRL+X]" || !dataIsStartupMessage(data))) {
        response.append(data + "; ");
        lines.append(data);

        return false;;
    }

    response.append(data);
    // lines.append(data);

    CmdStatus cmdStatus{.ok = false, .errorCode = 0};
    if (data == "ok" || data == ">:ok") {
        cmdStatus.ok = true;
    } else if (data.startsWith("error:")) {
        cmdStatus.errorCode = data.mid(6).toInt();
        qDebug() << "[Communicator] error" << cmdStatus.errorCode;
    } else if (data.startsWith(">:error:")) {
        cmdStatus.errorCode = data.mid(8).toInt();
        qDebug() << "[Communicator] error" << cmdStatus.errorCode;
    } else {
        qDebug() << "[Communicator] unknown response status" << data;
        assert(false);
    }

    // Take command from buffer
    CommandAttributes commandAttributes = m_commands.takeFirst();
    // qDebug() << "taking command, new size " << m_commands.length() << ", command is " << commandAttributes.commandLine;

    QString command = GcodePreprocessorUtils::removeComment(commandAttributes.commandLine).toUpper();

    if (m_sbManager.hasCurrent()) {
        AbstractStateBehavior* sb = m_sbManager.current();
        AbstractStateBehavior::Result sbResult = sb->onCommandResponse(command, commandAttributes, cmdStatus, data, lines);
        switch (sbResult) {
            case AbstractStateBehavior::Result::Ok:
                result = true;
                break;

            case AbstractStateBehavior::Result::ReturnCommandToQueue:
                // return command to the queue
                qDebug() << "[Communicator] Returning command to queue:" << commandAttributes.commandLine;
                m_commands.prepend(commandAttributes);
                break;
        }
    }

    // Store current coordinate system
    // if (command == "$G") {
    //     processGCodeParserState(commandAttributes, response);
    // }

    // // Offsets
    if (command == "$#" && cmdStatus.ok) {
        processOffsetsVars(lines);
    }

    // // Settings response
    // if (command == "$$") {
    //     processDeviceConfiguration(response, commandAttributes);
    // }

    // Homing response
    // if ((command == "$H" || command == "$T") && m_homing) m_homing = false;

    // Reset complete response
    // if (command == "[CTRL+X]") {
    //     m_resetCompleted = true;
    //     m_updateParserState = true;

    //     // Query grbl settings
    //     sendCommand(CommandSource::System, "$$", TABLE_INDEX_UTIL1);
    //     sendCommand(CommandSource::System, "$#", TABLE_INDEX_UTIL1, true);
    // }

    // Clear command buffer on "M2" & "M30" command (old firmwares)
    // static QRegularExpression M230("(M0*2|M30)(?!\\d)");
    // if (command.contains(M230) && response.contains("ok") && !response.contains("Pgm End")) {
    //     m_commands.clear();
    //     m_queue.clear();

    //     emit aborted();
    // }

    // Update probe coords on user commands
    // if (command.contains("G38.2") && commandAttributes.tableIndex < 0) {
    //     static QRegularExpression PRB(".*PRB:([^,]*),([^,]*),([^,:]*)");

    //     QRegularExpressionMatch match = PRB.match(response);
    //     if (match.hasMatch()) {
    //         m_storedVars.setCoords("PRB", QVector3D(
    //             match.captured(1).toDouble(),
    //             match.captured(2).toDouble(),
    //             match.captured(3).toDouble()
    //         ));
    //     }
    // }

    // Process probing on heightmap mode only from table commands
    // @TODO refactor height map mode
    // if (uncomment.contains("G38.2") && m_form->heightMapMode() && commandAttributes.tableIndex > -1) {
    //     // Get probe Z coordinate
    //     // "[PRB:0.000,0.000,0.000:0];ok"
    //     // "[PRB:0.000,0.000,0.000,0.000:0];ok"
    //     QRegularExpression rx(".*PRB:([^,]*),([^,]*),([^,:]*)");
    //     double z = qQNaN();
    //     if (rx.indexIn(response) != -1) {
    //         z = toMetric(rx.cap(3).toDouble());
    //     }

    //     static double firstZ;
    //     if (m_probeIndex == -1) {
    //         firstZ = z;
    //         z = 0;
    //     } else {
    //         // Calculate delta Z
    //         z -= firstZ;

    //         // Calculate table indexes
    //         int row = (m_probeIndex / m_form->heightMapModel().columnCount());
    //         int column = m_probeIndex - row * m_form->heightMapModel().columnCount();
    //         if (row % 2) column = m_form->heightMapModel().columnCount() - 1 - column;

    //         // Store Z in table
    //         m_form->heightMapModel().setData(m_form->heightMapModel().index(row, column), z, Qt::UserRole);
    //         ui->tblHeightMap->update(m_form->heightMapModel().index(m_form->heightMapModel().rowCount() - 1 - row, column));
    //         m_form->updateHeightMapInterpolationDrawer();
    //     }

    //     m_probeIndex++;
    // }

    // Change state query time on check mode on
    // if (command.contains(QRegularExpression("$[cC]"))) {
    //     m_timerStateQuery.setInterval(response.contains("Enable") ? 1000 : m_configuration->connectionModule().queryStateInterval());
    // }

    // emit signal, was `Add response to console`
    commandAttributes.response = response;
    emit commandResponseReceived(commandAttributes);
    // if (tb.isValid() && tb.text() == ca.command) {
        //emit commandResponseReceived(ca, response);

        // bool scrolledDown = ui->txtConsole->verticalScrollBar()->value()
        //                     == ui->txtConsole->verticalScrollBar()->maximum();

        // removed appending to console
    // }

    // Check queue, try to send queued commands
    static bool processingQueue = false;
    if (m_queue.length() > 0 && !processingQueue) {
        processingQueue = true;
        while (m_queue.length() > 0) {
            CommandQueue command = m_queue.takeFirst();
            SendCommandResult r = sendCommand(command.source, command.commandLine, command.tableIndex, false, command.callback);
            if (r == SendCommandResult::Status::Done) {
                break;
            } else if (r == SendCommandResult::Status::Queue) {
                m_queue.prepend(m_queue.takeLast());
                break;
            }
        }
        processingQueue = false;
    }

//     // Add response to table, send next program commands
//     if (m_senderState != SenderState::Stopped) {
//         // Only if command from table
//         if (commandAttributes.tableIndex > -1) {
//             m_streamer->resetProcessed(commandAttributes.tableIndex);

//             emit commandProcessed(commandAttributes.tableIndex, response);
//         }

// // Update taskbar progress
// #ifdef WINDOWS
//         // @TODO move progress to streamer??
//         // if (QSysInfo::windowsVersion() >= QSysInfo::WV_WINDOWS7) {
//         //     if (m_taskBarProgress) m_taskBarProgress->setValue(m_fileProcessedCommandIndex);
//         // }
// #endif
//         // Process error messages
//         static bool holding = false;
//         static QString errors;

//         if (
//             commandAttributes.tableIndex > -1 &&
//             response.toUpper().contains("ERROR") &&
//             !m_configuration->senderModule().ignoreErrorResponses()
//         ) {
//             errors.append(QString::number(commandAttributes.tableIndex + 1) + ": " + commandAttributes.commandLine + " < " + response + "\n");

//             // @TODO move to UI
//             // m_form->senderErrorBox().setText(tr("Error message(s) received:\n") + errors);

//             if (!holding) {
//                 holding = true;         // Hold transmit while messagebox is visible
//                 response.clear();

//                 sendRealtimeCommand(GRBL_LIVE_FEED_HOLD);
//                 // @TODO move to UI
//                 // m_form->senderErrorBox().checkBox()->setChecked(false);
//                 // qApp->beep();
//                 // int result = m_form->senderErrorBox().exec();
//                 int result = QMessageBox::Ignore;

//                 holding = false;
//                 errors.clear();
//                 // @TODO move to UI
//                 // if (m_form->senderErrorBox().checkBox()->isChecked()) m_settings->setIgnoreErrors(true);
//                 if (result == QMessageBox::Ignore) {
//                     sendRealtimeCommand(GRBL_LIVE_CYCLE_START);
//                 } else {
//                     sendRealtimeCommand(GRBL_LIVE_CYCLE_START);

//                     emit aborted();
//                 }
//             }
//         }

//         // Check transfer complete (last row always blank, last command row = rowcount - 2)
// //        if ((m_streamer->processedCommandIndex() == m_form->currentModel().rowCount() - 2) || uncomment.contains(QRegularExpression("(M0*2|M30)(?!\\d)"))) {
//         if (m_streamer->isLastCommandProcessed() || command.contains(QRegularExpression("(M0*2|M30)(?!\\d)"))) {
//             if (m_deviceState == DeviceState::Run) {
//                 setSenderStateAndEmitSignal(SenderState::Stopping);
//             } else {
//                 completeTransfer();
//             }
//         } else if (m_streamer->hasMoreCommands()  // /*(m_streamer->commandIndex() < m_form->currentModel().rowCount())
//                    && (m_senderState == SenderState::Transferring)
//                    && !holding)
//         {
//             // Send next program commands
//             sendStreamerCommandsUntilBufferIsFull();
//             //m_form->sendNextFileCommands();
//         }
//     }

    // // Tool change mode
    // static QRegularExpression M6("(M0*6)(?!\\d)");
    // if ((m_senderState == SenderState::Pausing) && command.contains(M6)) {
    //     response.clear();

    //     if (m_configuration->senderModule(). pauseSenderOnToolChange()) {
    //         // QMessageBox::information(this, qApp->applicationDisplayName(),
    //         //                          tr("Change tool and press 'Pause' button to continue job"));
    //     }

    //     if (m_configuration->senderModule().useToolChangeCommands()) {
    //         if (m_configuration->senderModule().confirmToolChangeCommandsExecution()) {
    //             // QMessageBox box(this);
    //             // box.setIcon(QMessageBox::Information);
    //             // box.setText(tr("M6 command detected. Send tool change commands?\n"));
    //             // box.setWindowTitle(qApp->applicationDisplayName());
    //             // box.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    //             // box.setCheckBox(new QCheckBox(tr("Don't show again")));
    //             // int res = box.exec();
    //             // if (box.checkBox()->isChecked()) m_settings->setToolChangeUseCommandsConfirm(false);
    //             // if (res == QMessageBox::Yes) {
    //             //     sendCommands(m_settings->toolChangeCommands());
    //             // }
    //         } else {
    //             sendCommands(CommandSource::ProgramAdditionalCommands, m_configuration->senderModule().toolChangeCommands());
    //         }
    //     }

    //     setSenderStateAndEmitSignal(SenderState::ChangingTool);
    // }

    // // Pausing on button?
    // if ((m_senderState == SenderState::Pausing) && !command.contains(M6)) {
    //     if (m_configuration->senderModule().usePauseCommands()) {
    //         sendCommands(CommandSource::ProgramAdditionalCommands, m_configuration->senderModule().beforePauseCommands());
    //         setSenderStateAndEmitSignal(SenderState::Pausing2);
    //     }
    // }
    // if ((m_senderState == SenderState::ChangingTool) && !m_configuration->senderModule().pauseSenderOnToolChange()
    //     && m_commands.isEmpty())
    // {
    //     setSenderStateAndEmitSignal(SenderState::Transferring);
    // }

    // // Switch to pause mode
    // if (isSenderState(SenderState::Pausing, SenderState::Pausing2)) {
    //     if (m_commands.isEmpty()) {
    //         setSenderStateAndEmitSignal(SenderState::Paused);
    //     }
    // }

    // // Same as M2, Program End, turn off spindle/laser and stops the machine.
    // // Scroll to first line on "M30" command
    // if (command.contains("M30")) {
    //     // @TODO new signal here?
    //     emit commandProcessed(-1, "");
    // }

    // Toolpath shadowing on check mode - moved to responseReceived signal handler
    // if (m_deviceState == DeviceCheck) {
    //     GcodeViewParse *parser = m_form->currentDrawer().viewParser();
    //     QList<LineSegment*> list = parser->getLineSegmentList();
    //     {...}
    // }

    // @TODO is it necessary?? does it duplicate the commandResponseReceived signal???
    // Emit response signal
    emit responseReceived(commandAttributes.commandLine, commandAttributes.tableIndex, response);

    response.clear();
    lines.clear();

    return result;
}
#endif

void Communicator::processUnhandledResponse(QString data)
{
    if (dataIsStartupMessage(data)) {
        // Welcome message, hardware reset occurred?
        this->processWelcomeMessageDetected(data);
    }

    if (data.startsWith("error")) {
        QString errorMessage;

        if (data == "error") {
            errorMessage = "Error";
        } else if (data.startsWith("error:")) {
            bool ok = false;
            int code = data.mid(6).toInt(&ok);
            // if (ok && AbstractStateBehavior::ERRORS.contains(code)) {
            //     errorMessage = QString("Error: %1 (%2)").arg(code).arg(AbstractStateBehavior::ERRORS.value(code));
            // } else
            if (ok) {
                errorMessage = QString("Error: %1").arg(code);
            } else {
                errorMessage = data;
            }
        } else {
            errorMessage = data;
        }

        qWarning() << "[Communicator][Error]" << errorMessage;
        emit log(errorMessage);
    }
}

void Communicator::processWelcomeMessageDetected(QString message)
{
    return;
    emit welcomeMessageReceived(message);

    // m_streamer->reset();
    //m_form->fileCommandIndex() = 0;

    m_reseting = false;
    m_homing = false;

    m_updateParserState = true;
    m_statusReceived = true;

    clearCommandsAndQueue();

    // sendCommand(CommandSource::System, "$$", TABLE_INDEX_UTIL1);
    // sendCommand(CommandSource::System, "$#", TABLE_INDEX_UTIL1, true);

}

void Communicator::processMessage(QString data)
{
    static QRegularExpression re("^\\[MSG:(.+)\\]$");
    QRegularExpressionMatch match = re.match(data);

    QString message = match.hasMatch() ? match.captured(1) : data;

    qDebug() << "[Communicator][Msg]" << message;
    emit log("Msg: " + message);
}

void Communicator::processAlarm(QString data)
{
    static QRegularExpression re("^(\\[)?ALARM:(\\d+)(\\])?$");
    QRegularExpressionMatch match = re.match(data);
    if (match.hasMatch()) {
        m_lastAlarmCode = match.captured(2).toInt();

        emit alarm(m_lastAlarmCode);

        if (m_sbManager.hasCurrent()) {
            m_sbManager.current()->onAlarm(m_lastAlarmCode);
        }
    }
}

// Save offset to be used when calculating work coordinates
void Communicator::processOffsetsVars(QStringList response)
{
    m_posTracker->processOffsetsVars(response);

    emit coordinatesCacheChanged();
}
