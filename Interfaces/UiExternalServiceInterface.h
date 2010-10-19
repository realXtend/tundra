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

#include "ServiceInterface.h"

#include <QObject>
#include <QWidget>
#include <QDockWidget>


namespace Foundation
{

    /*! Interface for Naali's user interface ulitizing Qt's External Widgets like QDockWidgets, QMenus, etc
     *  
     */
    class MODULE_API UiExternalServiceInterface : public QObject, public Foundation::ServiceInterface
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


        /*! TODO!
		 *	\brief	Remove the QDockWidget that contains the widget passed from the main Window.
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
		 *	\param	icon Icon of the action
         *         
         *  \return true if everything is ok (action addded and connected)
         */
		virtual bool AddExternalMenuPanel(QWidget *widget, const QString &name, const QString &menu, const QString &icon = 0) = 0;

		/*! \brief	Insert the given action in the Menu of the main window
         *  \param  action Action
         *  \param  name Name of the Action
		 *	\param	menu name of the Menu to put the action inside it
		 *	\param	icon Icon of the action
         *         
         *  \return true if everything is ok (action addded)
         */
		virtual bool AddExternalMenuAction(QAction *action, const QString &name, const QString &menu, const QString &icon = 0) = 0;

		/*! TODO!!
		 *	\brief	Remove the Action that is connected to the given widget
         *  \param	widget widget.
         *  \return	true if everything is ok (Action deleted)
         *
         *  \note Doesn't Removes the QDockWidget and the widget.
         */     
        virtual bool RemoveExternalMenuPanel(QWidget *widget) = 0; 

        /*! Shows the widget's DockWidget in the main window.
         *  @param widget Widget.
         */
        virtual void ShowWidget(QWidget *widget)= 0;

        /*! Hides the widget's DockWidget in the main window.
         *  @param widget Widget.
         */
        virtual void HideWidget(QWidget *widget) = 0;       

		/*! Returns the QDockWidget where the widget with the name widget is in the QMainWindow. Used (at least) to use WorldBuildingModule with this module.
         *  \param widget Name of the widget.
         */
		virtual QDockWidget* GetExternalMenuPanel(QString *widget) = 0;
    };
}

#endif
