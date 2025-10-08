// This file is a part of "G-Pilot (formerly Candle)" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich
// Copyright 2024 BTS

#include "gcodeloader.h"
#include "parser/gcodeparser.h"
#include "parser/gcodeviewparser.h"
#include <QDebug>

GCodeLoader::GCodeLoader(QObject *parent)
    : AbstractGCodeLoader(parent)
{
}

void GCodeLoader::loadFromFile(const QString &fileName, GCodeLoaderConfiguration &configuration)
{
    QFile file(fileName);

    if (!file.open(QIODevice::ReadOnly)) {
        emit cancelled();

        //        QMessageBox::critical(this, this->windowTitle(), tr("Can't open file:\n") + fileName);
        return;
    }

    // Set filename
    // m_programFileName = fileName;

    // Prepare text stream
    // QTextStream textStream(&file);
    // textStream.setCodec("UTF-8");

    qDebug() << "Size: " << file.size();

    loadFromFileObject(file, file.size(), configuration);

    file.close();

    // Read lines
    // QList<std::string> data;
    // while (!textStream.atEnd()) {
    //     data.append(textStream.readLine().toStdString());
    // }

    //qDebug() << "Lines: " << count();

    //loadFromString(data);
}

void GCodeLoader::loadFromLines(const QStringList &lines, GCodeLoaderConfiguration &configuration)
{

}

void GCodeLoader::loadFromFileObject(QFile &file, int size, GCodeLoaderConfiguration &configuration)
{
    emit started();

    qDebug() << configuration.arcApproximationValue();
    qDebug() << configuration.arcApproximationMode();

    m_cancel = false;

    // assert(m_communicator->isMachineConfigurationReady());
    // if (!m_communicator->isMachineConfigurationReady()) {
    //     return;
    // }

    // Reset tables
    // clearTable();
    // m_probeModel.clear();
    // m_programHeightmapModel.clear();
    // updateCurrentModel(&m_programModel);

    // Reset parsers
    // m_viewParser.reset();
    // m_probeParser.reset();

    // Reset code drawer
    // m_currentDrawer = m_codeDrawer;
    // m_codeDrawer->update();
    // ui->glwVisualizer->fitDrawable(m_codeDrawer);
    // updateProgramEstimatedTime(QList<LineSegment*>());

    // Update interface
    // ui->chkHeightMapUse->setChecked(false);
    // ui->grpHeightMap->setProperty("overrided", false);
    // style()->unpolish(ui->grpHeightMap);
    // ui->grpHeightMap->ensurePolished();

    // Reset tableview
    // QByteArray headerState = ui->tblProgram->horizontalHeader()->saveState();
    // ui->tblProgram->setModel(NULL);

    // Prepare parser
    // parser.setTraverseSpeed(m_communicator->machineConfiguration().maxRate().x()); // uses only x axis speed
    // if (m_codeDrawer->getIgnoreZ()) parser.reset(QVector3D(qQNaN(), qQNaN(), 0));

    // Block parser updates on table changes
    // m_programLoading = true;

    // Prepare model
    // m_programModel.data().clear();
    // m_programModel.data().reserve(data.count());

    std::string command;
    std::string stripped;
    std::string trimmed;
    QList<QString> args;
    int remaining = size;
    GcodeParser parser;
    GCode* gcode = new GCode();

    while (!file.atEnd()) {
        command = file.readLine().toStdString();

        trimmed = GcodePreprocessorUtils::trimCommand(command);

        if (!trimmed.empty()) {
            // Split command
            stripped = GcodePreprocessorUtils::removeComment(command);
            args = GcodePreprocessorUtils::splitCommand(stripped);

            parser.addCommand(args);

            GCodeItem item;
            item.command = QString::fromStdString(trimmed);
            item.state = GCodeItem::InQueue;
            item.lineNumber = parser.getCommandNumber();
            item.args = args;

            *gcode << item;
        }

        remaining = size - file.pos();

        int percentage = 100 - (remaining * 100 / size);
        static int lastPercentage = 0;
        if (percentage != lastPercentage) {
            lastPercentage = percentage;
            emit progress(percentage);
        }

        // if (progress.isVisible() && (remaining % PROGRESSSTEP == 0)) {
        //     progress.setValue(progress.maximum() - remaining);
        //     qApp->processEvents();
        //     if (progress.wasCanceled()) break;
        // }

        if (m_cancel || QThread::currentThread()->isInterruptionRequested()) {
            emit cancelled();
            return;
        }
    }

    // m_programModel.insertRow(m_programModel.rowCount());

    // updateProgramEstimatedTime(
    GCodeViewParser* viewParser = new GCodeViewParser();
    viewParser->getLinesFromParser(
        &parser,
        configuration.arcApproximationValue(),
        configuration.arcApproximationMode() == ConfigurationParser::ParserArcApproximationMode::ByAngle
    );
    //     );

    // m_programLoading = false;

    // Set table model
    // ui->tblProgram->setModel(&m_programModel);
    // ui->tblProgram->horizontalHeader()->restoreState(headerState);

    // Update tableview
    // connect(ui->tblProgram->selectionModel(), SIGNAL(currentChanged(QModelIndex,QModelIndex)), this, SLOT(onTableCurrentChanged(QModelIndex,QModelIndex)));
    // ui->tblProgram->selectRow(0);

    //  Update code drawer
    // m_codeDrawer->update();
    // ui->glwVisualizer->fitDrawable(m_codeDrawer);

    // resetHeightmap();
    // updateControlsState();

    if (m_cancel) {
        emit cancelled();
    } else {
        emit progress(100);

        GCodeLoaderData *result = new GCodeLoaderData();
        result->gcode = gcode;
        result->viewParser = viewParser;

        emit finished(result);
    }
}

void GCodeLoader::cancel()
{
}
