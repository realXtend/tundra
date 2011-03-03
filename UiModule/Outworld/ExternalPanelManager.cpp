//$ HEADER_NEW_FILE $
// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#include "ExternalPanelManager.h"
#include "ExternalMenuManager.h"

#include "MemoryLeakCheck.h"

namespace UiServices
{
	ExternalPanelManager::ExternalPanelManager(QMainWindow *qWin, UiModule *owner):
        qWin_(qWin),
		owner_(owner)
    {
        assert(qWin_);
		//Define QDockAreas allowed, or configuration staff
    }

    ExternalPanelManager::~ExternalPanelManager()
    {
    }

	QDockWidget* ExternalPanelManager::AddExternalPanel(QWidget *widget, QString title, Qt::WindowFlags flags)
    {
        QDockWidget *wid = new QDockWidget(title, qWin_, flags);
		wid->setObjectName(title);
		wid->setWidget(widget);
        if (!AddQDockWidget(wid))
        {
            SAFE_DELETE(wid);
            return false;
        }        
        return wid;
    }

	bool ExternalPanelManager::AddQDockWidget(QDockWidget *widget)
    {
		if (all_qdockwidgets_in_window_.contains(widget))
			return false;

        //Configure zones for the dockwidget		
		qWin_->addDockWidget(Qt::LeftDockWidgetArea, widget, Qt::Vertical);
		widget->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea | Qt::BottomDockWidgetArea);
				
		widget->setFloating(true);
		widget->hide();

        // Add to internal control list
        all_qdockwidgets_in_window_.append(widget);
		
		//Restore estate if neccesary
		restoreWidget(widget);

        return true;
    }

	bool ExternalPanelManager::RemoveExternalPanel(QWidget *widget)
    {
		/*TODO: Check if it is in the menu bar also?*/
		//The widget is passed, no the QDockWidget!
		if (all_qdockwidgets_in_window_.contains(dynamic_cast<QDockWidget*>(widget->parentWidget()))) {
			qWin_->removeDockWidget(dynamic_cast<QDockWidget*>(widget->parentWidget()));
			all_qdockwidgets_in_window_.removeOne(dynamic_cast<QDockWidget*>(widget->parentWidget()));
			return true;
		}
		else if (all_qdockwidgets_in_window_.contains(dynamic_cast<QDockWidget*>(widget))){
			//Someone has called the method with the dockwidget!
			qWin_->removeDockWidget(dynamic_cast<QDockWidget*>(widget));
			all_qdockwidgets_in_window_.removeOne(dynamic_cast<QDockWidget*>(widget));
			return true;
		}
		else
			return false;		
    }
    
	void ExternalPanelManager::ShowWidget(QWidget *widget){
		if (all_qdockwidgets_in_window_.contains(dynamic_cast<QDockWidget*>(widget))){
			//QDockWidget *aux = dynamic_cast<QDockWidget *>(widget->parentWidget());
			//if (!aux)
			//	return;
			////Hide or show the widget; we make every widget a qdockwidget before integrate it in the qmainwindow
			//if (aux->isHidden())
			//	aux->show();
			//widget->parentWidget()->show();	
			widget->show();
		}
	}

	void ExternalPanelManager::HideWidget(QWidget *widget){
		if (all_qdockwidgets_in_window_.contains(dynamic_cast<QDockWidget*>(widget->parentWidget())))
			widget->parentWidget()->hide();		
	}

	void ExternalPanelManager::EnableDockWidgets(){
		//Enable QDockWidgets
		QDockWidget *s;
		foreach( s , all_qdockwidgets_in_window_ )
			s->widget()->setEnabled(true);
	}

	void ExternalPanelManager::DisableDockWidgets(){
		//Disable QDockWidgets
		QDockWidget *s;
		foreach( s , all_qdockwidgets_in_window_ )
			s->widget()->setEnabled(false);
	}

	void ExternalPanelManager::SceneChanged(const QString &old_name, const QString &new_name)
    {
        if (new_name == "Ether")
			DisableDockWidgets();  
		else if(old_name == "Ether")
			EnableDockWidgets();
    }

	QDockWidget* ExternalPanelManager::GetExternalMenuPanel(QString &widget){
		QDockWidget *s;
		foreach( s , all_qdockwidgets_in_window_ )
			if (s->windowTitle() == widget)
				return s;
		return false;

	}

	void ExternalPanelManager::restoreWidget(QDockWidget *widget) {
		//Check if we have to restore it
		if (owner_->HasBeenPostinitializaded()) {
#if QT_VERSION > 0x040700
			//QtBug: if version <4.7 the restore of the widget doesnt work properly. It's put in front of the application
			qWin_->restoreDockWidget(widget);
#endif
		}
			
	}
}
