//$ HEADER_NEW_FILE $
// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "DebugOperatorNew.h"
#include "ExternalToolBarManager.h"

#include <QDebug>

#include "MemoryLeakCheck.h"

namespace UiExternalServices
{
    QString ExternalToolBarManager::defaultItemIcon = "./data/ui/images/menus/edbutton_MATWIZ_normal.png";
    QString ExternalToolBarManager::defaultGroupIcon = "./data/ui/images/menus/edbutton_WRLDTOOLS_icon.png";

    ExternalToolBarManager::ExternalToolBarManager(QMainWindow *main_window) :
            main_window_(main_window)
    {
    }

    ExternalToolBarManager::~ExternalToolBarManager()
    {
        //SAFE_DELETE(root_menu_);
    }

	void ExternalToolBarManager::SceneChanged(const QString &old_name, const QString &new_name)
    {
        if (new_name == "Ether")
			DisableToolBars();
		else
			EnableToolBars();
	}

	void ExternalToolBarManager::DisableToolBars(){
		foreach (QToolBar *tool, controller_toolbar_){
			tool->setDisabled(true);
			tool->setVisible(false);
		}
	}

	void ExternalToolBarManager::EnableToolBars(){
		foreach (QToolBar *tool, controller_toolbar_){
				tool->setVisible(true);
				tool->setDisabled(false); 
		}
	}

	
	bool ExternalToolBarManager::AddExternalToolbar(QToolBar *toolbar, const QString &name){
		//Insert a new QToolBar given by a module
		if (!main_window_ || controller_toolbar_.contains(name))
			return false;

		main_window_->addToolBar(toolbar);
		controller_toolbar_[name]=toolbar;
		HideExternalToolbar(name);
		return true;
	}

	bool ExternalToolBarManager::RemoveExternalToolbar(QString name){
		if (!main_window_ || controller_toolbar_.contains(name))
			return false;

		main_window_->removeToolBar(controller_toolbar_.value(name));
		controller_toolbar_.remove(name);
		return true;
	}

	bool ExternalToolBarManager::ShowExternalToolbar(QString name){
		if (!main_window_ || controller_toolbar_.contains(name))
			return false;

		dynamic_cast<QToolBar*>(controller_toolbar_.value(name))->show();
		return true;
	}

	bool ExternalToolBarManager::HideExternalToolbar(QString name){
		if (!main_window_ || controller_toolbar_.contains(name))
			return false;

		dynamic_cast<QToolBar*>(controller_toolbar_.value(name))->hide();
		return true;
	}

	bool ExternalToolBarManager::EnableExternalToolbar(QString name){
		if (!main_window_ || controller_toolbar_.contains(name))
			return false;

		dynamic_cast<QToolBar*>(controller_toolbar_.value(name))->setEnabled(true);
		return true;
	}
	bool ExternalToolBarManager::DisableExternalToolbar(QString name){
		if (!main_window_ || controller_toolbar_.contains(name))
			return false;

		dynamic_cast<QToolBar*>(controller_toolbar_.value(name))->setEnabled(false);
		return true;
	}
	QToolBar* ExternalToolBarManager::GetExternalToolbar(QString name){
		if (!main_window_)
			return false;

		if (!controller_toolbar_.contains(name)) {
			//Create it and add to list
			QToolBar *aux = new QToolBar(name, main_window_);
			aux->setObjectName(name); 
			AddExternalToolbar(aux, name);			
		}

		return controller_toolbar_.value(name);
	}
}
