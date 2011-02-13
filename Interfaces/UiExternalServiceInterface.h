//$ HEADER_NEW_FILE $ 
/*!
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   UiExternalServiceInterface.h
 *  @brief  Interface for Naali's user interface ulitizing Qt's External Widgets like QDockWidgets, QMenus, etc
 *    
 */

#ifndef incl_Interfaces_UiExternalServiceInterface_h
#define incl_Interfaces_UiExternalServiceInterface_h

#include "IService.h"
#include "Entity.h"

#include <QObject>
#include <QWidget>
#include <QDockWidget>
#include <QToolBar>


namespace Foundation
{

    /*! Interface for Naali's user interface ulitizing Qt's External Widgets like QDockWidgets, QMenus, etc
     *  
     */
    class MODULE_API UiExternalServiceInterface : public QObject, public IService
    {
		 Q_OBJECT

    public:
        //! Default constructor.
        UiExternalServiceInterface() {}

        //! Destructor.
        virtual ~UiExternalServiceInterface() {}

    public slots:
        /*! \brief	Adds widget to the main Window in a QDockWidget.
         *  \param  widget Widget.
		 *	\param	title Title of the Widget
         *  \param  flags Window flags. Qt::Dialog is used as default		
         *         
         *  \return widget of the added widget (is a QDockWidget).
         */
        virtual QWidget* AddExternalPanel(QWidget *widget, QString title, Qt::WindowFlags flags = Qt::Dialog) = 0;
		
		/*! \brief	Adds QDockWidget to the main Window.
         *  \param  widget QDockWidget. 
         *         
         *  \return true if everything is ok
         */
		virtual bool AddExternalPanel(QDockWidget *widget) = 0;


        /*! \brief	Remove the QDockWidget that contains the widget passed from the main Window.
         *  \param	widget widget.
         *  \return	true if everything is ok (widget deleted)
         *
         *  \note Removes the QDockWidget and the widget.
         */      
        virtual bool RemoveExternalPanel(QWidget *widget) = 0; 
 
		/*! 
		 *	\brief	Create a new Action, insert it in the Menu "Panels", and connect the action with the Widget to show/hide.
         *  \param  widget Widget.
         *  \param  name Name of the Action (usually the same as the widget)
		 *	\param	menu name of the Menu to put the action inside it
         *         
         *  \return true if everything is ok (action addded and connected)
         */
		virtual bool AddExternalMenuPanel(QWidget *widget, const QString &name, const QString &menu, bool moveable = true) = 0;

		/*! \brief	Insert the given action in the Menu of the main window
         *  \param  action Action
         *  \param  name Name of the Action
		 *	\param	menu name of the Menu to put the action inside it
		 *	\param	icon Icon of the action
         *         
         *  \return true if everything is ok (action addded)
         */
		virtual bool AddExternalMenuAction(QAction *action, const QString &name, const QString &menu, const QString &icon = 0) = 0;

		/*! \brief	Remove the Action that is connected to the given widget
         *  \param	widget widget.
         *  \return	true if everything is ok (Action deleted)
         *
         *  \note Doesn't Removes the QDockWidget and the widget.
         */     
        virtual bool RemoveExternalMenuPanel(QWidget *widget) = 0; 

        /*! Shows the widget's DockWidget in the main window.
         *  \param widget Widget.
         */
        virtual void ShowWidget(QWidget *widget)= 0;

        /*! Hides the widget's DockWidget in the main window.
         *  \param widget Widget.
         */
        virtual void HideWidget(QWidget *widget) = 0;    

		/*! Switch on/off the edit mode
		 *  \param b Enable/disable the edit mode
		 */
		virtual void SetEnableEditMode(bool b) = 0;

		/*! Return if the Edit Mode is enabled
		 *  \return true if Edit Mode is enabled
		 */
		virtual bool IsEditModeEnable() = 0;

		/*! Add the panel to the Edit Mode configuration
		 *  \param widget Widget
		 */
		virtual void AddPanelToEditMode(QWidget* widget) = 0;

		/*! Returns the QDockWidget where the widget with the name widget is in the QMainWindow. 
		 *  Used (at least) to use WorldBuildingModule with this module.
         *  \param widget Name of the widget.
		 *
		 *  \return QDockWidget
         */
		virtual QDockWidget* GetExternalMenuPanel(QString &widget) = 0;

		//TOOLBARS

		/*! Adds a QToolBar given with the name to the main window 
         *  \param toolbar Pointer to the toolbar
		 *	\param name  Name of the Toolbar.
		 *  \return true if everything right
         */
		virtual bool AddExternalToolbar(QToolBar *toolbar, const QString &name) = 0;

		/*! Removes a QToolBar given with the name from the main window 
         *  \param name  Name of the Toolbar.
		 *  \return true if everything right
         */
		virtual bool RemoveExternalToolbar(QString name) = 0;

		/*! Shows a QToolBar given by name
         *  \param name  Name of the Toolbar.
		 *  \return true if everything right
         */
		virtual bool ShowExternalToolbar(QString name) = 0;

		/*! Hide a QToolBar given by name
         *  \param name  Name of the Toolbar.
		 *  \return true if everything right
         */
		virtual bool HideExternalToolbar(QString name) = 0;

		/*! Enable a QToolBar given by name
         *  \param name  Name of the Toolbar.
		 *  \return true if everything right
         */
		virtual bool EnableExternalToolbar(QString name) = 0;

		/*! Disable a QToolBar given by name
         *  \param name  Name of the Toolbar.
		 *  \return true if everything right
         */
		virtual bool DisableExternalToolbar(QString name) = 0;

		/*! Returns a QToolBar with the name given
		 *	
         *  \param name  Name of the Toolbar.
		 *  \return true if everything right
		 *	
		 *	\Note: if the Toolbar doesn't exist, it is created first
         */
		virtual QToolBar* GetExternalToolbar(QString name) = 0;

	signals:
		
		/*! Signal to advise of edit mode change
		 */
		void EditModeChanged(bool b);

    };
}

#endif
