/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   QtUtils.cpp
 *  @brief  Cross-platform utility functions using Qt.
 */

#include "StableHeaders.h"
#include "DebugOperatorNew.h"
#include "QtUtils.h"

#include <QString>
#include <QFileDialog>
#include <QDir>
#include <QFileInfo>
#include <QCloseEvent>
#include <QGraphicsProxyWidget>

#include "MemoryLeakCheck.h"

QStringList DirectorySearch(const QString &path, QDir::Filters filters)
{
    QStringList ret;
    if (path.trimmed().isEmpty())
        return ret; // QFileInfo behavior for empty string is undefined
    foreach(const QFileInfo &info, QDir(path).entryInfoList(filters))
        ret.append(info.filePath());
    return ret;
}

QStringList RecursiveDirectorySearch(const QString &path, QDir::Filters filters)
{
    QStringList ret;
    if (path.trimmed().isEmpty())
        return ret; // QFileInfo behavior for empty string is undefined
    foreach(const QFileInfo &info, QDir(path).entryInfoList(filters))
    {
        QString subdir = info.filePath();
        ret.append(subdir);
        ret.append(RecursiveDirectorySearch(subdir, filters));
    }
    return ret;
}

class CustomFileDialog : public QFileDialog
{
public:
    CustomFileDialog(QWidget* parent, const QString& caption, const QString& dir, const QString& filter) :
        QFileDialog(parent, caption, dir, filter)
    {
    }

protected:
    virtual void hideEvent(QHideEvent* e)
    {
        if (e->type() == QEvent::Hide)
        {
            emit finished(0);
            deleteLater();
        }
    }
    virtual void closeEvent(QCloseEvent* e)
    {
        if (e->type() == QEvent::Hide)
        {
            emit finished(0);
        }
    }
};

namespace QtUtils
{
    QFileDialog* OpenFileDialogNonModal(
        const QString& filter,
        const QString& caption,
        const QString& dir,
        QWidget* parent,
        QObject* initiator,
        const char* slot,
        bool multipleFiles)
    {
        QFileDialog* dialog = new CustomFileDialog(parent, caption, dir, filter);
        dialog->setAttribute(Qt::WA_DeleteOnClose);
        if (multipleFiles)
            dialog->setFileMode(QFileDialog::ExistingFiles);
        QObject::connect(dialog, SIGNAL(finished(int)), initiator, slot);
        dialog->show();
        dialog->resize(500, 300);

        if (dialog->graphicsProxyWidget())
            dialog->graphicsProxyWidget()->setWindowTitle(caption);

        return dialog;
    }

    QFileDialog *SaveFileDialogNonModal(
        const QString& filter,
        const QString& caption,
        const QString& dir,
        QWidget* parent,
        QObject* initiator,
        const char* slot)
    {
        QFileDialog* dialog = new CustomFileDialog(parent, caption, dir, filter);
        dialog->setFileMode(QFileDialog::AnyFile);
        dialog->setAcceptMode(QFileDialog::AcceptSave);
        dialog->setAttribute(Qt::WA_DeleteOnClose);
        QObject::connect(dialog, SIGNAL(finished(int)), initiator, slot);
        dialog->show();
        dialog->resize(500, 300);

        if (dialog->graphicsProxyWidget())
            dialog->graphicsProxyWidget()->setWindowTitle(caption);

        return dialog;
    }

    QFileDialog *DirectoryDialogNonModal(const QString &caption, const QString &dir, QWidget *parent, QObject* initiator, const char* slot)
    {
        QFileDialog* dialog = new QFileDialog(parent, caption, dir, "");
        dialog->setFileMode(QFileDialog::Directory);
        dialog->setOption(QFileDialog::ShowDirsOnly, true);
        dialog->setAttribute(Qt::WA_DeleteOnClose);
        QObject::connect(dialog, SIGNAL(finished(int)), initiator, slot);
        dialog->show();
        dialog->resize(500, 300);

        if (dialog->graphicsProxyWidget())
            dialog->graphicsProxyWidget()->setWindowTitle(caption);

        return dialog;
    }
}
