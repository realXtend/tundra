// For conditions of distribution and use, see copyright notice in license.txt

/**
 *  @file   PropertyTableWidget.h
 *  @brief  PropertyTableWidget inherits QTableWidget and add some custom drop-functionality.
 */

#ifndef incl_OgreAssetEditorModule_PropertyTableWidget_h
#define incl_OgreAssetEditorModule_PropertyTableWidget_h

#include <QTableWidget>

namespace Naali
{
    class PropertyTableWidget : public QTableWidget
    {
        Q_OBJECT

    public:
        /// Default constuctor.
        /// @param parent Parent widget.
        explicit PropertyTableWidget(QWidget *parent = 0);

        /// Constuctor.
        /// @param rows Number of rows.
        /// @param columns Number of columns.
        /// @param parent Parent widget.
        explicit PropertyTableWidget(int rows, int columns, QWidget *parent = 0);

        /// Destructor.
        ~PropertyTableWidget();

    protected:
        /// QTableWidget override.
        void dragMoveEvent(QDragMoveEvent *event);

        /// QTableWidget override.
        QStringList mimeTypes() const;

        /// QTableWidget override.
        bool dropMimeData(int row, int column, const QMimeData *data, Qt::DropAction action);

        /// QTableWidget override.
        Qt::DropActions supportedDropActions() const;

    private:
        Q_DISABLE_COPY(PropertyTableWidget);

        /// Convenience function for initializing the widget.
        void InitWidget();
    };
}

#endif
