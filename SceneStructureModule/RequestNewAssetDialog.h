/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   RequestNewAssetDialog.h
 *  @brief  Dialog for requesting new asset from arbitrary source.
 */

#ifndef incl_SceneStructureModule_RequestNewAssetDialog_h
#define incl_SceneStructureModule_RequestNewAssetDialog_h

#include <QDialog>

class QLineEdit;
class QComboBox;

class AssetAPI;

/// Dialog for requesting new asset from arbitrary source.
/** The dialog is deleted when it's closed.
*/
class RequestNewAssetDialog : public QDialog
{
    Q_OBJECT

public:
    /// Constructs the dialog.
    /** @param assetApi Core Asset API.
        @param parent Parent widget.
        @param f Window flags.
    */
    RequestNewAssetDialog(AssetAPI *assetApi, QWidget *parent = 0, Qt::WindowFlags f = 0);

    /// Returns text in the source line edit.
    QString Source() const;

    /// Returns text in the type combo box.
    QString Type() const;

protected:
    /// QWidget override. Deletes the window.
    void hideEvent(QHideEvent *e);

private:
    QLineEdit *sourceLineEdit; ///< Source line edit.
    QComboBox *typeComboBox;  ///< Asset type combo box.
};

#endif
