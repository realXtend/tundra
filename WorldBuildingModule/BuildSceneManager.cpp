// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "ModuleInterface.h"
#include "BuildSceneManager.h"

#include <UiModule.h>
#include <EC_OpenSimPrim.h>

#include "BuildScene.h"
#include "AnchorLayout.h"
#include "PropertyEditorHandler.h"

#include <QPixmap>

namespace WorldBuilding
{
    BuildSceneManager::BuildSceneManager(QObject *parent, Foundation::Framework *framework) :
        framework_(framework),
        inworld_state(false),
        scene_name_("WorldBuilding"),
        property_editor_handler_(0),
        prim_selected_(true)
    {
        setParent(parent);
        InitialseScene();
        ObjectSelected(false);
    }

    BuildSceneManager::~BuildSceneManager()
    {
        SAFE_DELETE(object_info_widget_);
        SAFE_DELETE(object_manipulations_widget_);
    }

    void BuildSceneManager::InitialseScene()
    {
        scene_ = new BuildScene(this);
        layout_ = new AnchorLayout(this, scene_);

        // Register scene to ui module
        StateMachine *machine = GetStateMachine();
        if (machine)
            machine->RegisterScene(scene_name_, scene_);

        // Init info widget
        object_info_widget_ = new Ui::BuildingWidget(Ui::BuildingWidget::Right);
        object_info_ui.setupUi(object_info_widget_->GetInternal());
        
        QPixmap debug(object_info_ui.object_viewport->size());
        debug.fill(Qt::gray);
        object_info_ui.object_viewport->setPixmap(debug);

        layout_->AddCornerAnchor(object_info_widget_, Qt::TopRightCorner, Qt::TopRightCorner);
        layout_->AddCornerAnchor(object_info_widget_, Qt::BottomRightCorner, Qt::BottomRightCorner);

        object_info_widget_->PrepWidget();
        connect(scene_, SIGNAL(sceneRectChanged(const QRectF&)), object_info_widget_, SLOT(SceneRectChanged(const QRectF&)));
        
        // Init manipulations widget
        object_manipulations_widget_ = new Ui::BuildingWidget(Ui::BuildingWidget::Left);
        object_manip_ui.setupUi(object_manipulations_widget_->GetInternal());

        layout_->AddCornerAnchor(object_manipulations_widget_, Qt::TopLeftCorner, Qt::TopLeftCorner);
        layout_->AddCornerAnchor(object_manipulations_widget_, Qt::BottomLeftCorner, Qt::BottomLeftCorner);

        object_manipulations_widget_->PrepWidget();
        connect(scene_, SIGNAL(sceneRectChanged(const QRectF&)), object_manipulations_widget_, SLOT(SceneRectChanged(const QRectF&)));

        // Init python handler
        python_handler_ = new PythonHandler(this);

        // Init editor handler
        property_editor_handler_ = new PropertyEditorHandler(&ui_helper_, this);
    }

    QObject *BuildSceneManager::GetPythonHandler() const
    {
        return python_handler_; 
    }

    StateMachine *BuildSceneManager::GetStateMachine()
    {
        UiServices::UiModule *ui_module = framework_->GetModule<UiServices::UiModule>();
        if (ui_module)
            return ui_module->GetUiStateMachine();
        else
            return 0;
    }

    void BuildSceneManager::ToggleBuildScene()
    {
        if (!scene_->isActive())
            ShowBuildScene();
        else
            HideBuildScene();
    }

    void BuildSceneManager::ShowBuildScene()
    {
        if (!inworld_state)
            return;

        StateMachine *machine = GetStateMachine();
        if (machine)
        {
            machine->SwitchToScene(scene_name_);

            object_info_widget_->CheckSize();
            object_manipulations_widget_->CheckSize();
        }
    }

    void BuildSceneManager::HideBuildScene()
    {
        StateMachine *machine = GetStateMachine();
        if (machine)
        {
            if (inworld_state)
                machine->SwitchToScene("Inworld");
            else
                machine->SwitchToScene("Ether");
        }
    }

    void BuildSceneManager::ObjectSelected(bool selected)
    {
        // Lets not do this on every prim selection
        // if not really needed
        if (prim_selected_ == selected)
            return;
        object_info_ui.status_label->setVisible(!selected);
        object_info_ui.object_viewport->setVisible(selected);
        object_info_ui.server_id_title->setVisible(selected);
        object_info_ui.server_id_value->setVisible(selected);
        object_info_ui.local_id_title->setVisible(selected);
        object_info_ui.local_id_value->setVisible(selected);
        prim_selected_ = selected;
    }

    void BuildSceneManager::ObjectSelected(Scene::Entity *entity)
    {
        if (!entity)
            return;

        EC_OpenSimPrim *prim = entity->GetComponent<EC_OpenSimPrim>().get();
        if (!prim)
            return;

        // Update our widgets UI
        object_info_ui.server_id_value->setText(ui_helper_.CheckUiValue(prim->getFullId()));
        object_info_ui.local_id_value->setText(ui_helper_.CheckUiValue(prim->getLocalId()));

        // Update pixmap here!

        if (!property_editor_handler_->HasCurrentPrim())
        {
            // Create browser and element visibility
            property_editor_handler_->CreatePropertyBrowser(object_info_widget_->GetInternal(),
                                                            object_info_ui.property_browser_layout, prim);
            ObjectSelected(true);
        }
        else
        {
            // Update property editor UI
            property_editor_handler_->PrimSelected(prim);
        }
    }
}