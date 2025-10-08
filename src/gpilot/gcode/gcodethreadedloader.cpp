#include "gcodethreadedloader.h"
#include "gcodeloader.h"
#include <QThread>
#include <QObject>

GCodeThreadedLoader::GCodeThreadedLoader(QObject* parent) : AbstractGCodeLoader(parent)
{
}

void GCodeThreadedLoader::loadFromFile(const QString& fileName, GCodeLoaderConfiguration& configuration)
{
    GCodeLoaderWorker *m_thread = new GCodeLoaderWorker(
        configuration,
        GCodeLoaderWorker::Source::File,
        fileName,
        QStringList()
    );

    connect(m_thread, &GCodeLoaderWorker::progress, this, [this](int value){
        emit progress(value);
    });
    connect(m_thread, &GCodeLoaderWorker::finished, this, [this](GCodeLoaderData* result){
        emit finished(result);
    });
    connect(m_thread, &GCodeLoaderWorker::cancelled, this, [this](){
        emit cancelled();
    });

    m_thread->start();

    emit started();
}

void GCodeThreadedLoader::loadFromLines(const QStringList& lines, GCodeLoaderConfiguration& configuration)
{
    GCodeLoaderWorker *m_thread = new GCodeLoaderWorker(
        configuration,
        GCodeLoaderWorker::Source::Lines,
        "",
        lines
    );

    m_thread->run();

    emit started();
}

void GCodeThreadedLoader::cancel()
{
    m_thread->requestInterruption();
}

GCodeLoaderWorker::GCodeLoaderWorker(GCodeLoaderConfiguration& configuration, Source source, const QString &fileName, const QStringList &lines, QObject *parent)
    : QThread(parent)
    , m_configuration(configuration)
{
    this->m_source = source;
    this->m_fileName = fileName;
    this->m_lines = lines;
}

void GCodeLoaderWorker::run() {
    GCodeLoader loader;

    connect(
        &loader, &GCodeLoader::progress, this,
        [this](int value) { emit progress(value); }, Qt::QueuedConnection);
    connect(
        &loader, &GCodeLoader::finished, this,
        [this](GCodeLoaderData *result) { emit finished(result); },
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
    }
}
