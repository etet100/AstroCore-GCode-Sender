#ifndef GCODETHREADEDLOADER_H
#define GCODETHREADEDLOADER_H

#include "gcodeloader.h"
#include <QThread>

class GCodeLoaderWorker : public QThread
{
    Q_OBJECT

    public:
        enum class Source {
            File,
            Lines
        };

        GCodeLoaderWorker(GCodeLoaderConfiguration& configuration, Source source, const QString &fileName, const QStringList &lines, QObject *parent = nullptr);

        void run();

    signals:
        void progress(int value);
        void finished(GCodeLoaderData *result);
        void cancelled();

    private:
        GCodeLoaderConfiguration m_configuration;
        GCodeLoader *m_loader;
        Source m_source;
        QString m_fileName;
        QStringList m_lines;
};

class GCodeThreadedLoader : public AbstractGCodeLoader
{
    Q_OBJECT

    public:
        GCodeThreadedLoader(QObject *parent = nullptr);
        void loadFromFile(const QString &fileName, GCodeLoaderConfiguration &configuration) override;
        void loadFromLines(const QStringList &lines, GCodeLoaderConfiguration &configuration) override;
        void cancel() override;

    private:
        GCodeLoaderWorker *m_thread;
};

#endif // GCODETHREADEDLOADER_H
