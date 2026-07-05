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
    m_gcodeModified = false;

    emit gcodeFileStateChanged(m_gcodeOpened, filePath, m_gcodeModified);
}

void FilesManager::resetGcodeFile()
{
    m_gcodeFilePath.clear();
    m_gcodeOpened = false;
    m_gcodeModified = false;

    emit gcodeFileStateChanged(false, "", false);
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
    m_heightmapModified = false;

    emit heightmapFileStateChanged(m_heightmapOpened, filePath, m_heightmapModified);
}

void FilesManager::resetHeightmapFile()
{
    m_heightmapFilePath.clear();
    m_heightmapOpened = false;
    m_heightmapModified = false;

    emit heightmapFileStateChanged(false, "", false);
}

bool FilesManager::heightmapOpened() const
{
    return m_heightmapOpened;
}

void FilesManager::setGcodeModified(const bool modified)
{
    m_gcodeModified = modified;

    emit gcodeFileStateChanged(m_gcodeOpened, m_gcodeFilePath, modified);
}

bool FilesManager::gcodeModified() const
{
    return m_gcodeModified;
}

void FilesManager::setHeightmapModified(const bool modified)
{
    m_heightmapModified = modified;

    emit heightmapFileStateChanged(m_heightmapOpened, m_heightmapFilePath, modified);
}

bool FilesManager::heightmapModified() const
{
    return m_heightmapModified;
}
