// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#include <QObject>
#include <QRunnable>
#include <QString>
#include <QDateTime>
#include <QList>

struct zzip_dir;

struct ZipArchiveFile
{
    QString relativePath;
    QString cachePath;
    uint compressedSize;
    uint uncompressedSize;
    QDateTime lastModified;
    bool doExtract;
};
typedef QList<ZipArchiveFile> ZipFileList;

/// Worker thread that unpacks zip file contents.
class ZipWorker : public QObject, public QRunnable
{
Q_OBJECT

public:
    ZipWorker(const QString &diskSource, ZipFileList files);
    ~ZipWorker();

    /// QThread override.
    virtual void run();
    
signals:
    /// Emitted when zip processing has been completed.
    /** @note Connect your slot with Qt::QueuedConnection so
        you will receive the callback in your thread. */
    void AsynchLoadCompleted(bool successfull);
    
private:
    void Close();
    
    QString diskSource_;
    ZipFileList files_;
    zzip_dir *archive_;
};
    