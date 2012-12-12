/**
    For conditions of distribution and use, see copyright notice in LICENSE

    @file   FileUtils.cpp
    @brief  Utility file dialog functions. */

#include "StableHeaders.h"
#include "DebugOperatorNew.h"
#include "FileUtils.h"

#include <QString>
#include <QFileDialog>
#include <QDirIterator>
#include <QCloseEvent>
#include <QGraphicsProxyWidget>

#include "MemoryLeakCheck.h"

QStringList DirectorySearch(const QString &path, bool recursive, QDir::Filters filters)
{
    QStringList ret;
    QDirIterator it(path, filters, recursive ? QDirIterator::Subdirectories : QDirIterator::NoIteratorFlags);
    while(it.hasNext())
        ret.append(it.next());
    return ret;
}

/// @cond PRIVATE
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
/// @endcond

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

#ifdef __APPLE__
    dialog->setOption(QFileDialog::DontUseNativeDialog, true);
#endif

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
