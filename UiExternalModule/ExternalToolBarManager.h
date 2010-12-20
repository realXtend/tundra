//$ HEADER_NEW_FILE $
// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_UiExternalModule_ExternalToolBarManager_h
#define incl_UiExternalModule_ExternalToolBarManager_h

#include "UiExternalModule.h"

#include <QObject>
#include <QMap>
#include <QWidget>
#include <QToolBar>
#include <QAction>
#include <QStringList>


namespace UiExternalServices
{
	class UiExternalModule;

	/*! Is the ToolBar Manager of the main window, and implements the services offered by UiExternalService 
     *  All the actions and ToolBars are managed by this class
     */
    class ExternalToolBarManager : public QObject
    {
        Q_OBJECT

    public:
        /*! Constuctor. Gets the main menubar of the main window.
         *  \param parent Main MenuBar of the main window
         */
		ExternalToolBarManager(QMainWindow *main_window, UiExternalModule *owner);

        //! Destructor.
        ~ExternalToolBarManager();

    public slots:
        
		/*!Slot used when the scene is changed, if we "go" to ether Scene, then disable the Menu "Panels" in the Menu Bar
		 */
		void SceneChanged(const QString &old_name, const QString &new_name);

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

		/*! Disables all QToolBars in QMainWindow
		 *	
         */
		void DisableToolBars();

		/*! Enables all QToolBars in QMainWindow
		 *	
         */
		void EnableToolBars();

    private slots:

		void ActionNodeClicked();


    private:

        //! Track maps
        QMap<QString, QToolBar*> controller_toolbar_;

        //! Pointer to the main menu Bar
		QMainWindow *main_window_;

		//Owner of the class
		UiExternalModule *owner_;

        /// Default icon for menu item node.
        static QString defaultItemIcon;

        /// Default icon for menu group node.
        static QString defaultGroupIcon;
    };
}

#endif
