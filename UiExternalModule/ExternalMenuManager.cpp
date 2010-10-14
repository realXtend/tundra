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

    ExternalMenuManager::ExternalMenuManager(QMenuBar *parent) :
            root_menu_(parent)
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

	bool ExternalMenuManager::AddExternalMenuPanel(QWidget *widget, const QString &name, const QString &menu, const QString &icon)
    {
        QAction *action = new QAction(QIcon("./data/ui/images/menus/"+icon), name, widget);
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
		
		QString *aux = new QString(name+"+"+menu);
        controller_panels_[*aux] = widget;
		controller_actions_[*aux]= action;
		connect(action, SIGNAL(triggered()), SLOT(ActionNodeClicked()));
		return true;
    }

    bool ExternalMenuManager::RemoveExternalMenuPanel(QWidget *controlled_widget)
    { 
		/*QString del = controller_panels_.key(controlled_widget);
		//if (del){
			QAction *adel = controller_actions_[del];
			//disconnect action
			QObject::disconnect(adel, SIGNAL(triggered()), this, SLOT(ActionNodeClicked()));
			//Delete action from menu
			if (category_menu_.contains(del))
				category_menu_[del]->removeAction(adel);
			return true;
		//}*/
		return true;
    }

    void ExternalMenuManager::ActionNodeClicked()
    {
		QAction *act = dynamic_cast<QAction*>(sender());

		QDockWidget *aux = dynamic_cast<QDockWidget *>(act->parentWidget());

		if (!aux)
            return;

		//Hide or show the widget; we make every widget a qdockwidget before integrate it in the qmainwindow
		if (aux->isHidden())
			aux->show();
		else
			aux->hide();

		//To test it
		RemoveExternalMenuPanel(aux->widget());

    }
}
