#ifndef FILESMANAGER_H
#define FILESMANAGER_H

#include <QObject>

class FilesManager : public QObject
{
    Q_OBJECT

    public:
        static FilesManager& instance();

        explicit FilesManager(QObject* parent = nullptr);

        QString gcodeFilePath() const;
        QString gcodeFileName() const;
        void setGcodeFilePath(const QString& filePath);
        void resetGcodeFile();
        bool gcodeOpened() const;
        QString heightmapFilePath() const;
        QString heightmapFileName () const;
        void setHeightmapFilePath(const QString& filePath);
        void resetHeightmapFile();
        bool heightmapOpened() const;

    signals:
        void gcodeFileStateChanged(const bool opened, const QString& filePath);
        void heightmapFileStateChanged(const bool opened, const QString& filePath);

    private:
        QString m_gcodeFilePath;
        bool m_gcodeOpened = false;
        QString m_heightmapFilePath;
        bool m_heightmapOpened = false;
};

#endif // FILESMANAGER_H
