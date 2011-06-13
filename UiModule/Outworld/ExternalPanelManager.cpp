//$ HEADER_NEW_FILE $
// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "DebugOperatorNew.h"
#include "UiWidget.h"

#include "ExternalPanelManager.h"

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

	QDockWidget* ExternalPanelManager::AddExternalPanel(UiWidget *widget, QString title, Qt::WindowFlags flags)
    {
        QDockWidget *wid = new QDockWidget(title, qWin_, flags);
		wid->setObjectName(title);
		wid->setWidget(widget);
        wid->setMinimumSize(widget->minimumSize());
        if (!AddQDockWidget(wid))
        {
            //to keep widget, removes widget parent and dock widget 
            QWidget *child_widget = wid->widget();
			wid->setWidget(0);
			child_widget ->setParent(0);
            SAFE_DELETE(wid);
            return 0;
        }
        
        connect(widget, SIGNAL(visibilityChanged(bool)), SLOT(ModifyPanelVisibility(bool)));
       
        return wid;
    }

	bool ExternalPanelManager::AddQDockWidget(QDockWidget *widget)
    {
		if (controller_panels_.contains(widget))
			return false;

        //Configure zones for the dockwidget
		qWin_->addDockWidget(Qt::LeftDockWidgetArea, widget, Qt::Vertical);
		widget->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea | Qt::BottomDockWidgetArea);
				
		widget->setFloating(true);
		widget->move(150,150);
		widget->hide();

        // Add to internal control list
        controller_panels_[widget] = widget->isVisible();
		
		//Restore estate if neccesary
		restoreWidget(widget);

        connect(widget, SIGNAL(visibilityChanged(bool)), SLOT(DockVisibilityChanged(bool)));

        return true;
    }

    void ExternalPanelManager::DockVisibilityChanged(bool vis)
	{		
		QDockWidget *qdoc = dynamic_cast<QDockWidget*>(sender());
		if (!controller_panels_.contains(qdoc))
			return;
        controller_panels_[qdoc] = vis;
        QWidget* titleBar = qdoc->titleBarWidget();
        if (titleBar)
        {
            QObjectList children = titleBar->children();
            foreach (QObject* object, children)
            {
                QString objectName = object->objectName();
            }
        }
        if (vis && !qdoc->widget()->isVisible())
            qdoc->widget()->setVisible(vis);
        else if (!vis && !qdoc->isVisible())
            qdoc->widget()->setVisible(vis);
        else if (!vis && qdoc->isVisible())
        {
            UiWidget *uiwidget = dynamic_cast<UiWidget*>(qdoc->widget());
            if (uiwidget)
                uiwidget->sendVisibilityChanged(vis);
        }
        else if (vis && qdoc->widget()->isVisible())
        {
            UiWidget *uiwidget = dynamic_cast<UiWidget*>(qdoc->widget());
            if (uiwidget)
                uiwidget->sendVisibilityChanged(vis);
        }
	}

    void ExternalPanelManager::ModifyPanelVisibility(bool vis)
    {
        UiWidget *uiwidget = dynamic_cast<UiWidget*>(sender());
        if (!uiwidget->parentWidget())
            return;
        QDockWidget *qdoc = dynamic_cast<QDockWidget *>(uiwidget->parentWidget());
        if (!qdoc)
            return;
		if (!controller_panels_.contains(qdoc))
			return;
        if ((vis && !controller_panels_[qdoc]) || (!vis && !uiwidget->isVisible() && !controller_panels_[qdoc] && qdoc->isVisible()))
        {
            qdoc->show();            
            QList<QDockWidget *> docks = dynamic_cast<QMainWindow *>(qWin_)->tabifiedDockWidgets(qdoc);
            if (docks.size() > 0)
            {
                QWidget* titleBar = qdoc->titleBarWidget();
                if (titleBar)
                {
                    QObjectList children = titleBar->children();
                    foreach (QObject* object, children)
                    {
                        QString objectName = object->objectName();
                    }
                }

            }

			QDockWidget *value;
			foreach (value, docks)
            {
				dynamic_cast<QMainWindow *>(qWin_)->tabifyDockWidget(value, qdoc);
                /*if (value->width() > qdoc->width())
                    qdoc->setFixedWidth(value->width());*/
            }
        }
        else if(!vis && controller_panels_[qdoc])
            qdoc->hide();
    }

	bool ExternalPanelManager::RemoveExternalPanel(QDockWidget *widget)
    {
		/*TODO: Check if it is in the menu bar also?*/
		//The widget is passed, no the QDockWidget!
        QDockWidget *doc_widget = dynamic_cast<QDockWidget*>(widget);
        if (!doc_widget)
        {
            return false;
        }

        if (controller_panels_.contains(doc_widget)) {
            controller_panels_.remove(doc_widget);
            qWin_->removeDockWidget(doc_widget);
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
    
	void ExternalPanelManager::EnableDockWidgets(){
		//Enable QDockWidgets
		QDockWidget *s;
        foreach( s , controller_panels_.keys() )
			s->widget()->setEnabled(true);
	}

	void ExternalPanelManager::DisableDockWidgets(){
		//Disable QDockWidgets
		QDockWidget *s;
		foreach( s , controller_panels_.keys() )
			s->widget()->setEnabled(false);
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
