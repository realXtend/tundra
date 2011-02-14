//$ HEADER_NEW_FILE $ 
#include "StableHeaders.h"

#include "EventManager.h"
#include "ServiceManager.h"
#include "SceneEvents.h"
#include "InputEvents.h"
#include "UiExternalServiceInterface.h"
#include "StaticToolBar.h"
#include "UiServiceInterface.h"

namespace UiExternalServices
{

	StaticToolBar::StaticToolBar(const QString &title, QWidget *parent, Foundation::Framework* framework): 
		QToolBar(title,parent), 
		flyAction_(0),
		cameraAction_(0),
		editAction_(0),
		scene_event_category_(0),
		currentScene(""),
		framework_(framework)
	{
        bool create_actions = false; // todo read from ini
        if (create_actions)
        {
            createActions();
		    addActions();
        }
//		CreateMenu();
	}

	StaticToolBar::~StaticToolBar()
	{
	}

	void StaticToolBar::createActions()
	{
		flyAction_ = new QAction(QIcon("./media/icons/fly.png"),tr("&Fly"), this);
		flyAction_->setStatusTip(tr("Set fly mode"));
		connect(flyAction_, SIGNAL(triggered()), this, SLOT(flyMode()));

		cameraAction_ = new QAction(QIcon("./media/icons/camera.png"),tr("&Free Camera"), this);
		cameraAction_->setStatusTip(tr("Set free camera mode"));
		connect(cameraAction_, SIGNAL(triggered()), this, SLOT(freeCameraMode()));

		editAction_ = new QAction(QIcon("./media/icons/build.png"),tr("&Edit Mode"), this);
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
		framework_->GetEventManager()->SendEvent(event_category, InputEvents::TOGGLE_FLYMODE, 0);
	}

	void StaticToolBar::freeCameraMode()
	{
		event_category_id_t event_category = framework_->GetEventManager()->QueryEventCategory("Input");
		framework_->GetEventManager()->SendEvent(event_category, InputEvents::SWITCH_CAMERA_STATE, 0);
	}

	void StaticToolBar::editMode()
	{
		Foundation::UiExternalServiceInterface *uiExternal = framework_->GetService<Foundation::UiExternalServiceInterface>();
		uiExternal->SetEnableEditMode(!uiExternal->IsEditModeEnable());
	}


}
