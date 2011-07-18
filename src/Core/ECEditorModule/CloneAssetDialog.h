/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   CloneAssetDialog.h
 *  @brief  Dialog for cloning asset.
 */

#pragma once

#include "AssetFwd.h"

#include <QDialog>

class QLineEdit;
class QPushButton;
class QLabel;

/// Dialog for cloning asset.
/** The dialog is deleted when it's closed.
*/
class CloneAssetDialog : public QDialog
{
    Q_OBJECT

public:
    /// Constructs the dialog.
    /** @param asset Asset to be cloned.
        @param assetApi Core Asset API.
        @param parent Parent widget.
        @param f Window flags.
    */
    CloneAssetDialog(const AssetPtr &asset, AssetAPI *assetApi, QWidget *parent = 0, Qt::WindowFlags f = 0);

    /// Returns text in the name line edit.
    QString NewName() const;

    /// Returns the asset to be cloned.
    AssetWeakPtr Asset() const { return asset; }

protected:
    /// QWidget override. Deletes the window.
    void hideEvent(QHideEvent *e);

private slots:
    /// Validates that the request new name isn't already taken.
    /** If the name's already taken, the OK button is disabled.
        @param newName The requested new name.
    */
    void ValidateNewName(const QString &newName);

private:
    QPushButton *okButton; ///< OK button.
    QLineEdit *nameLineEdit; ///< New name line edit.
    QLabel *errorLabel; ///< Label for showing error message.
    AssetAPI *assetApi; ///< Asset core API.
    AssetWeakPtr asset; ///< Asset to be cloned.
};
