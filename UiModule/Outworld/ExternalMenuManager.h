//$ HEADER_NEW_FILE $
// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_UiModule_ExternalMenuManager_h
#define incl_UiModule_ExternalMenuManager_h

#include "UiModule.h"

#include <QObject>
#include <QMap>
#include <QDockWidget>
#include <QWidget>
#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QStringList>


namespace UiServices
{
	class UiModule;

	/*! Is the Menu Manager of the main window, and implements the services offered by UiExternalService 
     *  All the actions and menus are managed by this class
     */
    class UI_MODULE_API ExternalMenuManager : public QObject
    {
        Q_OBJECT

    public:
        /*! Constuctor. Gets the main menubar of the main window.
         *  \param parent Main MenuBar of the main window
         */
		ExternalMenuManager(QMenuBar *parent, UiModule *owner);

        //! Destructor.
        ~ExternalMenuManager();

    public slots:
        /*! Adds new menu item.
         *  \param widget Controlled widget.
         *  \param name Name of the menu entry.
         *  \param menu Name of the menu. If the menu doesn't exist, it is created. If no name is given the entry is added to the menu others
         *  \param icon Path to image which will be used as the icon for the entry. If no path is given, default icon is used.
		 *
		 *	\note: a new action is created and connected to the given widget
         */
        bool AddExternalMenuPanel(QWidget *widget, const QString &name, const QString &menu);

		/*! \brief	Insert the given meu in the Menu of the main window
         *  \param  action menu
         *  \param  name Name of the menu
		 *	\param	menu name of the Menu to put the menu inside it
		 *	\param	icon Icon of the menu
         *         
         *  \return true if everything is ok (action addded)
         */
		bool AddExternalMenu(QMenu *new_menu, const QString &menu, const QString &icon);

        /*! Removes menu item.
         *  \param widget Controlled widget.
         */
        bool RemoveExternalMenuPanel(QWidget *controlled_widget);

		/*! Adds new menu action.
         *  \param action new action.
         *  \param name Name of the menu entry.
         *  \param menu Name of the menu. If the menu doesn't exist, it is created. If no name is given the entry is added to the menu others
         *  \param icon Path to image which will be used as the icon for the entry. If no path is given, default icon is used.
		 *
		 *	\note There isn't any management of the given action
         */
		bool AddExternalMenuAction(QAction *action, const QString &name, const QString &menu, const QString &icon = 0);

		/*! Enable Menu "Panels" in the Menu Bar
		 */
		void EnableMenus();

		/*! Disable Menu "Panels" in the Menu Bar
		 */
		void DisableMenus();

		void ModifyPanelVisibility(bool vis);

    private slots:

		/*! Adds a new Menu in the Main MenuBar
		 *	\param name name of the new menu
		 *	\param icon icon (optional)
		 */
        void AddMenu(const QString &name, const QString &icon = defaultGroupIcon);

		/*! Private Slot to manage the action clicked
		*/
        void ActionNodeClicked();	

		/*! Private Slot to manage the action clicked
		
        void ActionNodeClickedInside();	

		/*! Private Slot to manage the action clicked
		
        void ActionNodeClickedOutside();*/

    private:

        //! Track maps
        QMap<QString, QWidget*> controller_panels_;
		QMap<QString, QAction*> controller_actions_;
        QMap<QString, QMenu*> category_menu_;
		QMap<QString, bool> controller_panels_visibility_;

        //! Pointer to the main menu Bar
        QMenuBar *root_menu_;
		UiModule *owner_;

        /// Default icon for menu item node.
        static QString defaultItemIcon;

        /// Default icon for menu group node.
        static QString defaultGroupIcon;
    };
}

#endif
