#include "gcodethreadedloader.h"
#include "gcodeloader.h"
#include <QThread>
#include <QObject>

GCodeThreadedLoader::GCodeThreadedLoader(QObject* parent) : AbstractGCodeLoader(parent)
{
}

GCodeThreadedLoader::~GCodeThreadedLoader()
{
    deleteThread();
}

void GCodeThreadedLoader::loadFromFile(const QString& fileName, GCodeLoaderConfiguration& configuration)
{
    qDebug() << "[GCodeThreadedLoader] Creating worker thread to load file:" << fileName;
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
    qDebug() << "[GCodeThreadedLoader] Creating worker thread to load from" << lines.size() << "lines";
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
    qDebug() << "[GCodeThreadedLoader] Creating worker thread to update GCode with" << gcode->count() << "items";
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
    if (m_thread) {
        qDebug() << "[GCodeThreadedLoader] Deleting worker, waiting for worker thread to finish";
        m_thread->requestInterruption();
        if (!m_thread->wait(1500)) {
            qWarning() << "[GCodeThreadedLoader] Worker thread did not finish in time, terminating";
            m_thread->terminate();
        }
        delete m_thread;
        m_thread = nullptr;
    } else {
        qDebug() << "[GCodeThreadedLoader] No worker to delete";
    }
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

GCodeLoaderWorker::~GCodeLoaderWorker()
{
    qDebug() << "[GCodeLoader][Worker] Destructor";
}

void GCodeLoaderWorker::run() {
    qDebug() << "[GCodeLoader][Worker] Thread started for source:" << static_cast<int>(m_source);

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

    qDebug() << "[GCodeLoader][Worker] Thread finished";
}
