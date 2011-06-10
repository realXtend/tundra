//$ HEADER_NEW_FILE $
// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_UiModule_ExternalPanelManager_h
#define incl_UiModule_ExternalPanelManager_h

#include "UiModuleApi.h"
#include "ExternalMenuManager.h"
#include "UiWidget.h"

#include <QObject>
#include <QList>
#include <QRectF>
#include <QMap>
#include <QString>
#include <QDockWidget>
#include <QString>

#include <QMainWindow>
#include <QDockWidget>


namespace UiServices
{
    class UI_MODULE_API ExternalPanelManager : public QObject
    {
        Q_OBJECT

    public:
        /*! Constructor.
         *	\param qWin MainWindow pointer.
		 */
		ExternalPanelManager(QMainWindow *qWin, UiModule *owner);

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
		QDockWidget* AddExternalPanel(UiWidget *widget, QString title, Qt::WindowFlags flags = Qt::Dialog);

        //! Adds a already created QDockWidget into the main window.
        /*! Please prefer using AddExternalWidget() with normal QWidget and properties instead of this directly.
         *  \param widget QDockWidget.
         */
        bool AddQDockWidget(QDockWidget *widget);

        /*! Remove a widget from main window if it exist there
         *  Used for removing your widget from main window. The show/hide toggle button will also be removed from the main menu.
         *  \param widget Proxy widget.
         */
        bool RemoveExternalPanel(QDockWidget *widget);

		/*!Disable all the dockwidgets in the qmainwindow
		 * 
		 */
		void DisableDockWidgets();

		/*!Enable all the dockwidgets in the qmainwindow
		 * 
		 */
		void EnableDockWidgets();

		void restoreWidget(QDockWidget *widget);

    private slots:
        void ModifyPanelVisibility(bool vis);
        void DockVisibilityChanged(bool vis);
		
    private:

        //! Pointer to main QMainWindow
        QMainWindow *qWin_;

		//Pointer to owner
		UiModule *owner_;

        QMap<QDockWidget*, bool> controller_panels_;
    };
}

#endif // incl_UiModule_ExternalPanelManager_h
