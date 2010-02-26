// For conditions of distribution and use, see copyright notice in license.txt

/**
 *  @file   OgreScriptEditor.h
 *  @brief  Editing tool for OGRE material and particle scripts.
 *          Provides raw text edit for particles and QProperty editing for materials.
 */

#ifndef incl_OgreAssetEditorModule_OgreScriptEditor_h
#define incl_OgreAssetEditorModule_OgreScriptEditor_h

#include <RexTypes.h>

#include <boost/shared_ptr.hpp>

#include <QWidget>

QT_BEGIN_NAMESPACE
class QVBoxLayout;
class QPushButton;
class QLineEdit;
class QTextEdit;
QT_END_NAMESPACE

namespace UiServices
{
    class UiProxyWidget;
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
    class PropertyTableWidget;

    class OgreScriptEditor : public QWidget
    {
        Q_OBJECT

    public:
        /// Constructor.
        /// @param framework Framework.pointer.
        /// @param asset_type Asset type.
        /// @param name Name.
        OgreScriptEditor(Foundation::Framework *framework,
            const QString &inventory_id,
            const asset_type_t &asset_type,
            const QString &name,
            QWidget *parent = 0);

        /// Destructor.
        virtual ~OgreScriptEditor();

        /// @return ProxyWidget pointer.
        UiServices::UiProxyWidget *GetProxyWidget() const { return proxyWidget_; }

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

        /// Validates the propertys new value.
        /// @param row Row of the cell.
        /// @param column Column of the cell.
        void PropertyChanged(int row, int column);

    signals:
        /// This signal is emitted when the editor is closed.
        void Closed(const QString &inventory_id, asset_type_t asset_type);

    private:
        Q_DISABLE_COPY(OgreScriptEditor);

        /// Initializes the inventory UI.
        void InitEditorWindow();

        /// Creates the text edit field for raw editing.
        void CreateTextEdit();

        /// Creates the property table for material property editing.
        void CreatePropertyEditor();

        /// Framework pointer.
        Foundation::Framework *framework_;

        /// Layout 
        QVBoxLayout *layout_;

        /// Main widget loaded from .ui file.
        QWidget *mainWidget_;

        /// Proxy widget for the ui
        UiServices::UiProxyWidget *proxyWidget_;

        /// Save As button.
        QLineEdit *lineEditName_;

        /// Save As button.
        QPushButton *buttonSaveAs_;

        /// Cancel button.
        QPushButton *buttonCancel_;

        /// Text edit field used in raw edit mode.
        QTextEdit *textEdit_;

        /// Table widget for editing material properties.
        PropertyTableWidget *propertyTable_;

        /// Inventory id.
        QString inventoryId_;

        /// Asset type.
        const asset_type_t assetType_;

        /// Script name.
        QString name_;

        /// Material properties.
        OgreMaterialProperties *materialProperties_;
    };
}

#endif
