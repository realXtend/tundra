// For conditions of distribution and use, see copyright notice in license.txt

/**
 *  @file   OgreScriptEditor.h
 *  @brief  Editing tool for OGRE material and particle scripts.
 *          Provides raw text edit for particles and QProperty editing for materials.
 */

#ifndef incl_InventoryModule_OgreScriptEditor_h
#define incl_InventoryModule_OgreScriptEditor_h

#include "RexTypes.h"

#include <boost/shared_ptr.hpp>

#include <QObject>

class QPushButton;
class QLineEdit;
class QWidget;
class QTextEdit;

namespace QtUI
{
    class UICanvas;
}

namespace Foundation
{
    class Framework;
    class AssetInterface;
    typedef boost::shared_ptr<AssetInterface> AssetPtr;
}

namespace PropertyEditor
{
    class PropertyEditor;
}

namespace OgreAssetEditor
{
    class OgreMaterialProperties;

    class OgreScriptEditor : public QObject
    {
        Q_OBJECT

    public:
        /// Constructor.
        /// @param framework Framework.pointer.
        /// @param asset_type Asset type.
        /// @param name Name.
        OgreScriptEditor(Foundation::Framework *framework,
            const RexTypes::asset_type_t &asset_type,
            const QString &name);

        /// Destructor.
        virtual ~OgreScriptEditor();

    public slots:
        ///
        void HandleAssetReady(Foundation::AssetPtr asset);

    private slots:
        /// Save As
        void SaveAs();

        /// Cancel
        void Cancel();

    private:
        Q_DISABLE_COPY(OgreScriptEditor);

        /// Initializes the inventory UI.
        void InitEditorWindow();

        /// Framework pointer.
        Foundation::Framework *framework_;

        /// Canvas for the inventory window.
        boost::shared_ptr<QtUI::UICanvas> canvas_;

        /// The editor main window widget.
        QWidget *mainWidget_;

        /// Property editor widget.
        QWidget *editorWidget_;

        /// Save As button.
        QLineEdit *lineEditName_;

        /// Save As button.
        QPushButton *buttonSaveAs_;

        /// Cancel button.
        QPushButton *buttonCancel_;

        /// Property editor.
        PropertyEditor::PropertyEditor *propertyEditor_;

        /// Text edit field used in raw edit mode.
        QTextEdit *textEdit_;

        /// Asset type.
        const RexTypes::asset_type_t assetType_;
        
        OgreMaterialProperties *materialProperties_;
    };
}

#endif
