// For conditions of distribution and use, see copyright notice in license.txt

/**
 *  @file   OgreScriptEditor.h
 *  @brief  Editing tool for OGRE material and particle scripts.
 *          Provides raw text edit for particles and QProperty editing for materials.
 */

#ifndef incl_OgreAssetEditorModule_OgreScriptEditor_h
#define incl_OgreAssetEditorModule_OgreScriptEditor_h

#include "RexTypes.h"

#include <boost/shared_ptr.hpp>

#include <QObject>

QT_BEGIN_NAMESPACE
class QPushButton;
class QLineEdit;
class QWidget;
class QTextEdit;
class QTableWidget;
QT_END_NAMESPACE

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
        /// Handles the asset data for script.
        void HandleAssetReady(Foundation::AssetPtr asset);

        /// Closes the window.
        void Close();

    private slots:
        /// Save As
        void SaveAs();

        /// Validates the script name
        /// @param name Name.
        void ValidateScriptName(const QString &name);

    private:
        Q_DISABLE_COPY(OgreScriptEditor);

        /// Initializes the inventory UI.
        void InitEditorWindow();

        /// 
        void CreateTextEdit();

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

        /// Text edit field used in raw edit mode.
        QTextEdit *textEdit_;

        /// Asset type.
        const RexTypes::asset_type_t assetType_;

        /// Name.
        QString name_;

        /// Table widget for editing material properties.
        QTableWidget *propertyTable_;

        ///
        OgreMaterialProperties *materialProperties_;
    };
}

#endif
