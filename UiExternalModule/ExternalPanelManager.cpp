//$ HEADER_NEW_FILE $
// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#include "ExternalPanelManager.h"
#include "ExternalMenuManager.h"
#include "UiExternalServiceInterface.h"

#include "MemoryLeakCheck.h"

namespace UiExternalServices
{
	ExternalPanelManager::ExternalPanelManager(QMainWindow *qWin):
          qWin_(qWin),
		  edit_mode_(false)
    {
        assert(qWin_);
		//Define QDockAreas allowed, or configuration staff
    }

    ExternalPanelManager::~ExternalPanelManager()
    {
    }

	QWidget* ExternalPanelManager::AddExternalPanel(QWidget *widget, QString title, Qt::WindowFlags flags)
    {
        QDockWidget *wid = new QDockWidget(title, qWin_, flags);
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
		//TODO Check if it is in menuBar???


        //Configure zones for the dockwidget
		qWin_->addDockWidget(Qt::LeftDockWidgetArea, widget, Qt::Vertical);
		widget->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea | Qt::BottomDockWidgetArea);
		widget->setFloating(false);
		widget->hide();

        // Add to internal control list
        all_qdockwidgets_in_window_.append(widget);
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
		if (all_qdockwidgets_in_window_.contains(dynamic_cast<QDockWidget*>(widget->parentWidget())))
			widget->parentWidget()->show();	
	}

	void ExternalPanelManager::HideWidget(QWidget *widget){
		if (all_qdockwidgets_in_window_.contains(dynamic_cast<QDockWidget*>(widget->parentWidget())))
			widget->parentWidget()->hide();		
	}

	void ExternalPanelManager::SetEnableEditMode(bool b){
		edit_mode_=b;
		emit changeEditMode(edit_mode_);
	}

	void ExternalPanelManager::AddToEditMode(QWidget* widget){
		if (all_qdockwidgets_in_window_.contains(dynamic_cast<QDockWidget*>(widget->parentWidget())))
			connect(this,SIGNAL(changeEditMode(bool)),widget,SLOT(setEnabled(bool)));
	}

	bool ExternalPanelManager::IsEditModeEnable(){
		return edit_mode_;
	}
}
