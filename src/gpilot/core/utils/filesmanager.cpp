#include "filesmanager.h"
#include <QFileInfo>

FilesManager &FilesManager::instance()
{
    static FilesManager instance;

    return instance;
}

FilesManager::FilesManager(QObject* parent)
    : QObject{parent}
{}

QString FilesManager::gcodeFilePath() const
{
    return m_gcodeFilePath;
}

QString FilesManager::gcodeFileName() const
{
    return QFileInfo(m_gcodeFilePath).fileName();
}

void FilesManager::setGcodeFilePath(const QString &filePath)
{
    m_gcodeFilePath = filePath;
    m_gcodeOpened = true;

    emit gcodeFileStateChanged(m_gcodeOpened, filePath);
}

void FilesManager::resetGcodeFile()
{
    m_gcodeFilePath.clear();
    m_gcodeOpened = false;

    emit gcodeFileStateChanged(false, "");
}

bool FilesManager::gcodeOpened() const
{
    return m_gcodeOpened;
}

QString FilesManager::heightmapFilePath() const
{
    return m_heightmapFilePath;
}

QString FilesManager::heightmapFileName() const
{
    return QFileInfo(m_heightmapFilePath).fileName();
}

void FilesManager::setHeightmapFilePath(const QString &filePath)
{
    m_heightmapFilePath = filePath;
    m_heightmapOpened = true;

    emit heightmapFileStateChanged(m_heightmapOpened, filePath);
}

void FilesManager::resetHeightmapFile()
{
    m_heightmapFilePath.clear();
    m_heightmapOpened = false;

    emit heightmapFileStateChanged(false, "");
}

bool FilesManager::heightmapOpened() const
{
    return m_heightmapOpened;
}

void FilesManager::setGcodeModified(const bool modified)
{
    m_gcodeModified = modified; }

bool FilesManager::gcodeModified() const
{
    return m_gcodeModified;
}

void FilesManager::setHeightmapModified(const bool modified)
{
    m_heightmapModified = modified;
}

bool FilesManager::heightmapModified() const
{
    return m_heightmapModified;
}
