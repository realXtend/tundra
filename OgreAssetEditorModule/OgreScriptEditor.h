/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   OgreScriptEditor.h
 *  @brief  Editing tool for OGRE material and particle scripts.
 *          Provides raw text edit for particles and QProperty editing for materials.
 */

#ifndef incl_OgreAssetEditorModule_OgreScriptEditor_h
#define incl_OgreAssetEditorModule_OgreScriptEditor_h

#include "OgreAssetEditorModuleApi.h"
#include "AssetFwd.h"

#include <QWidget>

class QVBoxLayout;
class QPushButton;
class QLineEdit;
class QTextEdit;

class OgreMaterialProperties;
class PropertyTableWidget;

class ASSET_EDITOR_MODULE_API OgreScriptEditor : public QWidget
{
    Q_OBJECT

public:
    enum ScriptType
    {
        MaterialScript,
        ParticleScript
    };

    OgreScriptEditor(ScriptType type, const QString &name, QWidget *parent = 0);

    virtual ~OgreScriptEditor();

    static OgreScriptEditor *OpenOgreScriptEditor(const QString &asset_id, ScriptType type, QWidget* parent = 0);

public slots:
    void Open();

    /// Closes the window.
    void Close();

private slots:
    /// Save As
    void SaveAs();

    /// Validates the script name
    /** @param name Name.
    */
    void ValidateScriptName(const QString &name);

    /// Validates the property's new value.
    /** @param row Row of the cell.
        @param column Column of the cell.
    */
    void PropertyChanged(int row, int column);

    /// Delete this object.
    void Deleted() { delete this; }

private:
    Q_DISABLE_COPY(OgreScriptEditor);

    /// Creates the text edit field for raw editing.
    void CreateTextEdit();

    /// Creates the property table for material property editing.
    void CreatePropertyEditor();

    QWidget *mainWidget_; ///< Main widget loaded from .ui file.
    QLineEdit *lineEditName_; ///< Save As button.
    QPushButton *buttonSaveAs_; ///< Save As button.
    QPushButton *buttonCancel_; ///< Cancel button.
    QTextEdit *textEdit_; ///< Text edit field used in raw edit mode.
    PropertyTableWidget *propertyTable_; ///< Table widget for editing material properties.
    QString inventoryId_; ///< Inventory id.
    ScriptType type_; ///< Script type.
    QString name_; ///< Script name.
    OgreMaterialProperties *materialProperties_; ///< Material properties.
};

#endif
