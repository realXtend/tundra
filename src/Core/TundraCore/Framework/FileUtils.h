/**
    For conditions of distribution and use, see copyright notice in LICENSE

    @file   FileUtils.h
    @brief  Utility file dialog functions. */

#pragma once

#include "TundraCoreApi.h"
#include "CoreTypes.h"

#include <QStringList>
#include <QDir>

class QFileDialog;

/// Searches contents of directory.
/** @param path Path to search.
    @param recursive Do we want to search contents of directory recursively i.e. include all subdirectories.
    @param filters What is wanted to be found (directories, files, etc)
    @return List of absolute paths that match the filter. */
QStringList TUNDRACORE_API DirectorySearch(const QString &path, bool recursive, QDir::Filters filters);

/// Opens a non-modal file dialog
/** @param filter The files to be shown.
    @param caption Dialog's caption
    @param dir Working directory.
    @param parent Parent widget. If you give parent widget, the file dialog will be part of the UI scene.
        Give 0 if you want the dialog to be external.
    @param initiator QObject that initiated the dialog and wants the results
    @param slot Slot on initiator object, to which dialog's finished() signal will be 
           connected. Result value will be 1 if OK was pressed. Use sender() and dynamic 
           cast to QFileDialog to get to know the chosen file(s)
    @param multipleFiles Do we want to be able to select multiple files.
    @return The created file dialog */
QFileDialog TUNDRACORE_API *OpenFileDialogNonModal(
    const QString& filter,
    const QString& caption,
    const QString& dir,
    QWidget* parent,
    QObject* initiator,
    const char* slot,
    bool multipleFiles = false);

/// Opens a non-modal file save dialog
/** @param filter The files to be shown.
    @param caption Dialog's caption
    @param dir Working directory.
    @param parent Parent widget. If you give parent widget, the file dialog will be part of the UI scene.
        Give 0 if you want the dialog to be external.
    @param initiator QObject that initiated the dialog and wants the results
    @param slot Slot on initiator object, to which dialog's finished() signal will be 
           connected. Result value will be 1 if OK was pressed. Use sender() and dynamic 
           cast to QFileDialog to get to know the chosen file(s)
    @return The created file dialog */
QFileDialog TUNDRACORE_API *SaveFileDialogNonModal(
    const QString& filter,
    const QString& caption,
    const QString& dir,
    QWidget* parent,
    QObject* initiator,
    const char* slot);

/// Opens non-modal select directory dialog.
/** @note Differs from other functions in that this won't send a finished signal if user closes the dialog without
          selecting a directory.
    @param caption Dialog's caption
    @param dir Working directory.
    @param parent Parent widget. If you give parent widget, the file dialog will be part of the UI scene.
        Give 0 if you want the dialog to be external.
    @param initiator QObject that initiated the dialog and wants the results
    @param slot Slot on initiator object, to which dialog's finished() signal will be 
           connected. Result value will be 1 if OK was pressed. Use sender() and dynamic 
           cast to QFileDialog to get to know the chosen file(s)
    @return The created file dialog */
QFileDialog TUNDRACORE_API *DirectoryDialogNonModal(
    const QString &caption,
    const QString &dir,
    QWidget *parent,
    QObject* initiator,
    const char* slot);
