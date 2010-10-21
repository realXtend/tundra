//$ HEADER_NEW_FILE $ 
#include "StableHeaders.h"

#include "EventManager.h"
#include "ServiceManager.h"
#include "SceneEvents.h"
#include "InputEvents.h"
#include "UiExternalServiceInterface.h"
#include "UiServiceInterface.h"
#include "WorldBuildingServiceInterface.h"
#include "MainWindow.h"
#include "Entity.h"
#include "EC_Highlight.h"
#include "StaticToolBar.h"

namespace UiExternalServices
{

	StaticToolBar::StaticToolBar(const QString &title, QWidget *parent, Foundation::Framework* framework): 
		QToolBar(title,parent), 
		framework_(framework),
		entitySelected_(0)
	{
		createActions();
		addActions();
		CreateMenu();
		scene_event_category_ = framework_->GetEventManager()->QueryEventCategory("Scene");
	}

	StaticToolBar::~StaticToolBar()
	{
	}

	void StaticToolBar::createActions()
	{
		flyAction_ = new QAction(tr("&Fly"), this);
		flyAction_->setStatusTip(tr("Set fly mode"));
		connect(flyAction_, SIGNAL(triggered()), this, SLOT(flyMode()));

		cameraAction_ = new QAction(tr("&Free Camera"), this);
		cameraAction_->setStatusTip(tr("Set free camera mode"));
		connect(cameraAction_, SIGNAL(triggered()), this, SLOT(freeCameraMode()));

		editAction_ = new QAction(tr("&Edit Mode"), this);
		editAction_->setStatusTip(tr("Set edit mode to modify entity properties"));
		connect(editAction_, SIGNAL(triggered()), this, SLOT(editMode()));
	}

	void StaticToolBar::addActions()
	{
		addAction(flyAction_);
		addAction(cameraAction_);
		addAction(editAction_);
	}

	void StaticToolBar::flyMode()
	{
		event_category_id_t event_category = framework_->GetEventManager()->QueryEventCategory("Input");
		framework_->GetEventManager()->SendEvent(event_category, Input::Events::TOGGLE_FLYMODE, 0);
	}

	void StaticToolBar::freeCameraMode()
	{
		event_category_id_t event_category = framework_->GetEventManager()->QueryEventCategory("Input");
		framework_->GetEventManager()->SendEvent(event_category, Input::Events::SWITCH_CAMERA_STATE, 0);
	}

	void StaticToolBar::editMode()
	{
		Foundation::UiExternalServiceInterface *uiExternal = framework_->GetService<Foundation::UiExternalServiceInterface>();
		uiExternal->SetEnableEditMode(!uiExternal->IsEditModeEnable());
	}

	void StaticToolBar::openECEditor()
	{
		Foundation::UiExternalServiceInterface *uiExternal = framework_->GetService<Foundation::UiExternalServiceInterface>();
		uiExternal->ShowWidget(uiExternal->GetExternalMenuPanel(QString("Entity-component Editor"))->widget());
	}

	void StaticToolBar::openBuild()
	{
		UiServiceInterface *uiService = framework_->GetService<UiServiceInterface>();
		uiService->SwitchToScene("WorldBuilding");
	}

	void StaticToolBar::HandleEvent(event_category_id_t category_id, event_id_t event_id, IEventData* data)
    {
       if (category_id == scene_event_category_)
       {
		    menu_asset->close();
			switch(event_id)
            {
			case Scene::Events::EVENT_ENTITY_CLICKED:
            {
				Scene::Events::EntityClickedData *entity_clicked_data = dynamic_cast<Scene::Events::EntityClickedData *>(data);
				Foundation::UiExternalServiceInterface *uiExternal = framework_->GetService<Foundation::UiExternalServiceInterface>();
				if (entity_clicked_data && uiExternal->IsEditModeEnable()){
					
					if (entitySelected_ && currentScene!="WorldBuilding"){
						menu_asset->exec(framework_->GetMainWindow()->cursor().pos());
					}    
					else if(!entitySelected_){
						entitySelected_=entity_clicked_data->entity;
						EC_Highlight *luz = checked_static_cast<EC_Highlight*>(entitySelected_->GetOrCreateComponent("EC_Highlight","editMode",AttributeChange::LocalOnly,true).get());
						luz->Show();
					}
				}
				break;
            }
   		  case Scene::Events::EVENT_ENTITY_NONE_CLICKED:
            {
				if(entitySelected_)
					entitySelected_->RemoveComponent(entitySelected_->GetComponent("EC_Highlight","editMode"));
               	entitySelected_=0;
				break;
            }
            default:
                break;
            }
		}
	}

	void StaticToolBar::Enabled(){
		setVisible(true);
	}

	void StaticToolBar::Disabled(){
		setVisible(false);
	}

	void StaticToolBar::SceneChanged(const QString &old_name, const QString &new_name)
    {
		currentScene=new_name;
        if (currentScene == "Ether")
			Disabled();     
		else
			Enabled();
    }

	void StaticToolBar::CreateMenu()
	{
		menu_asset = new QMenu(dynamic_cast<QMainWindow *>(parentWidget())->centralWidget());

		QAction *ec_action = new QAction("Entiy Component",menu_asset);
		connect(ec_action, SIGNAL(triggered()), this, SLOT(openECEditor()));
		menu_asset->addAction(ec_action);

		QAction *build_action = new QAction("Properties",menu_asset);
		connect(build_action, SIGNAL(triggered()), this, SLOT(openBuild()));
		menu_asset->addAction(build_action);
	}
}