#include "gcodethreadedloader.h"
#include "abstractgcodeloader.h"
#include <QThread>
#include <QObject>

GCodeThreadedLoader::GCodeThreadedLoader(QObject* parent) : AbstractGCodeLoader(parent)
{
}

GCodeThreadedLoader::~GCodeThreadedLoader()
{
    deleteThread();
}

void GCodeThreadedLoader::loadFromFile(const QString& fileName)
{
    qDebug() << "[GCodeThreadedLoader] Creating worker thread to load file:" << fileName;
    m_thread = new GCodeLoaderWorker(fileName);
    connectSignals();
    m_thread->start();

    emit started();
}

std::optional<GCodeLoaderData> GCodeThreadedLoader::loadFromLines(const QStringList& lines)
{
    qDebug() << "[GCodeThreadedLoader] Creating worker thread to load from" << lines.size() << "lines";
    m_thread = new GCodeLoaderWorker(lines);
    connectSignals();
    m_thread->start();

    emit started();

    return std::nullopt;
}

void GCodeThreadedLoader::update(GCode* gcode)
{
    qDebug() << "[GCodeThreadedLoader] Creating worker thread to update GCode with" << gcode->count() << "items";
    m_thread = new GCodeLoaderWorker(gcode);
    connectSignals();
    m_thread->start();

    emit started();
}

void GCodeThreadedLoader::cancel()
{
    if (m_thread) {
        m_thread->requestInterruption();
    }
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

GCodeLoaderWorker::GCodeLoaderWorker(const QString &fileName, QObject *parent)
    : QThread(parent)
{
    this->m_source = Source::File;
    this->m_fileName = fileName;
}

GCodeLoaderWorker::GCodeLoaderWorker(const QStringList &lines, QObject *parent)
    : QThread(parent)
{
    this->m_source = Source::Lines;
    this->m_lines = lines;
}

GCodeLoaderWorker::GCodeLoaderWorker(const GCode *gcode, QObject *parent)
    : QThread(parent)
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
            loader.loadFromFile(this->m_fileName);
            break;
        case Source::Lines:
            loader.loadFromLines(this->m_lines);
            break;
        case Source::UpdateGCode:
            loader.update(this->m_gcode);
            break;
    }

    qDebug() << "[GCodeLoader][Worker] Thread finished";
}
