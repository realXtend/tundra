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

#include <QObject>
#include <QTableWidget>

QT_BEGIN_NAMESPACE
class QPushButton;
class QLineEdit;
class QWidget;
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

    /// PropertyTableWidget inherits QTableWidget and add some custom drop-functionality.
    class PropertyTableWidget : public QTableWidget
    {
    public:
        /// Default constuctor.
        /// @param parent Parent widget.
        explicit PropertyTableWidget(QWidget *parent = 0);

        /// Constuctor.
        /// @param rows Number of rows.
        /// @param columns Number of columns.
        /// @param parent Parent widget.
        PropertyTableWidget(int rows, int columns, QWidget *parent = 0);

        /// Destructor.
        ~PropertyTableWidget();

    protected:
        Q_DISABLE_COPY(PropertyTableWidget);

        /// QTableWidget override.
        QStringList mimeTypes() const;

        /// QTableWidget override.
        bool dropMimeData (int row, int column, const QMimeData *data, Qt::DropAction action);

        /// QTableWidget override.
        Qt::DropActions supportedDropActions() const;

    private:
        /// Convenience function for initializing the widget.
        void InitWidget();
    };

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

        /// Validates the propertys new value.
        /// @param row Row of the cell.
        /// @param column Column of the cell.
        void PropertyChanged(int row, int column);

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

        /// Proxy widget for the ui
        UiServices::UiProxyWidget *proxyWidget_;

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

        /// Table widget for editing material properties.
        //QTableWidget 
        PropertyTableWidget *propertyTable_;

        /// Asset type.
        const RexTypes::asset_type_t assetType_;

        /// Script name.
        QString name_;

        /// Material properties.
        OgreMaterialProperties *materialProperties_;
    };
}

#endif
