//$ HEADER_NEW_FILE $
// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_UiModule_ExternalMenuManager_h
#define incl_UiModule_ExternalMenuManager_h

#include "UiModule.h"

#include <QObject>
#include <QMap>
#include <QPair>
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

		typedef QPair<QString, QAction *> menu_action_pair_;

    public slots:

		/*! \brief	Insert the given menu in the Menu of the main window
         *  \param  name Name of the menu
		 *	\param	priority priority to be placed
         *         
         *  \return true if everything is ok (action addded)
         */
		bool AddExternalMenu(const QString &menu, int priority = 50);

        /*! Adds new menu item.
         *  \param widget Controlled widget.
         *  \param name Name of the menu entry.
         *  \param menu Name of the menu. If the menu doesn't exist, it is created. If no name is given the entry is added to the menu others
         *  \param icon Path to image which will be used as the icon for the entry. If no path is given, default icon is used.
		 *
		 *	\note: a new action is created and connected to the given widget
         */
        bool AddExternalMenuPanel(QWidget *widget, const QString &name, const QString &menu, int priority = 50);

		/*! \brief	Insert the given meu in the Menu of the main window
         *  \param  action menu
         *  \param  name Name of the menu
		 *	\param	menu name of the Menu to put the menu inside it
		 *	\param	icon Icon of the menu
         *         
         *  \return true if everything is ok (action addded)
         */
		bool AddExternalMenuToMenu(QMenu *new_menu, const QString &menu, const QString &icon, int priority = 50);

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
		bool AddExternalMenuAction(QAction *action, const QString &name, const QString &menu, const QString &icon = 0, int priority = 50, bool ischeckable = false);

		/*! \brief	Removes the given action from QMainWindow
         *  \param  action Action
         *  \return true if everything is ok (action removed)
         */
		bool RemoveExternalMenuAction(QAction *action);

		/*! Enable Menu "Panels" in the Menu Bar
		 */
		void EnableMenus();

		/*! Disable Menu "Panels" in the Menu Bar
		 */
		void DisableMenus();

		void ModifyPanelVisibility(bool vis);

    private slots:
		/*! Private Slot to manage the action clicked
		*/
        void ActionNodeClicked();

		void SortMenus();

    private:

        //! Track maps
		QMap<QString, menu_action_pair_> all_actions_; //Qobject means qmenu and qaction
		QMap<QString, QString> all_menus_;

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
