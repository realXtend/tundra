/**
 *  For conditions of distribution and use, see copyright notice in LICENSE
 *
 *  @file   OgreScriptEditor.h
 *  @brief  Text editing tool for OGRE material and particle scripts.
 */

#pragma once

#include "OgreAssetEditorModuleApi.h"
#include "AssetFwd.h"

#include <QWidget>

class QVBoxLayout;
class QPushButton;
class QLineEdit;
class QTextEdit;

class OgreMaterialProperties;
class PropertyTableWidget;

/// Text editing tool for OGRE material and particle scripts.
class ASSET_EDITOR_MODULE_API OgreScriptEditor : public QWidget
{
    Q_OBJECT

public:
    OgreScriptEditor(const AssetPtr &scriptAsset, Framework *fw, QWidget *parent = 0);
    ~OgreScriptEditor();

    void SetScriptAsset(const AssetPtr &scriptAsset);

public slots:
    void Open();

private:
    Q_DISABLE_COPY(OgreScriptEditor);

    /// Creates the text edit field for raw editing.
    void CreateTextEdit();

    /// Creates the property table for material property editing.
    void CreatePropertyEditor();

    Framework *framework;
    AssetWeakPtr asset;
    QLineEdit *lineEditName; ///< Asset name line edit.
    QPushButton *buttonSave; ///< Save button.
    QPushButton *buttonSaveAs; ///< Save As button.
    QTextEdit *textEdit; ///< Text edit field used in raw edit mode.
    PropertyTableWidget *propertyTable; ///< Table widget for editing material properties.
    OgreMaterialProperties *materialProperties; ///< Material properties.

private slots:
    /// Saves changes made to the asset.
    void Save();

    /// Saves as new.
    void SaveAs();

    /// Validates the script name.
    /** @param name Name. */
    void ValidateScriptName(const QString &name);

    /// Validates the property's new value.
    /** @param row Row of the cell.
        @param column Column of the cell. */
    void PropertyChanged(int row, int column);

    void OnAssetTransferSucceeded(AssetPtr asset);
    void OnAssetTransferFailed(IAssetTransfer *transfer, QString reason);
};
