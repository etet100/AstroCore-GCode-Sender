#ifndef GCODETHREADEDLOADER_H
#define GCODETHREADEDLOADER_H

#include "abstractgcodeloader.h"
#include <QThread>

class GCodeLoaderWorker : public QThread
{
    Q_OBJECT

    public:
        enum class Source {
            File = 1,
            Lines,
            UpdateGCode
        };

        GCodeLoaderWorker(const QString& fileName, QObject* parent = nullptr);
        GCodeLoaderWorker(const QStringList& lines, QObject* parent = nullptr);
        GCodeLoaderWorker(const GCode* gcode, QObject* parent = nullptr);
        ~GCodeLoaderWorker();

        void run();

    signals:
        void progress(int value);
        void finished(GCodeLoaderData* result);
        void cancelled();

    private:
        GCodeLoader *m_loader = nullptr;
        Source m_source;
        QString m_fileName;
        QStringList m_lines;
        GCode* m_gcode = nullptr;
};

class GCodeThreadedLoader : public AbstractGCodeLoader
{
    Q_OBJECT

    public:
        GCodeThreadedLoader(QObject *parent = nullptr);
        ~GCodeThreadedLoader();
        void loadFromFile(const QString &fileName) override;
        std::optional<GCodeLoaderData> loadFromLines(const QStringList &lines) override;
        void update(GCode* gcode) override;
        void cancel() override;

    private:
        GCodeLoaderWorker *m_thread = nullptr;
        void connectSignals();
        void deleteThread();
};

#endif // GCODETHREADEDLOADER_H
