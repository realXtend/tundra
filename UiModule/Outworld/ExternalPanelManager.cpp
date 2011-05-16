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
            //to keep widget, removes widget parent and dock widget 
            QWidget *child_widget = wid->widget();
			wid->setWidget(0);
			child_widget ->setParent(0);
            SAFE_DELETE(wid);
            return false;
        }        
		connect(wid,SIGNAL(visibilityChanged(bool)),widget,SLOT(show()));
        return wid;
    }

	bool ExternalPanelManager::AddQDockWidget(QDockWidget *widget)
    {
		if (all_qdockwidgets_in_window_.contains(widget))
			return false;

        //Configure zones for the dockwidget
		//if (owner_->HasBeenPostinitializaded())
		//	qWin_->addDockWidget(Qt::NoDockWidgetArea, widget, Qt::Vertical);
		//else
		qWin_->addDockWidget(Qt::LeftDockWidgetArea, widget, Qt::Vertical);
		widget->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea | Qt::BottomDockWidgetArea);
				
		widget->setFloating(true);
		widget->move(150,150);
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
        QDockWidget *doc_widget = dynamic_cast<QDockWidget*>(widget);
        if (!doc_widget)
        {
            doc_widget = dynamic_cast<QDockWidget*>(widget->parentWidget());    
            if (!doc_widget)
                return false;
        }

        if (all_qdockwidgets_in_window_.contains(doc_widget)) {
            qWin_->removeDockWidget(doc_widget);
		    all_qdockwidgets_in_window_.removeOne(doc_widget);      
            //to keep widget, removes widget parent and dock widget 

			QWidget *child_widget = doc_widget->widget();
			doc_widget->setWidget(0);
			child_widget ->setParent(0);

            SAFE_DELETE(doc_widget);
            return true;
        }
        else
			return false;	
    }
    
	void ExternalPanelManager::ShowWidget(QWidget *widget){
		if (all_qdockwidgets_in_window_.contains(dynamic_cast<QDockWidget*>(widget))){
			QDockWidget *aux = dynamic_cast<QDockWidget *>(widget->parentWidget());
			if (aux)
                aux->show();
            else
                widget->show();
			//	return;
			////Hide or show the widget; we make every widget a qdockwidget before integrate it in the qmainwindow
			//if (aux->isHidden())
			//	aux->show();
			//widget->parentWidget()->show();	
			//dynamic_cast<QDockWidget*>(widget)->widget()->show();
            //widget->parentWidget()->show();
			//widget->show();
		}
	}

	void ExternalPanelManager::HideWidget(QWidget *widget){
		if (all_qdockwidgets_in_window_.contains(dynamic_cast<QDockWidget*>(widget->parentWidget()))){
            //widget->parentWidget()->hide();
			//widget->hide();
            QDockWidget *aux = dynamic_cast<QDockWidget *>(widget->parentWidget());
			if (aux)
                aux->hide();
            else
                widget->hide();
		}
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
