#include "gcodethreadedloader.h"
#include "gcodeloader.h"
#include <QThread>
#include <QObject>

GCodeThreadedLoader::GCodeThreadedLoader(QObject* parent) : AbstractGCodeLoader(parent)
{
}

GCodeThreadedLoader::~GCodeThreadedLoader()
{
    if (m_thread) {
        delete m_thread;
    }
}

void GCodeThreadedLoader::loadFromFile(const QString& fileName, GCodeLoaderConfiguration& configuration)
{
    m_thread = new GCodeLoaderWorker(
        configuration,
        fileName
    );
    connectSignals();
    m_thread->start();

    emit started();
}

void GCodeThreadedLoader::loadFromLines(const QStringList& lines, GCodeLoaderConfiguration& configuration)
{
    m_thread = new GCodeLoaderWorker(
        configuration,
        lines
    );
    connectSignals();
    m_thread->start();

    emit started();
}

void GCodeThreadedLoader::update(GCode* gcode, GCodeLoaderConfiguration& configuration)
{
    m_thread = new GCodeLoaderWorker(
        configuration,
        gcode
    );
    connectSignals();
    m_thread->start();

    emit started();
}

void GCodeThreadedLoader::cancel()
{
    m_thread->requestInterruption();
}

void GCodeThreadedLoader::deleteThread()
{
    m_thread->deleteLater();
    m_thread = nullptr;
}

void GCodeThreadedLoader::connectSignals()
{
    connect(m_thread, &GCodeLoaderWorker::progress, this, [this](int value){
        emit progress(value);
    });
    connect(m_thread, &GCodeLoaderWorker::finished, this, [this](GCodeLoaderData* result){
        emit finished(result);
        deleteThread();
    });
    connect(m_thread, &GCodeLoaderWorker::cancelled, this, [this](){
        emit cancelled();
        deleteThread();
    });
}

GCodeLoaderWorker::GCodeLoaderWorker(GCodeLoaderConfiguration &configuration, const QString &fileName, QObject *parent)
    : QThread(parent)
    , m_configuration(configuration)
{
    this->m_source = Source::File;
    this->m_fileName = fileName;
}

GCodeLoaderWorker::GCodeLoaderWorker(GCodeLoaderConfiguration &configuration, const QStringList &lines, QObject *parent)
    : QThread(parent)
    , m_configuration(configuration)
{
    this->m_source = Source::Lines;
    this->m_lines = lines;
}

GCodeLoaderWorker::GCodeLoaderWorker(GCodeLoaderConfiguration &configuration, const GCode *gcode, QObject *parent)
    : QThread(parent)
    , m_configuration(configuration)
{
    this->m_source = Source::UpdateGCode;
    this->m_gcode = const_cast<GCode*>(gcode);
}

void GCodeLoaderWorker::run() {
    GCodeLoader loader;

    connect(
        &loader, &GCodeLoader::progress, this,
        [this](int value) { emit progress(value); }, Qt::QueuedConnection);
    connect(
        &loader, &GCodeLoader::finished, this,
        [this](GCodeLoaderData *result) {
            emit finished(result);
        },
        Qt::QueuedConnection);
    connect(
        &loader, &GCodeLoader::cancelled, this,
        [this]() { emit cancelled(); }, Qt::QueuedConnection);

    switch (this->m_source) {
        case Source::File:
            loader.loadFromFile(this->m_fileName, m_configuration);
            break;
        case Source::Lines:
            loader.loadFromLines(this->m_lines, m_configuration);
            break;
        case Source::UpdateGCode:
            loader.update(this->m_gcode, m_configuration);
            break;
    }
}
