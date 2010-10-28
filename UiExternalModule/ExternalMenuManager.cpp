//$ HEADER_NEW_FILE $
// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "DebugOperatorNew.h"
#include "ExternalMenuManager.h"

#include <QDebug>

#include "MemoryLeakCheck.h"

namespace UiExternalServices
{
    QString ExternalMenuManager::defaultItemIcon = "./data/ui/images/menus/edbutton_MATWIZ_normal.png";
    QString ExternalMenuManager::defaultGroupIcon = "./data/ui/images/menus/edbutton_WRLDTOOLS_icon.png";

    ExternalMenuManager::ExternalMenuManager(QMenuBar *parent, UiExternalModule *owner) :
            root_menu_(parent),
			owner_(owner)
    {
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


	bool ExternalMenuManager::AddExternalMenuAction(QAction *action, const QString &name, const QString &menu, const QString &icon){
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

	bool ExternalMenuManager::AddExternalMenuPanel(QWidget *widget, const QString &name, const QString &menu, bool moveable)
    {       
		//Check if is not already
		if (controller_panels_.contains(QString(menu+"+"+name)))
			return false;
		
		//First of all, we create the Menu with 2options, internal and external
		QMenu* newmenu = new QMenu(name, widget);
		QAction *action1 = new QAction("In scene", dynamic_cast<QDockWidget*>(widget)/*->widget()*/);
		QAction *action2 = new QAction("Outside", dynamic_cast<QDockWidget*>(widget)/*->widget()*/);
		//To avoid put the in scene action when it is not available to move
		if (moveable)
			newmenu->addAction(action1);
		newmenu->addAction(action2);

        if (menu.isEmpty())		
        {
			AddMenu("Others");
			category_menu_["Others"]->addMenu(newmenu);
        }
        else if (category_menu_.contains(menu))
        {
            category_menu_[menu]->addMenu(newmenu);
        }
        else
        {
            AddMenu(menu);
            category_menu_[menu]->addMenu(newmenu);
        }
		
		QString *aux = new QString(menu+"+"+name);
        controller_panels_[*aux] = widget;
		controller_actions_[QString(menu+"+"+name+"in")]= action1;
		controller_actions_[QString(menu+"+"+name+"out")]= action2;
		connect(action1, SIGNAL(triggered()), SLOT(ActionNodeClickedInside()));
		connect(action2, SIGNAL(triggered()), SLOT(ActionNodeClickedOutside()));
		return true;
    }

    bool ExternalMenuManager::RemoveExternalMenuPanel(QWidget *controlled_widget)
    { 
		//TODO DISCONNECT PROPERLY!!
		//The widget is the widget of RealXtend that is inside a dockwidget!
		QString del = controller_panels_.key(dynamic_cast<QDockWidget*>(controlled_widget->parentWidget()));
		if (controller_actions_.contains(del)){
			QAction *adel = controller_actions_[del+"in"];
			QAction *adel2 = controller_actions_[del+"out"];
			//disconnect action
			QObject::disconnect(adel, SIGNAL(triggered()), this, SLOT(ActionNodeClickedInside()));
			QObject::disconnect(adel2, SIGNAL(triggered()), this, SLOT(ActionNodeClickedOutside()));
			//Get menu name and delete action from menu
			QStringList menu = del.split("+");
			if (category_menu_.contains(menu.at(0))) {
				SAFE_DELETE(category_menu_[menu.at(0)]);
				return true;
			}
		}
		return false;
    }

    void ExternalMenuManager::ActionNodeClickedInside()
    {
		QAction *act = dynamic_cast<QAction*>(sender());

		QDockWidget *aux = dynamic_cast<QDockWidget *>(act->parentWidget());
		QWidget *window = aux->widget();

		//Check where it is
		if (window){
			//is outside			
			//Change to inside mode and show it
			UiServiceInterface *ui = owner_->GetFramework()->GetService<UiServiceInterface>();
			if (ui){
				ui->TransferWidgetInOut(window->windowTitle());
				//Not neccesary?? ui->ShowWidget(window);
			}
		} else {
			//Is inside, just show or hide it
			UiServiceInterface *ui = owner_->GetFramework()->GetService<UiServiceInterface>();
			if (ui)			
				ui->BringWidgetToFront(aux->windowTitle()); //We cant because we dont have it!!
		}
    }

	void ExternalMenuManager::ActionNodeClickedOutside()
    {
		QAction *act = dynamic_cast<QAction*>(sender());

		QDockWidget  *aux = dynamic_cast<QDockWidget *>(act->parentWidget());

		if (aux->widget()){
			//Is outside, just show/hide it
			//Hide or show the widget; we make every widget a qdockwidget before integrate it in the qmainwindow
			if (aux->isHidden())
				aux->show();
			else
				aux->hide();
		} else {
			//Is inside, remove it and put it outside
			UiServiceInterface *ui = owner_->GetFramework()->GetService<UiServiceInterface>();
			if (ui)
				ui->TransferWidgetInOut(aux->windowTitle());
		}
	}

	void ExternalMenuManager::EnableMenus(){
		//Enable 
		QMenu * aux = category_menu_.value("Panels");
		aux->setEnabled(true);
	}

	void ExternalMenuManager::DisableMenus(){
		//Disable 
		QMenu * aux = category_menu_.value("Panels");
		aux->setEnabled(false);
	}

	void ExternalMenuManager::SceneChanged(const QString &old_name, const QString &new_name)
    {
        if (new_name == "Ether")
			DisableMenus();     
		else
			EnableMenus();
    }
}
