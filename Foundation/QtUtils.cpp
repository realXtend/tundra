// For conditions of distribution and use, see copyright notice in license.txt

/// @file QtUtils.cpp
/// @brief Cross-platform utility functions using Qt.

#include "StableHeaders.h"
#include "QtUtils.h"

#include <QString>
#include <QFileDialog>
#include <QDir>

namespace Foundation
{

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
