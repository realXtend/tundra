//$ HEADER_NEW_FILE $
#ifndef incl_UiModule_ViewManager_h
#define incl_UiModule_ViewManager_h

#include "UiModule.h"
#include "UiServiceInterface.h"
#include "UiAPI.h"
#include "ViewDialog.h"
#include <QMainWindow>
#include <QAction>
#include <QActionGroup>

namespace UiServices
{
	
	class UiModule;

	class UI_MODULE_API ViewManager : public QObject
	{

		Q_OBJECT

	public:

        /*! Constructor.
         * \param owner uiModule
         */
		ViewManager(UiModule *owner);

        //! Destructor.
		~ViewManager();

	public slots:

		//! Create the New View Window.
		void NewViewWindow();

        //! Hide view: hide all widgets.
		void HideView();

        /*! Save o delete the current view when it is changed
         * \param save True save the current view / false delete the current view
         */
		void TogglePreviousView(bool save);

        /*! Show the selected view.
         * \param name Name of view to show
         */
		void ShowView(const QString &name);

        /*! Save a view with the selected name.
         * \param name Name of view to save
         */
		void SaveView(const QString &name);

        /*! Delete the view with the selected name.
         * \param name Name of view to delete
         */
		void DeleteView(const QString &name);

        /*! Rename a view.
         * \param oldName Name of view to rename
         * \param newName New name of view
         */
		void RenameView(const QString &oldName,const QString &newName);

        /*! Call to show view when an action is checked
         * \param action Action checked
         */
		void ActionChanged(QAction* action);

	private:

		UiModule* owner_;
		UiServiceInterface* uiService_;
		QMainWindow* qWin_;
		QMenu* menu_;
		ViewDialog* configWindow_;
		QList<QAction*> actions_;
		QActionGroup* actionGroup_;
		QAction* previous_;

	};
}
#endif