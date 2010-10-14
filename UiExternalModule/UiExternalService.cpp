//$ HEADER_NEW_FILE $
#include "StableHeaders.h"
#include "UiExternalService.h"


namespace UiExternalServices
{
	//
	UiExternalService::UiExternalService(UiExternalModule *owner):
	   owner_(owner)
	   {		   
	   }

    // Destructor.
	UiExternalService:: ~UiExternalService(){
	}

	QWidget* UiExternalService::AddExternalPanel(QWidget *widget, QString title, Qt::WindowFlags flags) {
		return owner_->GetExternalPanelManager()->AddExternalPanel(widget, title, flags);
	}

	bool UiExternalService::RemoveExternalPanel(QWidget *widget){
		return owner_->GetExternalPanelManager()->RemoveExternalPanel(widget);
	}

	bool UiExternalService::AddExternalMenuPanel(QWidget *widget, const QString &name, const QString &menu, const QString &icon){
		return owner_->GetExternalMenuManager()->AddExternalMenuPanel(widget, name, menu, icon);
	}

	bool UiExternalService::AddExternalMenuAction(QAction *action, const QString &name, const QString &menu, const QString &icon){
		return owner_->GetExternalMenuManager()->AddExternalMenuAction(action, name, menu, icon);
	}

	bool UiExternalService::RemoveExternalMenuPanel(QWidget *widget){
		return owner_->GetExternalMenuManager()->RemoveExternalMenuPanel(widget);
	}

	void UiExternalService::ShowWidget(QWidget *widget){
		//find qdockwidget with this widget and make show()
		owner_->GetExternalPanelManager()->ShowWidget(widget);	
	}

	void UiExternalService::HideWidget(QWidget *widget){
		//find qdockwidget with this widget and make hide()	
		owner_->GetExternalPanelManager()->HideWidget(widget);
	}
}
