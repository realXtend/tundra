//$ HEADER_NEW_FILE $
#ifndef incl_UiModule_ViewManager_h
#define incl_UiModule_ViewManager_h

#include "UiModule.h"
#include "UiSceneService.h"
#include "NaaliUi.h"
#include "ViewDialog.h"

#include <QActionGroup>

namespace UiServices
{
	
	class UiModule;
	class UiSceneService;

	class UI_MODULE_API ViewManager : public QObject
	{

		Q_OBJECT

	public:
			ViewManager(UiModule *owner,UiSceneService* uiservice);

			~ViewManager();

	public slots:
			void NewViewWindow();

			void HideView();

			void ShowView(const QString &name);

			void SaveView(const QString &name);

			void DeleteView(const QString &name);

			void RenameView(const QString &oldName,const QString &newName);

			void ActionChanged(QAction* action);

	private:

		UiModule* owner_;
		UiSceneService* uiService_;
		QMainWindow* qWin_;
		QMenu* menu_;
		ViewDialog* configWindow_;
		QList<QAction*> actions_;
		QActionGroup* actionGroup_;

	};
}
#endif