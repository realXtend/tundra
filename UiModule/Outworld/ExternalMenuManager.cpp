//$ HEADER_NEW_FILE $
// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "DebugOperatorNew.h"
#include "ExternalMenuManager.h"

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
			controller_panels_(),
			controller_actions_(),
			controller_panels_visibility_(),
			category_menu_()
	{
        root_menu_->setVisible(false); // todo read from ini
    }

    ExternalMenuManager::~ExternalMenuManager()
    {
        SAFE_DELETE(root_menu_);
    }

	void ExternalMenuManager::AddMenu(const QString &name, const QString &icon)
    {
		//Creates a new Menu, if it doesnt exist already
		if (!root_menu_)
			return;

		if (category_menu_.contains(name))
			return;

		if (!root_menu_->isVisible())
			root_menu_->setVisible(true);
		//Qicon TODO
		QMenu* menu = root_menu_->addMenu(name); 
		category_menu_[name] = menu;
    }

	bool ExternalMenuManager::AddExternalMenu(QMenu *new_menu, const QString &menu, const QString &icon)
	{
		if (menu.isEmpty())  
        {
			AddMenu("Others");
            category_menu_["Others"]->addMenu(new_menu);
        }
        else if (category_menu_.contains(menu))
        {
            category_menu_[menu]->addMenu(new_menu);
        }
        else
        {
            AddMenu(menu);
            category_menu_[menu]->addMenu(new_menu);
        }
		if (!category_menu_.contains(new_menu->title()))
			category_menu_[new_menu->title()] = new_menu;
		return true;	
	}

	bool ExternalMenuManager::AddExternalMenuAction(QAction *action, const QString &name, const QString &menu, const QString &icon)
    {
		if (menu.isEmpty())  
        {
			AddMenu("Others");
            category_menu_["Others"]->addAction(action);
        }
        else if (category_menu_.contains(menu))
        {
            category_menu_[menu]->addAction(action);
        }
        else
        {
            AddMenu(menu);
            category_menu_[menu]->addAction(action);
        }
		action->setCheckable(true);
		QString *aux = new QString(name+menu);
        controller_actions_[*aux] = action;
		return true;	
	}

	bool ExternalMenuManager::AddExternalMenuPanel(QWidget *widget, const QString &name, const QString &menu)
    {       
		//Check if is not already
		if (controller_panels_.contains(QString(menu+"+"+widget->windowTitle())))
			return false;
		QAction *action = new QAction(name, widget);
		action->setCheckable(true);
		if (menu.isEmpty())		
        {
			AddMenu("Others");
			category_menu_["Others"]->addAction(action);
        }
        else if (category_menu_.contains(menu))
        {
            category_menu_[menu]->addAction(action);
        }
        else
        {
            AddMenu(menu);
            category_menu_[menu]->addAction(action);
        }

		//QString *aux = new QString(widget->windowTitle());
        controller_panels_[widget->windowTitle()] = widget;
		controller_actions_[widget->windowTitle()]= action;
		controller_panels_visibility_[widget->windowTitle()] = dynamic_cast<QDockWidget *>(widget)->isVisible();
		connect(action, SIGNAL(triggered()), SLOT(ActionNodeClicked()));
		connect(widget, SIGNAL(visibilityChanged(bool)), SLOT(ModifyPanelVisibility(bool)));
		return true;
    }

	void ExternalMenuManager::ModifyPanelVisibility(bool vis)
	{
		QDockWidget *qdoc = dynamic_cast<QDockWidget*>(sender());
		if (!controller_panels_visibility_.contains(qdoc->objectName()))
			return;
		controller_panels_visibility_[qdoc->objectName()] = vis;
		QAction* act= controller_actions_.value(qdoc->windowTitle());
		act->setChecked(vis);
	}

    bool ExternalMenuManager::RemoveExternalMenuPanel(QWidget *controlled_widget)
    { 
		//The widget is the widget of RealXtend that is inside a dockwidget!
		QString del = controller_panels_.key(controlled_widget->parentWidget());
		if (controller_actions_.contains(del)){
			//Begin_mod
			QAction *adel = controller_actions_[del];
			QObject::disconnect(adel, SIGNAL(triggered()), this, SLOT(ActionNodeClicked()));
			//Delete action from menu..
			SAFE_DELETE(adel);
			//Check if any menu has 0 items, then delete it!
			QMutableMapIterator<QString, QMenu*> i(category_menu_);
			while (i.hasNext()) {
				i.next();
				if (i.value()->isEmpty()) {					
					SAFE_DELETE(i.value());
					category_menu_.remove(i.key());
				}
			}
			return true;
		}
		return false;
    }

	void ExternalMenuManager::ActionNodeClicked()
    {
		QAction *act = dynamic_cast<QAction*>(sender());

		QDockWidget  *aux = dynamic_cast<QDockWidget *>(act->parentWidget());

		if (controller_panels_visibility_[aux->objectName()]){
			aux->hide();
			//act->setChecked(false);
		}
		else
		{
			aux->show();
			//act->setChecked(true);
			QList<QDockWidget *> docks = dynamic_cast<QMainWindow *>(root_menu_->parentWidget())->tabifiedDockWidgets(aux);
			QDockWidget *value;
			foreach (value, docks)
				dynamic_cast<QMainWindow *>(root_menu_->parentWidget())->tabifyDockWidget(value, aux);
		}

		/*
		if (aux->isHidden())
			aux->show();
		else
			aux->hide();
			*/
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
