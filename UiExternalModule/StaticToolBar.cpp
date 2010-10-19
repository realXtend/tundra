//$ HEADER_NEW_FILE $ 
#include "StableHeaders.h"

#include "EventManager.h"
#include "ServiceManager.h"
#include "SceneEvents.h"
#include "InputEvents.h"
#include "UiExternalServiceInterface.h"

#include "StaticToolBar.h"

namespace UiExternalServices
{

	StaticToolBar::StaticToolBar(const QString &title, QWidget *parent, Foundation::Framework* framework): 
		QToolBar(title,parent), 
		framework_(framework)
	{
		createActions();
		addActions();

		// scene_event_category_ = framework_->GetEventManager()->QueryEventCategory("Scene");
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

	 void StaticToolBar::HandleEvent(event_category_id_t category_id, event_id_t event_id, IEventData* data)
    {
   //     if (category_id == scene_event_category_)
   //     {
   //         switch(event_id)
   //         {
   //         case Scene::Events::EVENT_ENTITY_CLICKED:
   //         {
   //             //! \todo support multiple entity selection
   //             Scene::Events::EntityClickedData *entity_clicked_data = dynamic_cast<Scene::Events::EntityClickedData *>(data);
   //             if (editMode_ && entity_clicked_data)
   //                 entitySelected_=entity_clicked_data->entity->GetId();
   //             break;
   //         }
   //		  case Scene::Events::EVENT_ENTITY_NONE_CLICKED:
   //         {
   //			  entitySelected_=0;
   //             break;
   //         }
   //         default:
   //             break;
   //         }
   //     }
	 }
}