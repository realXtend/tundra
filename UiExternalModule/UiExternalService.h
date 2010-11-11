//$ HEADER_NEW_FILE $
#ifndef incl_UiExternalModule_UiExternalService_h
#define incl_UiExternalModule_UiExternalService_h

#include "UiExternalServiceInterface.h"
#include "UiExternalModule.h"

#include <QMainWindow>
#include <QDockWidget>
#include <QMenuBar>
#include <QMenu>

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

		void HandleEvent(event_category_id_t category_id, event_id_t event_id, IEventData* data);

    public slots:

		/*! \brief	Adds widget to the main Window in a QDockWidget.
         *  \param  widget Widget.
		 *	\param	title Title of the Widget
         *  \param  flags Window flags. Qt::Dialog is used as default		 
         *         
         *  \return widget of the added widget (is a QDockWidget).
         */
        QWidget* AddExternalPanel(QWidget *widget, QString title, Qt::WindowFlags flags = Qt::Dialog);

		/*! \brief	Adds QDockWidget to the main Window.
         *  \param  widget QDockWidget. 
         *         
         *  \return true if everything is ok
         */
		bool AddExternalPanel(QDockWidget *widget);

		/*! \brief	Remove the QDockWidget that contains the widget passed from the main Window.
         *  \param	widget widget.
         *  \return	true if everything is ok (widget deleted)
         *
         *  \note Removes the QDockWidget and the widget.
         */
        bool RemoveExternalPanel(QWidget *widget);

		/*! \brief	Create a new Action, insert it in the Menu "Panels", and connect the action with the Widget to show/hide.
         *  \param  widget Widget.
         *  \param  name Name of the Action (usually the same as the widget)
		 *	\param	menu name of the Menu to put the action inside it
		 *	\param	icon Icon of the action
         *         
         *  \return true if everything is ok (action addded and connected)
         */
		bool AddExternalMenuPanel(QWidget *widget, const QString &name, const QString &menu, bool moveable = true);

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

		/*! Returns the QDockWidget where the widget with the name widget is in the QMainWindow. 
		 *  Used (at least) to use WorldBuildingModule with this module.
         *  \param widget Name of the widget.
		 *
		 *  \return QDockWidget
         */
		QDockWidget* GetExternalMenuPanel(QString &widget);

		//TOOLBARS

		/*! Adds a QToolBar given with the name to the main window 
         *  \param toolbar Pointer to the toolbar
		 *	\param name  Name of the Toolbar.
		 *  \return true if everything right
         */
		bool AddExternalToolbar(QToolBar *toolbar, const QString &name);

		/*! Removes a QToolBar given with the name from the main window 
         *  \param name  Name of the Toolbar.
		 *  \return true if everything right
         */
		bool RemoveExternalToolbar(QString name);

		/*! Shows a QToolBar given by name
         *  \param name  Name of the Toolbar.
		 *  \return true if everything right
         */
		bool ShowExternalToolbar(QString name);

		/*! Hide a QToolBar given by name
         *  \param name  Name of the Toolbar.
		 *  \return true if everything right
         */
		bool HideExternalToolbar(QString name);

		/*! Enable a QToolBar given by name
         *  \param name  Name of the Toolbar.
		 *  \return true if everything right
         */
		bool EnableExternalToolbar(QString name);

		/*! Disable a QToolBar given by name
         *  \param name  Name of the Toolbar.
		 *  \return true if everything right
         */
		bool DisableExternalToolbar(QString name);

		/*! Returns a QToolBar with the name given
		 *	
         *  \param name  Name of the Toolbar.
		 *  \return true if everything right
		 *	
		 *	\Note: if the Toolbar doesn't exist, it is created first
         */
		QToolBar* GetExternalToolbar(QString name);

		void SceneChanged(const QString &old_name, const QString &new_name);
	
  //  private slots:

		////! Open the EC Editor Window.
		//void openECEditor();
		////! Open the Build Window..
		//void openBuild();

    private:

		/*! Create menu and actions to right button click functionality
		 *	note: not use for now
		 */
		//void CreateMenu();

        //! Owner UI module.
        UiExternalModule *owner_;

		//!Main Window
		QMainWindow *qWin_;

		//! Entity selected
		 Scene::Entity* entitySelected_;

		 //! Name of the current scene
		 QString currentScene;

		 //! Query category of events
		 event_category_id_t scene_event_category_;

		 //! Right button click menu
		 QMenu* menu_asset;

		 //! Edit Mode
		 bool edit_mode_;
    };
}

#endif
