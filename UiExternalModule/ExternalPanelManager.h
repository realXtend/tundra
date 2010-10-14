//$ HEADER_NEW_FILE $
// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_UiExternalModule_ExternalPanelManager_h
#define incl_UiExternalModule_ExternalPanelManager_h

#include "UiExternalModuleApi.h"
#include "ExternalMenuManager.h"

#include <QObject>
#include <QList>
#include <QRectF>
#include <QMap>
#include <QString>
#include <QDockWidget>
#include <QString>

#include <QMainWindow>
#include <QDockWidget>


namespace UiExternalServices
{
    class ExternalPanelManager : public QObject
    {
        Q_OBJECT

    public:
        /*! Constructor.
         *	\param qWin MainWindow pointer.
		 */
        ExternalPanelManager(QMainWindow *qWin);

        //! Destructor.
        ~ExternalPanelManager();

    public slots:
        /*! \brief	Adds widget to the main Window in a QDockWidget.
         *
         *  \param  widget Widget.
		 *	\param	title Title of the Widget
         *  \param  flags Window flags. Qt::Dialog is used as default		 
         *         
         *  \return widget of the added widget (is a QDockWidget).
         */
		QWidget* AddExternalPanel(QWidget *widget, QString title, Qt::WindowFlags flags = Qt::Dialog);

        //! Adds a already created QDockWidget into the main window.
        /*! Please prefer using AddExternalWidget() with normal QWidget and properties instead of this directly.
         *  \param widget QDockWidget.
         */
        bool AddQDockWidget(QDockWidget *widget);

        /*! Remove a widget from main window if it exist there
         *  Used for removing your widget from main window. The show/hide toggle button will also be removed from the main menu.
         *  \param widget Proxy widget.
         */
        bool RemoveExternalPanel(QWidget *widget);

		/*! Shows the widget's DockWidget in the main window.
         *  @param widget Widget.
         */
		void ShowWidget(QWidget *widget);

		/*! Hides the widget's DockWidget in the main window.
         *  @param widget Widget.
         */
		void HideWidget(QWidget *widget);


    private:

        //! Pointer to main QMainWindow
        QMainWindow *qWin_;

        //! Internal list of proxy widgets in scene.
		QList<QDockWidget *> all_qdockwidgets_in_window_;


    private slots:
        //! Deletes widget and the corresponding proxy widget if widget has WA_DeleteOnClose on.
        //! The caller of this slot is retrieved by using QObject::sender().
        void DeleteCallingWidgetOnClose();
    };
}

#endif // incl_UiExternalModule_ExternalPanelManager_h
