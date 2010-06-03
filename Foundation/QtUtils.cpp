// For conditions of distribution and use, see copyright notice in license.txt

/// @file QtUtils.cpp
/// @brief Cross-platform utility functions using Qt.

#include "StableHeaders.h"
#include "QtUtils.h"

#include <QString>
#include <QFileDialog>
#include <QDir>
#include <QCloseEvent>

namespace Foundation
{

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

QFileDialog* QtUtils::OpenFileDialogNonModal(
    const std::string& filter,
    const std::string& caption,
    const std::string& dir,
    QWidget* parent,
    QObject* initiator,
    const char* slot)
{
    QFileDialog* dialog = new CustomFileDialog(parent, QString::fromStdString(caption), QString::fromStdString(dir), 
        QString::fromStdString(filter));
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    QObject::connect(dialog, SIGNAL(finished(int)), initiator, slot);
    dialog->show();
    dialog->resize(500, 300);
    return dialog;
}

std::string QtUtils::GetOpenFileName(
    const std::string &filter,
    const std::string &caption,
    const std::string &dir)
{
    QString qcaption(caption.c_str());
    QString qdir(dir.c_str());
    QString qfilter(filter.c_str());
    QWidget *parent = 0;

    QString filename = QFileDialog::getOpenFileName(parent, qcaption, qdir, qfilter);

    return filename.toStdString();
}

StringList QtUtils::GetOpenFileNames(
    const std::string &filter,
    const std::string &caption,
    const std::string &dir)
{
    QString qcaption(caption.c_str());
    QString qdir(dir.c_str());
    QString qfilter(filter.c_str());
    QWidget *parent = 0;
    StringList filelist;

    QStringList filenames = QFileDialog::getOpenFileNames(parent, qcaption, qdir, qfilter);

    // Convert QStringList to std::list<std::string> list.
    for(QStringList::iterator q_it = filenames.begin(); q_it != filenames.end(); ++q_it)
        filelist.push_back(q_it->toStdString());

    return filelist;
}

StringList QtUtils::GetOpenRexFileNames(const std::string &dir)
{
    QString qfilter(
        "Images (*.tga; *.bmp; *.jpg; *.jpeg; *.png);;"
        "Sounds (*.ogg; *.wav);;"
        "Ogre 3D Models (*.mesh);;"
        "Ogre Particle Scripts (*.particle);;"
        "Ogre Skeleton (*.skeleton);;"
        "Ogre Material (*.material);;"
        "Flash Animation (*.swf);;"
        "All Files (*.*)");
    QString qcaption("Open");
    QString qdir(dir.c_str());
    QWidget *parent = 0;
    StringList filelist;

    QStringList filenames = QFileDialog::getOpenFileNames(parent, qcaption, qdir, qfilter);

    // Convert QStringList to std::list<std::string> list.
    for(QStringList::iterator q_it = filenames.begin(); q_it != filenames.end(); ++q_it)
        filelist.push_back(q_it->toStdString());

    return filelist;
}

QStringList QtUtils::GetOpenRexFilenames(const std::string &dir)
{
    QString qfilter(
        "Images (*.tga; *.bmp; *.jpg; *.jpeg; *.png);;"
        "Sounds (*.ogg; *.wav);;"
        "Ogre 3D Models (*.mesh);;"
        "Ogre Particle Scripts (*.particle);;"
        "Ogre Skeleton (*.skeleton);;"
        "Ogre Material (*.material);;"
        "Flash Animation (*.swf);;"
        "All Files (*.*)");
    QString qcaption("Open");
    QString qdir(dir.c_str());
    QWidget *parent = 0;

    return QFileDialog::getOpenFileNames(parent, qcaption, qdir, qfilter);
}

std::string QtUtils::GetCurrentPath()
{
    return QDir::currentPath().toStdString();
}

}
