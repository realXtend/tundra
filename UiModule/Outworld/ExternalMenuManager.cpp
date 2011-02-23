//$ HEADER_NEW_FILE $
// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "DebugOperatorNew.h"
#include "ExternalMenuManager.h"

#include <QDebug>

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
			category_menu_()
	{
        root_menu_->setVisible(true); // todo read from ini
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
		//Qicon TODO
		QMenu* menu = root_menu_->addMenu(name); 
		category_menu_[name] = menu;
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

		QString *aux = new QString(widget->windowTitle());
        controller_panels_[*aux] = widget;
		controller_actions_[*aux]= action;		
		connect(action, SIGNAL(triggered()), SLOT(ActionNodeClicked()));
		return true;
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

		QWidget  *aux = dynamic_cast<QWidget *>(act->parentWidget());
		if (aux->isHidden())
			aux->show();
		else
			aux->hide();
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

	void ExternalMenuManager::SceneChanged(const QString &old_name, const QString &new_name)
    {
        if (new_name == "Ether")
			DisableMenus();   			
		else
			EnableMenus();
    }
}
