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

	bool UiExternalService::AddExternalPanel(QDockWidget *widget) {
		return owner_->GetExternalPanelManager()->AddQDockWidget(widget);
	}

	bool UiExternalService::RemoveExternalPanel(QWidget *widget){
		return owner_->GetExternalPanelManager()->RemoveExternalPanel(widget);
	}

	bool UiExternalService::AddExternalMenuPanel(QWidget *widget, const QString &name, const QString &menu, bool moveable){
		return owner_->GetExternalMenuManager()->AddExternalMenuPanel(widget, name, menu, moveable);
	}

	bool UiExternalService::AddExternalMenuAction(QAction *action, const QString &name, const QString &menu, const QString &icon){
		return owner_->GetExternalMenuManager()->AddExternalMenuAction(action, name, menu, icon);
	}

	bool UiExternalService::RemoveExternalMenuPanel(QWidget *widget){
		return owner_->GetExternalMenuManager()->RemoveExternalMenuPanel(widget);
	}

	QDockWidget* UiExternalService::GetExternalMenuPanel(QString &widget){
		return owner_->GetExternalPanelManager()->GetExternalMenuPanel(widget);
	}

	void UiExternalService::ShowWidget(QWidget *widget){
		//find qdockwidget with this widget and make show()
		owner_->GetExternalPanelManager()->ShowWidget(widget);	
	}

	void UiExternalService::HideWidget(QWidget *widget){
		//find qdockwidget with this widget and make hide()	
		owner_->GetExternalPanelManager()->HideWidget(widget);
	}

	void UiExternalService::SetEnableEditMode(bool b){
		owner_->GetExternalPanelManager()->SetEnableEditMode(b);
	}

	void UiExternalService::AddPanelToEditMode(QWidget* widget){
		owner_->GetExternalPanelManager()->AddToEditMode(widget);
	}

	bool UiExternalService::IsEditModeEnable(){
		return owner_->GetExternalPanelManager()->IsEditModeEnable();
	}

	bool UiExternalService::AddExternalToolbar(QToolBar *toolbar, const QString &name){
		return owner_->GetExternalToolBarManager()->AddExternalToolbar(toolbar, name);	
	}

	bool UiExternalService::RemoveExternalToolbar(QString name){
		return owner_->GetExternalToolBarManager()->RemoveExternalToolbar(name);
	}
	bool UiExternalService::ShowExternalToolbar(QString name){
		return owner_->GetExternalToolBarManager()->ShowExternalToolbar(name);
	}
	bool UiExternalService::HideExternalToolbar(QString name){
		return owner_->GetExternalToolBarManager()->HideExternalToolbar(name);
	}
	bool UiExternalService::EnableExternalToolbar(QString name){
		return owner_->GetExternalToolBarManager()->EnableExternalToolbar(name);
	}
	bool UiExternalService::DisableExternalToolbar(QString name){
		return owner_->GetExternalToolBarManager()->DisableExternalToolbar(name);
	}
	QToolBar* UiExternalService::GetExternalToolbar(QString name){
		return owner_->GetExternalToolBarManager()->GetExternalToolbar(name);
	}
}
