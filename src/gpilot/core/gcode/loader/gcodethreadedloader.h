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
            Lines,
            UpdateGCode
        };

        GCodeLoaderWorker(GCodeLoaderConfiguration& configuration, const QString& fileName, QObject* parent = nullptr);
        GCodeLoaderWorker(GCodeLoaderConfiguration& configuration, const QStringList& lines, QObject* parent = nullptr);
        GCodeLoaderWorker(GCodeLoaderConfiguration& configuration, const GCode* gcode, QObject* parent = nullptr);

        void run();

    signals:
        void progress(int value);
        void finished(GCodeLoaderData* result);
        void cancelled();

    private:
        GCodeLoaderConfiguration m_configuration;
        GCodeLoader *m_loader;
        Source m_source;
        QString m_fileName;
        QStringList m_lines;
        GCode* m_gcode;
};

class GCodeThreadedLoader : public AbstractGCodeLoader
{
    Q_OBJECT

    public:
        GCodeThreadedLoader(QObject *parent = nullptr);
        ~GCodeThreadedLoader();
        void loadFromFile(const QString &fileName, GCodeLoaderConfiguration &configuration) override;
        void loadFromLines(const QStringList &lines, GCodeLoaderConfiguration &configuration) override;
        void update(GCode* gcode, GCodeLoaderConfiguration &configuration) override;
        void cancel() override;

    private:
        GCodeLoaderWorker *m_thread;
        void connectSignals();
        void deleteThread();
};

#endif // GCODETHREADEDLOADER_H
