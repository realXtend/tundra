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
#include <QDebug>

namespace WorldBuilding
{
    BuildSceneManager::BuildSceneManager(QObject *parent, Foundation::Framework *framework) :
        framework_(framework),
        inworld_state(false),
        scene_name_("WorldBuilding"),
        python_handler_(0),
        property_editor_handler_(0),
        camera_handler_(0),
        prim_selected_(true),
        selected_camera_id_(-1)
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
        {
            machine->RegisterScene(scene_name_, scene_);
            connect(machine, SIGNAL(SceneChangedTo(QString, QString)), SLOT(SceneChangedNotification(QString, QString)));
        }

        // Init info widget
        object_info_widget_ = new Ui::BuildingWidget(Ui::BuildingWidget::Right);
        object_info_ui.setupUi(object_info_widget_->GetInternal());

        layout_->AddCornerAnchor(object_info_widget_, Qt::TopRightCorner, Qt::TopRightCorner);
        layout_->AddCornerAnchor(object_info_widget_, Qt::BottomRightCorner, Qt::BottomRightCorner);

        object_info_widget_->PrepWidget();
        connect(scene_, SIGNAL(sceneRectChanged(const QRectF&)), object_info_widget_, SLOT(SceneRectChanged(const QRectF&)));
        
        // Init manipulations widget
        object_manipulations_widget_ = new Ui::BuildingWidget(Ui::BuildingWidget::Left);
        object_manip_ui.setupUi(object_manipulations_widget_->GetInternal());

        connect(object_manip_ui.pushButton_move, SIGNAL(clicked()), SLOT(ModeToggleMove()));
        connect(object_manip_ui.pushButton_scale, SIGNAL(clicked()), SLOT(ModeToggleScale()));
        connect(object_manip_ui.pushButton_rotate, SIGNAL(clicked()), SLOT(ModeToggleRotate()));

        layout_->AddCornerAnchor(object_manipulations_widget_, Qt::TopLeftCorner, Qt::TopLeftCorner);
        layout_->AddCornerAnchor(object_manipulations_widget_, Qt::BottomLeftCorner, Qt::BottomLeftCorner);

        object_manipulations_widget_->PrepWidget();
        connect(scene_, SIGNAL(sceneRectChanged(const QRectF&)), object_manipulations_widget_, SLOT(SceneRectChanged(const QRectF&)));

        // Init python handler
        python_handler_ = new PythonHandler(this);

        // Init editor handler
        property_editor_handler_ = new PropertyEditorHandler(&ui_helper_, this);

        // Init camera handler
        camera_handler_ = new View::CameraHandler(framework_, this);
    }

    void BuildSceneManager::KeyPressed(KeyEvent &key)
    {
        if (key.IsRepeat())
            return;

        // Ctrl + B to toggle build scene
        if (key.HasCtrlModifier() && key.keyCode == Qt::Key_B)
            ToggleBuildScene();

        if (!scene_->isActive() || !property_editor_handler_->HasCurrentPrim() || !prim_selected_)
            return;

        if (key.HasCtrlModifier())
        {
            ManipModeChanged(PythonParams::MOVE);
            return;
        }
        if (key.HasAltModifier())
        {
            ManipModeChanged(PythonParams::SCALE);
            return;
        }
        if (key.HasShiftModifier())
        {
            ManipModeChanged(PythonParams::ROTATE);
            return;
        }
    }

    void BuildSceneManager::KeyReleased(KeyEvent &key)
    {
        if (!scene_->isActive() || !property_editor_handler_->HasCurrentPrim() || !prim_selected_)
            return;
        if (key.IsRepeat())
            return;

        bool back_to_freemove = false;
        if (key.keyCode == Qt::Key_Control && (python_handler_->GetCurrentManipulationMode() == PythonParams::MOVE))
            back_to_freemove = true;
        if (key.keyCode == Qt::Key_Alt && (python_handler_->GetCurrentManipulationMode() == PythonParams::SCALE))
            back_to_freemove = true;
        if (key.keyCode == Qt::Key_Shift && (python_handler_->GetCurrentManipulationMode() == PythonParams::ROTATE))
            back_to_freemove = true;
        if (back_to_freemove)
            ManipModeChanged(PythonParams::FREEMOVE);
    }

    void BuildSceneManager::ModeToggleMove()
    {
        if (!scene_->isActive() || !property_editor_handler_->HasCurrentPrim() || !prim_selected_)
            return;

        if (python_handler_->GetCurrentManipulationMode() == PythonParams::MOVE)
            ManipModeChanged(PythonParams::FREEMOVE);
        else
            ManipModeChanged(PythonParams::MOVE);
    }

    void BuildSceneManager::ModeToggleScale()
    {
        if (!scene_->isActive() || !property_editor_handler_->HasCurrentPrim() || !prim_selected_)
            return;

        if (python_handler_->GetCurrentManipulationMode() == PythonParams::SCALE)
            ManipModeChanged(PythonParams::FREEMOVE);
        else
            ManipModeChanged(PythonParams::SCALE);
    }

    void BuildSceneManager::ModeToggleRotate()
    {
        if (!scene_->isActive() || !property_editor_handler_->HasCurrentPrim() || !prim_selected_)
            return;

        if (python_handler_->GetCurrentManipulationMode() == PythonParams::ROTATE)
            ManipModeChanged(PythonParams::FREEMOVE);
        else
            ManipModeChanged(PythonParams::ROTATE);
    }

    void BuildSceneManager::ManipModeChanged(PythonParams::ManipulationMode mode)
    {
        python_handler_->EmitManipulationModeChange(mode);

        QString selected_style = "background-color: qlineargradient(spread:pad, x1:0, y1:0.165, x2:0, y2:0.864, stop:0 rgba(248, 248, 248, 255), stop:1 rgba(232, 232, 232, 255));"
                                 "border-radius: 3px; color: black; font-weight: bold; padding-top: 5px; padding-bottom: 4px;";
        switch (mode)
        {
            case PythonParams::MOVE:
                object_manip_ui.pushButton_move->setStyleSheet(selected_style);
                object_manip_ui.pushButton_scale->setStyleSheet("");
                object_manip_ui.pushButton_rotate->setStyleSheet("");
                break;
            case PythonParams::SCALE:
                object_manip_ui.pushButton_scale->setStyleSheet(selected_style);
                object_manip_ui.pushButton_move->setStyleSheet("");
                object_manip_ui.pushButton_rotate->setStyleSheet("");
                break;
            case PythonParams::ROTATE:
                object_manip_ui.pushButton_rotate->setStyleSheet(selected_style);
                object_manip_ui.pushButton_move->setStyleSheet("");
                object_manip_ui.pushButton_scale->setStyleSheet("");
                break;
            case PythonParams::FREEMOVE:
                object_manip_ui.pushButton_move->setStyleSheet("");
                object_manip_ui.pushButton_scale->setStyleSheet("");
                object_manip_ui.pushButton_rotate->setStyleSheet("");
                break;
            default:
                break;
        }
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

            python_handler_->EmitEditingActivated(true);
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

    void BuildSceneManager::SceneChangedNotification(QString old_scene_name, QString new_scene_name)
    {
        if (new_scene_name == scene_name_)
        {
            object_info_widget_->CheckSize();
            object_manipulations_widget_->CheckSize();         
            python_handler_->EmitEditingActivated(true);
        }
        else if (old_scene_name == scene_name_)
        {
            ObjectSelected(false);
            python_handler_->EmitEditingActivated(false);
        }
    }

    void BuildSceneManager::CreateCamera()
    {
        if (selected_camera_id_ == -1 && camera_handler_)
            selected_camera_id_ = camera_handler_->CreateCustomCamera();
    }

    void BuildSceneManager::ResetCamera()
    {
        if (selected_camera_id_ != -1 && camera_handler_)
        {
            camera_handler_->DestroyCamera(selected_camera_id_);
            selected_camera_id_ = -1;
        }
    }

    void BuildSceneManager::ResetEditing()
    {
        ObjectSelected(false);
        property_editor_handler_->ClearCurrentPrim();
    }

    void BuildSceneManager::ObjectSelected(bool selected)
    {
        // Lets not do this on every prim selection if not really needed
        if (prim_selected_ == selected)
            return;

        object_info_ui.status_label->setVisible(!selected);
        object_info_ui.object_viewport->setVisible(selected);
        object_info_ui.server_id_title->setVisible(selected);
        object_info_ui.server_id_value->setVisible(selected);
        object_info_ui.local_id_title->setVisible(selected);
        object_info_ui.local_id_value->setVisible(selected);

        // TODO: Works but looks ugly, do something later for this...
        //object_manip_ui.pushButton_move->setEnabled(selected);
        //object_manip_ui.pushButton_scale->setEnabled(selected);
        //object_manip_ui.pushButton_rotate->setEnabled(selected);
        object_manip_ui.pushButton_move->setStyleSheet("");
        object_manip_ui.pushButton_scale->setStyleSheet("");
        object_manip_ui.pushButton_rotate->setStyleSheet("");

        property_editor_handler_->SetEditorVisible(selected);
        prim_selected_ = selected;
    }

    void BuildSceneManager::ObjectSelected(Scene::Entity *entity)
    {
        if (!entity || !scene_->isActive())
            return;

        EC_OpenSimPrim *prim = entity->GetComponent<EC_OpenSimPrim>().get();
        if (!prim)
        {
            // Manipulators wont have prim, but will have mesh
            if (entity->HasComponent("EC_OgreMesh"))
                return;
            ObjectSelected(false);
            python_handler_->EmitRemoveHightlight();
            return;
        }

        // Update our widgets UI
        object_info_ui.server_id_value->setText(ui_helper_.CheckUiValue(prim->getFullId()));
        object_info_ui.local_id_value->setText(ui_helper_.CheckUiValue(prim->getLocalId()));

        // Update entity viewport UI
        if (camera_handler_->FocusToEntity(selected_camera_id_, entity))
        {
            object_info_ui.object_viewport->setPixmap(camera_handler_->RenderCamera(selected_camera_id_, object_info_ui.object_viewport->size()));
            if (!object_info_ui.object_viewport->text().isEmpty())
                object_info_ui.object_viewport->setText("");
        }
        else
        {
            QPixmap disabled_pixmap(object_info_ui.object_viewport->size());
            disabled_pixmap.fill(Qt::gray);
            object_info_ui.object_viewport->setPixmap(disabled_pixmap);
            object_info_ui.object_viewport->setText("Could not focus to object");
        }

        // Update the property editor
        if (!property_editor_handler_->HasCurrentPrim())
            property_editor_handler_->CreatePropertyBrowser(object_info_widget_->GetInternal(), object_info_ui.property_browser_layout, prim);
        else
            property_editor_handler_->PrimSelected(prim);
        ObjectSelected(true);
        
    }

    void BuildSceneManager::ObjectDeselected()
    {
        ObjectSelected(false);
        python_handler_->EmitRemoveHightlight();
    }
}