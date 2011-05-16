//$ HEADER_NEW_FILE $
// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "DebugOperatorNew.h"
#include "ExternalMenuManager.h"
#include "UiServiceInterface.h"

#include <QDebug>
#include <QTabBar>

#include "MemoryLeakCheck.h"

namespace UiServices
{
    QString ExternalMenuManager::defaultItemIcon = "./data/ui/images/menus/edbutton_MATWIZ_normal.png";
    QString ExternalMenuManager::defaultGroupIcon = "./data/ui/images/menus/edbutton_WRLDTOOLS_icon.png";

    ExternalMenuManager::ExternalMenuManager(QMenuBar *parent, UiModule *owner) :
            root_menu_(parent),
			owner_(owner),
			controller_panels_visibility_(),
			category_menu_(),
			all_actions_(),
			all_menus_()
	{
        root_menu_->setVisible(false); // todo read from ini
    }

    ExternalMenuManager::~ExternalMenuManager()
    {
        SAFE_DELETE(root_menu_);
    }

	bool ExternalMenuManager::AddExternalMenu(const QString &name, int priority)
    {
		//Creates a new Menu, if it doesnt exist already
		if (!root_menu_)
			return false;

		if (category_menu_.contains(name))
			return false;

		if (!root_menu_->isVisible())
			root_menu_->setVisible(true);

		all_menus_[QString::number(100 - priority) + "_" + name] = name;
		SortMenus();
		return true;
    }

	bool ExternalMenuManager::AddExternalMenuToMenu(QMenu *new_menu, const QString &menu, const QString &icon, int priority)
	{
		if (menu.isEmpty())  
        {
			AddExternalMenu("Others");
			all_actions_[QString::number(100 - priority) + "_" + new_menu->windowTitle()] = menu_action_pair_("Others", category_menu_["Others"]->addMenu(new_menu));
        }
        else if (category_menu_.contains(menu))
			all_actions_[QString::number(100 - priority) + "_" + new_menu->windowTitle()] = menu_action_pair_(menu, category_menu_[menu]->addMenu(new_menu));
        else
        {
            AddExternalMenu(menu);
			all_actions_[QString::number(100 - priority) + "_" + new_menu->windowTitle()] = menu_action_pair_(menu, category_menu_[menu]->addMenu(new_menu));
        }
		return true;	
	}

	bool ExternalMenuManager::AddExternalMenuAction(QAction *action, const QString &name, const QString &menu, const QString &icon, int priority, bool ischeckable)
    {
		if (menu.isEmpty())  
        {
			AddExternalMenu("Others");
			all_actions_[QString::number(100 - priority) + "_" + name] = menu_action_pair_("Others", action);
        }
        else if (category_menu_.contains(menu))
			all_actions_[QString::number(100 - priority) + "_" + name] = menu_action_pair_(menu, action);
        else
        {
            AddExternalMenu(menu);
			all_actions_[QString::number(100 - priority) + "_" + name] = menu_action_pair_(menu, action);
        }
		if (ischeckable)
			action->setCheckable(true);
		SortMenus();
		return true;	
	}

	bool ExternalMenuManager::RemoveExternalMenuAction(QAction *action)
	{
		//The widget is the widget of RealXtend that is inside a dockwidget!
		foreach(menu_action_pair_ pair, all_actions_)
			if (action == pair.second)
			{
				QObject::disconnect(pair.second, SIGNAL(triggered()), this, SLOT(ActionNodeClicked()));
				controller_panels_visibility_.remove(action->text());
				all_actions_.remove(all_actions_.key(pair));
				SAFE_DELETE(pair.second);
				//Check if any menu has 0 items, then delete it!
				QMutableMapIterator<QString, QMenu*> i(category_menu_);
				while (i.hasNext()) {
					i.next();
					if (i.value()->isEmpty()) {					
						SAFE_DELETE(i.value());
						category_menu_.remove(i.key());
					}
				}
				SortMenus();
				return true;
			}
		return false;
	}

	bool ExternalMenuManager::AddExternalMenuPanel(QWidget *widget, const QString &name, const QString &menu, int priority)
    {       
		//Check if is not already
		foreach(menu_action_pair_ pair, all_actions_)
			if (menu == pair.first && widget->windowTitle() == pair.second->text())
				return false;

		QAction *action = new QAction(widget->windowTitle(), widget);
		action->setCheckable(true);
		if (menu.isEmpty())		
        {
			AddExternalMenu("Others");
			all_actions_[QString::number(100 - priority) + "_" + action->text()] = menu_action_pair_("Others", action);
        }
        else if (category_menu_.contains(menu))
			all_actions_[QString::number(100 - priority) + "_" + action->text()] = menu_action_pair_(menu, action);
        else
        {
            AddExternalMenu(menu);
			all_actions_[QString::number(100 - priority) + "_" + action->text()] = menu_action_pair_(menu, action);
        }
		controller_panels_visibility_[widget->windowTitle()] = dynamic_cast<QDockWidget *>(widget)->isVisible();
		connect(action, SIGNAL(triggered()), SLOT(ActionNodeClicked()));
		connect(widget, SIGNAL(visibilityChanged(bool)), SLOT(ModifyPanelVisibility(bool)));
		SortMenus();
		return true;
    }

	void ExternalMenuManager::ModifyPanelVisibility(bool vis)
	{		
		QDockWidget *qdoc = dynamic_cast<QDockWidget*>(sender());
		if (!controller_panels_visibility_.contains(qdoc->windowTitle()))
			return;
		controller_panels_visibility_[qdoc->windowTitle()] = vis;
		//Find act
		foreach(menu_action_pair_ pair, all_actions_)
            if (!owner_->HasBeenUninitializaded())
                if (qdoc->windowTitle() == pair.second->text())
                    pair.second->setChecked(vis);
	}

    bool ExternalMenuManager::RemoveExternalMenuPanel(QWidget *controlled_widget)
    { 
		//The widget is the widget of RealXtend that is inside a dockwidget!
		foreach(menu_action_pair_ pair, all_actions_)
			if (controlled_widget->windowTitle() == pair.second->text())
			{
				QObject::disconnect(pair.second, SIGNAL(triggered()), this, SLOT(ActionNodeClicked()));
				controller_panels_visibility_.remove(controlled_widget->windowTitle());
				all_actions_.remove(all_actions_.key(pair));
				SAFE_DELETE(pair.second);
				//Check if any menu has 0 items, then delete it!
				QMutableMapIterator<QString, QMenu*> i(category_menu_);
				while (i.hasNext()) {
					i.next();
					if (i.value()->isEmpty()) {					
						SAFE_DELETE(i.value());
						category_menu_.remove(i.key());
					}
				}
				SortMenus();
				return true;
			}
		return false;
	}

	void ExternalMenuManager::SortMenus()
	{
		//Clean category_menu_ from rootMenu
		foreach(QMenu *menu, category_menu_)
			SAFE_DELETE(menu);
		//Create menus
		foreach(QString new_menu, all_menus_)
			category_menu_[new_menu] = root_menu_->addMenu(new_menu);
		//Add actions
		foreach(menu_action_pair_ pair, all_actions_)
			category_menu_[pair.first]->addAction(pair.second);
	}

	void ExternalMenuManager::ActionNodeClicked()
    {
		QAction *act = dynamic_cast<QAction*>(sender());

		QDockWidget  *aux = dynamic_cast<QDockWidget *>(act->parentWidget());

		if (controller_panels_visibility_[aux->objectName()]){
			aux->hide();
		}
		else
		{
			aux->show();
			QList<QDockWidget *> docks = dynamic_cast<QMainWindow *>(root_menu_->parentWidget())->tabifiedDockWidgets(aux);
			QDockWidget *value;
			foreach (value, docks)
				dynamic_cast<QMainWindow *>(root_menu_->parentWidget())->tabifyDockWidget(value, aux);
		}
	}

	void ExternalMenuManager::EnableMenus(){
		//Enable 
		QMutableMapIterator<QString, QMenu*> i(category_menu_);
		while (i.hasNext()) {
			i.next();
			if (i.key() != "File")
				i.value()->setEnabled(true);
		}
	}

	void ExternalMenuManager::DisableMenus(){
		//Disable 
		QMutableMapIterator<QString, QMenu*> i(category_menu_);
		while (i.hasNext()) {
			i.next();
			if (i.key() != "File")
				i.value()->setEnabled(false);
		}
	}
}
