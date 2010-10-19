//$ HEADER_NEW_FILE $
#ifndef incl_UiExternalModule_UiExternalService_h
#define incl_UiExternalModule_UiExternalService_h

#include "UiExternalServiceInterface.h"
#include "UiExternalModule.h"

#include <QMainWindow>
#include <QDockWidget>
#include <QMenuBar>

namespace UiExternalServices
{
	class UiExternalModule;
	
    /*! Implements UiExternalServiceInterface and provides means of adding external widgets to the 
     *  main window and managing different UI items like menus or actions.
     *  All the services are done by the module managers
     */
	class UIEXTERNAL_MODULE_API UiExternalService : public Foundation::UiExternalServiceInterface
    {
        Q_OBJECT

    public:
        /*! Constuctor.
		 *	\param owner Owner Module (UiExternalModule)
         */
		UiExternalService(UiExternalModule* owner);

        //! Destructor.
        ~UiExternalService();

    public slots:

		/*! \brief	Adds widget to the main Window in a QDockWidget.
         *  \param  widget Widget.
		 *	\param	title Title of the Widget
         *  \param  flags Window flags. Qt::Dialog is used as default		 
         *         
         *  \return widget of the added widget (is a QDockWidget).
         */
        QWidget* AddExternalPanel(QWidget *widget, QString title, Qt::WindowFlags flags = Qt::Dialog);

		/*! TODO!
		 *	\brief	Remove the QDockWidget that contains the widget passed from the main Window.
         *  \param	widget widget.
         *  \return	true if everything is ok (widget deleted)
         *
         *  \note Removes the QDockWidget and the widget.
         */
        bool RemoveExternalPanel(QWidget *widget);

		/*! 
		 *	\brief	Create a new Action, insert it in the Menu "Panels", and connect the action with the Widget to show/hide.
         *  \param  widget Widget.
         *  \param  name Name of the Action (usually the same as the widget)
		 *	\param	menu name of the Menu to put the action inside it
		 *	\param	icon Icon of the action
         *         
         *  \return true if everything is ok (action addded and connected)
         */
		bool AddExternalMenuPanel(QWidget *widget, const QString &name, const QString &menu, const QString &icon = 0);

		/*! \brief	Insert the given action in the Menu of the main window
         *  \param  action Action
         *  \param  name Name of the Action
		 *	\param	menu name of the Menu to put the action inside it
		 *	\param	icon Icon of the action
         *         
         *  \return true if everything is ok (action addded)
         */
		bool AddExternalMenuAction(QAction *action, const QString &name, const QString &menu, const QString &icon = 0);

		/*! \brief	Remove the Action that is connected to the given widget
         *  \param	widget widget.
         *  \return	true if everything is ok (Action deleted)
         *
         *  \note Doesn't Removes the QDockWidget and the widget.
         */
        bool RemoveExternalMenuPanel(QWidget *widget);

        /*! Shows the widget's DockWidget in the main window.
         *  \param widget Widget.
         */
		void ShowWidget(QWidget *widget);

		/*! Hides the widget's DockWidget in the main window.
         *  \param widget Widget.
         */
        void HideWidget(QWidget *widget);

		/*! Switch on/off the edit mode
		 *  \param b Enable/disable the edit mode
		 */
		void SetEnableEditMode(bool b);

		/*! Return if the Edit Mode is enabled
		 *  \return true if Edit Mode is enabled
		 */
		bool IsEditModeEnable();
	
		/*! Add the panel to the Edit Mode configuration
		 *  \param widget Widget
		 */
		void AddPanelToEditMode(QWidget* widget);
		/*! Returns the QDockWidget where the widget with the name widget is in the QMainWindow. Used (at least) to use WorldBuildingModule with this module.
         *  \param widget Name of the widget.
         */
		QDockWidget* GetExternalMenuPanel(QString *widget);

    private:
        //! Owner UI module.
        UiExternalModule *owner_;
		//!Main Window
		QMainWindow *qWin_;
    };
}

#endif
