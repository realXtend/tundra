//$ HEADER_MOD_FILE $
// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#include "BuildSceneManager.h"
#include "BuildScene.h"
#include "AnchorLayout.h"
#include "PropertyEditorHandler.h"
#include "BuildingWidget.h"
#include "UiHelper.h"

#include "IModule.h"
#include "EC_OpenSimPrim.h"
#include "EC_Placeable.h"
#include "UiServiceInterface.h"
#include "NaaliApplication.h"
//$ BEGIN_MOD $
#include "UiExternalServiceInterface.h"
//$ END_MOD $
#include "BuildToolbar.h"
#include <QPixmap>
#include <QDebug>

#include "MemoryLeakCheck.h"

namespace WorldBuilding
{
    BuildSceneManager::BuildSceneManager(QObject *parent, Foundation::Framework *framework) :
        framework_(framework),
        scene_(0),
        ui_helper_(new Helpers::UiHelper(parent, framework)),
        inworld_state(false),
        scene_name_("WorldBuilding"),
        world_object_view_(0),
        python_handler_(0),
        property_editor_handler_(0),
        camera_handler_(0),
        prim_selected_(true),
        selected_entity_(0),
        selected_camera_id_(-1),
        viewport_poller_(new QTimer(this)),
        override_server_time_(false),
//$ BEGIN_MOD $
		inside_(false)
//$ END_MOD $
    {
        setParent(parent);
        connect(framework_->GetNaaliApplication(), SIGNAL(aboutToQuit()), SLOT(CleanPyWidgets()));
        connect(viewport_poller_, SIGNAL(timeout()), SLOT(UpdateObjectViewport()));

        InitScene();
        ObjectSelected(false); 

		//$ BEGIN_MOD $
		//Create Action to Create an object & switch to build scene 

		//Check if UiExternalIsAvailable 
		UiServiceInterface *ui = framework_->GetService<UiServiceInterface>();
		if (ui){
			//Create Action, insert into menu Create->Avatar
			action = new QAction("Object",this);
			if (ui->AddExternalMenuAction(action, "Object", tr("Create"))){
				//We change scene
				connect(action, SIGNAL(triggered()), SLOT(ChangeAndCreateObject()));
			}
			//Create also an action to switch to Scene in Menu->Panels
			action2 = new QAction("Modify Object",this);
			if (ui->AddExternalMenuAction(action2, "Modify Object", tr("Panels"))){
				//We change scene
				connect(action2, SIGNAL(triggered()), SLOT(ShowBuildPanels()));
				//connect(ui, SIGNAL(SceneChanged(const QString&, const QString &)),
                //SLOT(SceneChangedNotification(const QString&, const QString&)));
			}
		}
		//$ END_MOD $
    }

    BuildSceneManager::~BuildSceneManager()
    {
        SAFE_DELETE(object_info_widget_);
        SAFE_DELETE(object_manipulations_widget_);
    }

    void BuildSceneManager::CleanPyWidgets()
    {
        // Remove pythong inserted widgets from lauyout as they are deleted in py code
        foreach (QWidget *py_widget, python_deleted_widgets_)
        {
            py_widget->hide();
            int index = object_manip_ui.main_layout->indexOf(py_widget);
            if (index == -1)
                continue;
            QLayoutItem *item = object_manip_ui.main_layout->takeAt(index);
            if (!item)
                continue;
            object_manip_ui.main_layout->removeItem(item);
            delete item; // Deletes the layout item which != the widget
            py_widget->setParent(0);
            py_widget = 0;
        }
        foreach (QWidget *visib_widget, toggle_visibility_widgets_)
        {
            if (!visib_widget)
                continue;
            QLabel *label = dynamic_cast<QLabel*>(visib_widget);
            if (!label)
                continue;
            int index = object_manip_ui.main_layout->indexOf(label);
            if (index == -1)
                continue;
            QLayoutItem *item = object_manip_ui.main_layout->takeAt(index);
            if (!item)
                continue;
            object_manip_ui.main_layout->removeItem(item);
            delete item; // Deletes the layout item which != the widget
            label->setParent(0);
            SAFE_DELETE(label); // Delete QLabels also
        }
        python_deleted_widgets_.clear();
        toggle_visibility_widgets_.clear();
    }

    bool BuildSceneManager::IsBuildingActive()
    { 
        if (!scene_) 
            return false; 
        else 
            return scene_->isActive(); 
    }

    void BuildSceneManager::InitScene()
    {
        scene_ = new BuildScene(this);
        layout_ = new AnchorLayout(this, scene_);

        // Register scene to ui service
        UiServiceInterface *ui = framework_->GetService<UiServiceInterface>();
        if (ui)
        {
            ui->RegisterScene(scene_name_, scene_);
            connect(ui, SIGNAL(SceneChanged(const QString&, const QString &)),
                SLOT(SceneChangedNotification(const QString&, const QString&)));
            connect(ui, SIGNAL(TransferRequest(const QString&, QGraphicsProxyWidget*)),
                SLOT(HandleWidgetTransfer(const QString&, QGraphicsProxyWidget*)));
        }

        // Init info widget
        object_info_widget_ = new Ui::BuildingWidget(Ui::BuildingWidget::Right);
        object_info_ui.setupUi(object_info_widget_->GetInternal());

        layout_->AddCornerAnchor(object_info_widget_, Qt::TopRightCorner, Qt::TopRightCorner);
        layout_->AddCornerAnchor(object_info_widget_, Qt::BottomRightCorner, Qt::BottomRightCorner);

        object_info_widget_->PrepWidget();
        connect(scene_, SIGNAL(sceneRectChanged(const QRectF&)), object_info_widget_, SLOT(SceneRectChanged(const QRectF&)));

        world_object_view_ = new WorldObjectView();
        object_info_ui.viewport_layout->addWidget(world_object_view_);
        object_info_widget_->SetWorldObjectView(world_object_view_);
        connect(world_object_view_, SIGNAL(UpdateMe()), SLOT(UpdateObjectViewport()));

        // Init manipulations widget
        object_manipulations_widget_ = new Ui::BuildingWidget(Ui::BuildingWidget::Left);
        object_manip_ui.setupUi(object_manipulations_widget_->GetInternal());
        object_manip_ui.rotate_frame->hide();

        connect(object_manip_ui.button_new, SIGNAL(clicked()), SLOT(NewObjectClicked()));
        connect(object_manip_ui.button_clone, SIGNAL(clicked()), SLOT(DuplicateObjectClicked()));
        connect(object_manip_ui.button_delete, SIGNAL(clicked()), SLOT(DeleteObjectClicked()));
        connect(object_manip_ui.button_move, SIGNAL(clicked()), SLOT(ModeToggleMove()));
        connect(object_manip_ui.button_scale, SIGNAL(clicked()), SLOT(ModeToggleScale()));
        connect(object_manip_ui.button_rotate, SIGNAL(clicked()), SLOT(ModeToggleRotate()));

        layout_->AddCornerAnchor(object_manipulations_widget_, Qt::TopLeftCorner, Qt::TopLeftCorner);
        layout_->AddCornerAnchor(object_manipulations_widget_, Qt::BottomLeftCorner, Qt::BottomLeftCorner);

        object_manipulations_widget_->PrepWidget();
        connect(scene_, SIGNAL(sceneRectChanged(const QRectF&)), object_manipulations_widget_, SLOT(SceneRectChanged(const QRectF&)));

        // Visibility button
        ui_helper_->SetupVisibilityButtons(layout_, object_manipulations_widget_, object_info_widget_);

        // Init the toolbar
        toolbar_ = new Ui::BuildToolbar();
        layout_->AnchorWidgetsHorizontally(object_info_widget_, toolbar_);
        connect(toolbar_->button_exit, SIGNAL(clicked()), SLOT(HideBuildScene()));
        connect(toolbar_->button_lights, SIGNAL(clicked()), SLOT(ToggleLights()));
        connect(toolbar_->slider_lights, SIGNAL(sliderMoved(int)), SIGNAL(SetOverrideTime(int)));

        // Init python handler
        python_handler_ = new PythonHandler(this);
        connect(python_handler_, SIGNAL(WidgetRecieved(const QString&, QWidget*)), SLOT(HandlePythonWidget(const QString&, QWidget*)));

        // Init editor handler
        property_editor_handler_ = new PropertyEditorHandler(ui_helper_, this);

        // Init camera handler
        camera_handler_ = new View::CameraHandler(framework_, this);
        connect(world_object_view_, SIGNAL(RotateObject(qreal, qreal)), SLOT(RotateObject(qreal, qreal))); 
        connect(world_object_view_, SIGNAL(Zoom(qreal)), SLOT(Zoom(qreal))); 

        // Setup ui helper
        ui_helper_->SetupManipControls(&object_manip_ui, python_handler_);

//$ BEGIN_MOD $
		Foundation::UiExternalServiceInterface *uiExternal= framework_->GetService<Foundation::UiExternalServiceInterface>();
		if(uiExternal)
			CreateEditContent();
//$ END_MOD $
    }

    void BuildSceneManager::HandleWidgetTransfer(const QString &name, QGraphicsProxyWidget *widget)
    {

		if (!widget)
            return;
        if (!scene_->isActive())
            return;
        if (widget->scene() == scene_)
            return;

        bool scene_widget = true;
        QString name_compare = name.toLower();
        if (name_compare == "components")
        {
            tranfer_widgets_[name] = TransferPair(widget->widget(), widget);
            widget->setWidget(0);
            object_info_ui.components_layout->addWidget(tranfer_widgets_[name].first);
            tranfer_widgets_[name].first->show();
            scene_widget = false;
        }
        else if (name_compare == "console")
        {
            scene_->addItem(widget);
            scene_widget = false;
        }
        else if (name_compare == "inventory")
        {
            tranfer_widgets_[name] = TransferPair(widget->widget(), widget);
            widget->setWidget(0);
            object_manip_ui.tab_widget->addTab(tranfer_widgets_[name].first, name);
            tranfer_widgets_[name].first->show();
            scene_widget = false;
        }
        else if (name_compare == "library")
        {
            tranfer_widgets_[name] = TransferPair(widget->widget(), widget);
            widget->setWidget(0);
            object_manip_ui.tab_widget->addTab(tranfer_widgets_[name].first, name);
            tranfer_widgets_[name].first->show();
            scene_widget = false;
        }

        if (scene_widget)
        {
            // Add to scene and toolbar
            scene_->addItem(widget);
            toolbar_->AddButton(name, widget);
            // Set initial pos and hide
            widget->setPos(object_manipulations_widget_->rect().width() + 25, 60);
            widget->hide();
        }
//$ BEGIN_MOD $
		Foundation::UiExternalServiceInterface *uiExternal= framework_->GetService<Foundation::UiExternalServiceInterface>();
		if(uiExternal){
			if(!inside_){
				TransferAssetWidget(inside_);
			}
		}
//$ END_MOD $
    }

    void BuildSceneManager::HandleTransfersBack()
    {
        foreach (QString name, tranfer_widgets_.keys())
        {
            int index = -1;
            TransferPair pair = tranfer_widgets_[name];
            if (name == "Components")
            {
                index = object_info_ui.components_layout->indexOf(pair.first);
                QLayoutItem *item = object_info_ui.components_layout->takeAt(index);
                if (item)
                    delete item;
            }
            else
            {
                index = object_manip_ui.tab_widget->indexOf(pair.first);
                if (index != -1)
                    object_manip_ui.tab_widget->removeTab(index);
            }
            pair.first->setParent(0);
            pair.second->setWidget(pair.first);
            pair.second->hide();
        }
        tranfer_widgets_.clear();
        object_manip_ui.tab_widget->clear();
//$ BEGIN_MOD $
		Foundation::UiExternalServiceInterface *uiExternal= framework_->GetService<Foundation::UiExternalServiceInterface>();
		if(uiExternal){
			if(inside_){
				TransferAssetWidget(inside_);
			}
		}
//$ END_MOD $
    }

    void BuildSceneManager::HandlePythonWidget(const QString &type, QWidget *widget)
    {
        if (!widget)
            return;

        // Check for type
        bool create_widgets = true;
        bool toggle_visiblity = true;
        bool python_deletes = true;
        QString type_compare = type.toLower();
        QString label_title = type;
        if (type_compare == "materials")
        {
            label_title = "Textures";
        }
        else if (type_compare == "mesh")
        {
            label_title = "3D Mesh Model";

            QPushButton *mesh_browse = widget->findChild<QPushButton*>("Browse");
            if (mesh_browse)
                ui_helper_->AddBrowsePair("mesh", mesh_browse, widget);
        }
        else if (type_compare == "sound")
        {
            label_title = "Attached Sound";
            
            QPushButton *mesh_browse = widget->findChild<QPushButton*>("Browse");
            if (mesh_browse)
                ui_helper_->AddBrowsePair("sound", mesh_browse, widget);
        }
        else if (type_compare == "animation")
        {
            toggle_visiblity = false;
            label_title = "";

            QPushButton *mesh_browse = widget->findChild<QPushButton*>("Browse");
            if (mesh_browse)
                ui_helper_->AddBrowsePair("animation", mesh_browse, widget);
        }
        else
            create_widgets = false;

        if (create_widgets)
        {
            int inject_pos = object_manip_ui.main_layout->count() - 1;
//$ BEGIN_MOD $		
			Foundation::UiExternalServiceInterface *uiExternal= framework_->GetService<Foundation::UiExternalServiceInterface>();
//$ END_MOD $	
			// Make title if needed
            if (!label_title.isEmpty())
            {
                // Make title and insert widget
                QLabel *title = new QLabel(label_title);
                title->setStyleSheet("font-size:18px;font-weight:bold;padding-top:5px;");
                title->setIndent(0);
                title->hide();

                toggle_visibility_widgets_ << title;
                object_manip_ui.main_layout->insertWidget(inject_pos, title);
//$ BEGIN_MOD $		
				if(uiExternal){
					panel_widgets_ << title;
					asset_ui_.main_layout->insertWidget(asset_ui_.main_layout->count(), title);
				}
//$ END_MOD $
                inject_pos++;
            }

            // Insert widget to layout
            widget->hide();
            object_manip_ui.main_layout->insertWidget(inject_pos, widget);		
//$ BEGIN_MOD $			
			if(uiExternal){
				panel_widgets_ << widget;
				asset_ui_.main_layout->insertWidget(asset_ui_.main_layout->count(), widget);
			}
//$ END_MOD $
            // Put to internal lists for visibility and deleting
            if (toggle_visiblity)
                toggle_visibility_widgets_ << widget;
            if (python_deletes)
                python_deleted_widgets_ << widget;
        }
    }

    void BuildSceneManager::KeyPressed(KeyEvent *key)
    {
        if (key->IsRepeat())
            return;

        // Ctrl + B to toggle build scene
        if (key->HasCtrlModifier() && key->keyCode == Qt::Key_B)
        {
            ToggleBuildScene();
            return;
        }

        // Below are only taken in if scene is active and we have a selected object
        if (!scene_->isActive() || !property_editor_handler_->HasCurrentPrim() || !prim_selected_)
            return;

        // Delete object
        if (key->keyCode == Qt::Key_Delete)
        {
            DeleteObjectClicked();
            return;
        }

        PythonParams::ManipulationMode mode = PythonParams::MANIP_NONE;
        if (key->HasCtrlModifier())
        {
            mode = PythonParams::MANIP_MOVE;
            if (key->HasAltModifier() && (python_handler_->GetCurrentManipulationMode() != PythonParams::MANIP_ROTATE))
                mode = PythonParams::MANIP_SCALE;
            if (key->HasShiftModifier() && (python_handler_->GetCurrentManipulationMode() != PythonParams::MANIP_SCALE))
                mode = PythonParams::MANIP_ROTATE;
        }
        if (mode != PythonParams::MANIP_NONE)
            ManipModeChanged(mode);
    }

    void BuildSceneManager::KeyReleased(KeyEvent *key)
    {
        if (!scene_->isActive() || !property_editor_handler_->HasCurrentPrim() || !prim_selected_)
            return;
        if (key->IsRepeat())
            return;

        PythonParams::ManipulationMode mode = PythonParams::MANIP_NONE;
        if (key->keyCode == Qt::Key_Control && (python_handler_->GetCurrentManipulationMode() == PythonParams::MANIP_MOVE))
            mode = PythonParams::MANIP_FREEMOVE;
        if (key->keyCode == Qt::Key_Alt && (python_handler_->GetCurrentManipulationMode() == PythonParams::MANIP_SCALE))
        {
            if (key->HasCtrlModifier())
                mode = PythonParams::MANIP_MOVE;
            else
                mode = PythonParams::MANIP_FREEMOVE;
        }
        if (key->keyCode == Qt::Key_Shift && (python_handler_->GetCurrentManipulationMode() == PythonParams::MANIP_ROTATE))
        {
            if (key->HasCtrlModifier())
                mode = PythonParams::MANIP_MOVE;
            else
                mode = PythonParams::MANIP_FREEMOVE;
        }
        if (mode != PythonParams::MANIP_NONE)
            ManipModeChanged(mode);
    }

    void BuildSceneManager::ModeToggleMove()
    {
        if (!scene_->isActive() || !property_editor_handler_->HasCurrentPrim() || !prim_selected_)
            return;

        if (python_handler_->GetCurrentManipulationMode() == PythonParams::MANIP_MOVE)
            ManipModeChanged(PythonParams::MANIP_FREEMOVE);
        else
            ManipModeChanged(PythonParams::MANIP_MOVE);
    }

    void BuildSceneManager::ModeToggleScale()
    {
        if (!scene_->isActive() || !property_editor_handler_->HasCurrentPrim() || !prim_selected_)
            return;

        if (python_handler_->GetCurrentManipulationMode() == PythonParams::MANIP_SCALE)
            ManipModeChanged(PythonParams::MANIP_FREEMOVE);
        else
            ManipModeChanged(PythonParams::MANIP_SCALE);
    }

    void BuildSceneManager::ModeToggleRotate()
    {
        if (!scene_->isActive() || !property_editor_handler_->HasCurrentPrim() || !prim_selected_)
            return;

        if (python_handler_->GetCurrentManipulationMode() == PythonParams::MANIP_ROTATE)
            ManipModeChanged(PythonParams::MANIP_FREEMOVE);
        else
            ManipModeChanged(PythonParams::MANIP_ROTATE);
    }

    void BuildSceneManager::NewObjectClicked()
    {
        python_handler_->EmitObjectAction(PythonParams::OBJ_NEW);
    }

    void BuildSceneManager::DuplicateObjectClicked()
    {
        python_handler_->EmitObjectAction(PythonParams::OBJ_CLONE);
    }

    void BuildSceneManager::DeleteObjectClicked()
    {
        python_handler_->EmitObjectAction(PythonParams::OBJ_DELETE);
        ObjectSelected(false);
    }

    void BuildSceneManager::ManipModeChanged(PythonParams::ManipulationMode mode)
    {
        python_handler_->EmitManipulationModeChange(mode);
        ui_helper_->SetManipMode(mode);

        bool show_rotate_controls = false;
        bool show_scale_controls = false;
        bool show_pos_controls = false;
        QString selected_style = "background-color: qlineargradient(spread:pad, x1:0, y1:0.165, x2:0, y2:0.864, stop:0 rgba(248, 248, 248, 255), stop:1 rgba(232, 232, 232, 255));"
                                 "border: 1px solid grey; border-radius: 0px; color: black; font-weight: bold; padding-top: 5px; padding-bottom: 4px;";
        switch (mode)
        {
            case PythonParams::MANIP_MOVE:
                object_manip_ui.button_move->setStyleSheet(selected_style);
                object_manip_ui.button_scale->setStyleSheet("");
                object_manip_ui.button_rotate->setStyleSheet("");
                show_pos_controls = true;
                break;
            case PythonParams::MANIP_SCALE:
                object_manip_ui.button_scale->setStyleSheet(selected_style);
                object_manip_ui.button_move->setStyleSheet("");
                object_manip_ui.button_rotate->setStyleSheet("");
                show_scale_controls = true;
                break;
            case PythonParams::MANIP_ROTATE:
                object_manip_ui.button_rotate->setStyleSheet(selected_style);
                object_manip_ui.button_move->setStyleSheet("");
                object_manip_ui.button_scale->setStyleSheet("");
                show_rotate_controls = true;
                break;
            case PythonParams::MANIP_FREEMOVE:
                object_manip_ui.button_move->setStyleSheet("");
                object_manip_ui.button_scale->setStyleSheet("");
                object_manip_ui.button_rotate->setStyleSheet("");
                break;
            default:
                break;
        }
        object_manip_ui.rotate_frame->setVisible(show_rotate_controls);
        object_manip_ui.scale_frame->setVisible(show_scale_controls);
        object_manip_ui.pos_frame->setVisible(show_pos_controls);
    }

    QObject *BuildSceneManager::GetPythonHandler() const
    {
        return python_handler_; 
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

        UiServiceInterface *ui = framework_->GetService<UiServiceInterface>();
        if (ui)
        {
            ui->SwitchToScene(scene_name_);
            object_info_widget_->CheckSize();
            object_manipulations_widget_->CheckSize();
        }

    }

    void BuildSceneManager::HideBuildScene()
    {
        UiServiceInterface *ui = framework_->GetService<UiServiceInterface>();
        if (ui)
        {
			if (inworld_state)
                ui->SwitchToScene("Inworld");
			else
                ui->SwitchToScene("Ether");
        }
    }

    void BuildSceneManager::SceneChangedNotification(const QString &old_name, const QString &new_name)
    {
        if (new_name == scene_name_)
        {
            object_info_widget_->CheckSize();
            object_manipulations_widget_->CheckSize();
            python_handler_->EmitEditingActivated(true);
            tranfer_widgets_.clear();
            object_manip_ui.tab_widget->clear();
			//$ BEGIN_MOD $
			action->setDisabled(true);
			action2->setDisabled(true);
			//$ END_MOD $
        }
        else if (old_name == scene_name_)
        {
            ObjectSelected(false);
            python_handler_->EmitEditingActivated(false);
            toolbar_->RemoveAllButtons();
            HandleTransfersBack();
			//$ BEGIN_MOD $
			action->setDisabled(false);
			action2->setDisabled(false);
			//$ END_MOD $
        }
//$ BEGIN_MOD $
		Foundation::UiExternalServiceInterface *uiExternal= framework_->GetService<Foundation::UiExternalServiceInterface>();
		if(uiExternal && new_name=="Inworld")
			ActiveEditMode(uiExternal->IsEditModeEnable());
//$ END_MOD $
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
        override_server_time_ = false;
    }
    
    void BuildSceneManager::ObjectDeselected()
    {
        ObjectSelected(false);
        python_handler_->EmitRemoveHightlight();
    }

    void BuildSceneManager::ObjectSelected(bool selected)
    {
        // Update ui elements
        if (selected)
            ManipModeChanged(python_handler_->GetCurrentManipulationMode());
        else
            ManipModeChanged(PythonParams::MANIP_FREEMOVE);
//$ BEGIN_MOD $
		Foundation::UiExternalServiceInterface *uiExternal= framework_->GetService<Foundation::UiExternalServiceInterface>();
		if(uiExternal){
		    if (selected)
				ManipExternalModeChanged(python_handler_->GetCurrentManipulationMode());
			else
				ManipExternalModeChanged(PythonParams::MANIP_FREEMOVE);
		}

         // Update ui visibility if the mode changed from previous known one
        if (prim_selected_ == selected)
            return;
        prim_selected_ = selected;

        world_object_view_->setVisible(selected);
        object_info_ui.status_label->setVisible(!selected);
        object_info_ui.server_id_title->setVisible(selected);
        object_info_ui.server_id_value->setVisible(selected);
        object_info_ui.local_id_title->setVisible(selected);
        object_info_ui.local_id_value->setVisible(selected);

		object_manip_ui.button_clone->setEnabled(selected);
		object_manip_ui.button_delete->setEnabled(selected);
		object_manip_ui.button_scale->setEnabled(selected);
		object_manip_ui.button_move->setEnabled(selected);
		object_manip_ui.button_rotate->setEnabled(selected);

//$ BEGIN_MOD $
		if(uiExternal){
			asset_ui_.button_clone->setEnabled(selected);
			asset_ui_.button_delete->setEnabled(selected);
			asset_ui_.button_scale->setEnabled(selected);
			asset_ui_.button_move->setEnabled(selected);
			asset_ui_.button_rotate->setEnabled(selected);
		}
//$ END_MOD $
        property_editor_handler_->SetEditorVisible(selected);

        foreach(QWidget *widget, toggle_visibility_widgets_)
            widget->setVisible(selected);

        if (selected)
            viewport_poller_->start(50);
        else
        {
            selected_entity_ = 0;
            viewport_poller_->stop();
        }
    }

    void BuildSceneManager::ObjectSelected(Scene::Entity *entity)
    {
        if (!entity || !scene_->isActive())
            return;

        EC_OpenSimPrim *prim = entity->GetComponent<EC_OpenSimPrim>().get();
        if (!prim)
        {
            // Manipulators wont have prim, but will have mesh
            if (entity->HasComponent("EC_Mesh"))
                return;
            ObjectSelected(false);
            python_handler_->EmitRemoveHightlight();
            return;
        }

        // Update our widgets UI
        object_info_ui.server_id_value->setText(ui_helper_->CheckUiValue(prim->getFullId()));
        object_info_ui.local_id_value->setText(ui_helper_->CheckUiValue(prim->getLocalId()));
        
        // Update the property editor
        if (!property_editor_handler_->HasCurrentPrim())
            property_editor_handler_->CreatePropertyBrowser(object_info_widget_->GetInternal(), object_info_ui.property_browser_layout, prim);
        else
            property_editor_handler_->PrimSelected(prim);
        selected_entity_ =  entity;

        // Starts to update the viewport with a timer
        ObjectSelected(true);
    }

    void BuildSceneManager::UpdateObjectViewport()
    {
        if (selected_camera_id_ != -1 && world_object_view_ && selected_entity_)
            if (camera_handler_->FocusToEntity(selected_camera_id_, selected_entity_))
                world_object_view_->setPixmap(camera_handler_->RenderCamera(selected_camera_id_, world_object_view_->size()));
    }

    void BuildSceneManager::Zoom(qreal delta)
    {
        if (selected_entity_)
        {
            EC_Placeable *entity_ec_placable = selected_entity_->GetComponent<EC_Placeable>().get();
            if (entity_ec_placable)
            {
                qreal acceleration = 0.01;
                camera_handler_->ZoomRelativeToPoint(entity_ec_placable->GetPosition(),selected_camera_id_, delta*acceleration);
            }
        }
    }

    void BuildSceneManager::RotateObject(qreal x, qreal y)
    {
        if (selected_entity_)
        {
            EC_Placeable *entity_ec_placable = selected_entity_->GetComponent<EC_Placeable>().get();
            if (entity_ec_placable)
            {
                qreal acceleration_x = 1;
                qreal acceleration_y = 1;
                camera_handler_->RotateCamera(entity_ec_placable->GetPosition(),selected_camera_id_,x*acceleration_x,y*acceleration_y);
            }
        }
    }

    void BuildSceneManager::ToggleLights()
    {
        override_server_time_ = !override_server_time_;
        emit OverrideServerTime((int)override_server_time_);
        if (override_server_time_)
        {
            emit SetOverrideTime(toolbar_->slider_lights->value());
            toolbar_->button_lights->setText("Disable Custom Lights");
        }
        else
            toolbar_->button_lights->setText("Custom Lights");
        toolbar_->slider_lights->setVisible(override_server_time_);
    }
//$ BEGIN_MOD $
	void BuildSceneManager::CreateEditContent()
	{
		Foundation::UiExternalServiceInterface *uiExternal= framework_->GetService<Foundation::UiExternalServiceInterface>();
		UiServiceInterface *ui = framework_->GetService<UiServiceInterface>();
		if(uiExternal && ui){

			editToolbar_= ui->GetExternalToolbar("EditToolBar");

			QAction* move=new QAction(QIcon("./media/icons/move.png"),"Move",editToolbar_);
			editToolbar_->addAction(move);
			connect(move, SIGNAL(triggered()), this, SLOT(ActionToolBarMove()));

			QAction* scale=new QAction(QIcon("./media/icons/scale.png"),"Scale",editToolbar_);
			editToolbar_->addAction(scale);
			connect(scale, SIGNAL(triggered()), this, SLOT(ActionToolBarScale()));

			QAction* rotate=new QAction(QIcon("./media/icons/rotate.png"),"Rotate",editToolbar_);
			editToolbar_->addAction(rotate);
			connect(rotate, SIGNAL(triggered()), this, SLOT(ActionToolBarRotate()));

			QAction* create=new QAction(QIcon("./media/icons/create.png"),"Create",editToolbar_);
			editToolbar_->addAction(create);
			connect(create, SIGNAL(triggered()), this, SLOT(ActionToolBarCreate()));

			QAction* assign=new QAction(QIcon("./media/icons/asset.png"),"Asset",editToolbar_);
			editToolbar_->addAction(assign);
			connect(assign, SIGNAL(triggered()), this, SLOT(ActionToolBarAsset()));

			QAction* properties=new QAction(QIcon("./media/icons/properties.png"),"Properties",editToolbar_);
			editToolbar_->addAction(properties);
			connect(properties, SIGNAL(triggered()), this, SLOT(ActionToolBarProperties()));

			//Current World Build
			QToolBar *worldBuildToolbar= ui->GetExternalToolbar("WorldBuildToolBar");
			worldBuildToolbar->setIconSize(QSize(76, 32));
			QAction* worldbuild=new QAction(QIcon("./media/icons/uibutton_BUILD_normal.png"),"WorldBuildScene",worldBuildToolbar);
			worldBuildToolbar->addAction(worldbuild);
			connect(worldbuild, SIGNAL(triggered()), this, SLOT(ToggleBuildScene()));

			

			UiServiceInterface *ui = framework_->GetService<UiServiceInterface>();

			//Create the Properties Widget
			propertyWidget_ = property_editor_handler_->CreatePropertyWindow();
			ui->AddWidgetToScene(propertyWidget_,true,true);
			ui->AddWidgetToMenu(propertyWidget_,"Properties","Panels");
			
			//Create the Assign Widget
			assignWidget_= new QWidget();
			asset_ui_.setupUi(assignWidget_);
			asset_ui_.tab_widget->setVisible(false);

			connect(asset_ui_.button_new, SIGNAL(clicked()), SLOT(NewObjectClicked()));
			connect(asset_ui_.button_clone, SIGNAL(clicked()), SLOT(DuplicateObjectClicked()));
			connect(asset_ui_.button_delete, SIGNAL(clicked()), SLOT(DeleteObjectClicked()));
			connect(asset_ui_.button_move, SIGNAL(clicked()), SLOT(ExternalToggleMove()));
			connect(asset_ui_.button_scale, SIGNAL(clicked()), SLOT(ExternalToggleScale()));
			connect(asset_ui_.button_rotate, SIGNAL(clicked()), SLOT(ExternalToggleRotate()));

			// Setup ui helper
			ui_helper_->SetupManipControls(&asset_ui_, python_handler_);

			assignWidget_->setMinimumSize(QSize(300,300));
			assignWidget_->setWindowTitle("Asset");
			ui->AddWidgetToScene(assignWidget_,true,true);
			ui->AddWidgetToMenu(assignWidget_,"Asset","Panels");
			ui->AddPanelToEditMode(assignWidget_);

			connect(uiExternal,SIGNAL(EditModeChanged(bool)),this,SLOT(ActiveEditMode(bool)));
		}
	}
	void BuildSceneManager::ActiveEditMode(bool active){
		if(active){
			python_handler_->EmitEditingActivated(true);
			editToolbar_->setEnabled(true);
		}else{
			python_handler_->EmitEditingActivated(false);
			ObjectSelected(false);
			editToolbar_->setEnabled(false);
		}
	}
	void BuildSceneManager::ActionToolBarMove()
    {
		ExternalToggleMove();
	}

    void BuildSceneManager::ActionToolBarScale()
    {
		ExternalToggleScale();
    }

    void BuildSceneManager::ActionToolBarRotate()
    {
		ExternalToggleRotate();
    }

	void BuildSceneManager::ActionToolBarCreate()
	{
		python_handler_->EmitObjectAction(PythonParams::OBJ_NEW);
	}

	void BuildSceneManager::ActionToolBarProperties()
	{
		UiServiceInterface *ui = framework_->GetService<UiServiceInterface>();
		if (ui && !scene_->isActive()){
			if(!propertyWidget_->isVisible())
				ui->ShowWidget(propertyWidget_);
			else
				ui->HideWidget(propertyWidget_);
		}
	}

	void BuildSceneManager::ActionToolBarAsset()
	{
		UiServiceInterface *ui = framework_->GetService<UiServiceInterface>();
		if(ui && !scene_->isActive()){
			if(!assignWidget_->isVisible())
				ui->ShowWidget(assignWidget_);
			else
				ui->HideWidget(assignWidget_);
		}
	}

	void BuildSceneManager::GetSelectedEntity(Scene::Entity *entity)
	{
		if(!scene_->isActive())
			if(entity && !entity->HasComponent("EC_Terrain") && !entity->HasComponent("EC_Controllable")){
				ObjectSelected(true);
				EC_OpenSimPrim *prim = entity->GetComponent<EC_OpenSimPrim>().get();
				if (prim && propertyWidget_->isVisible())
					property_editor_handler_->UpdatePropertyWindow(prim);
			}else{
				if(prim_selected_)
					python_handler_->EmitRemoveHightlight();
				ObjectSelected(false);
				property_editor_handler_->UpdatePropertyWindow(0);
			}
	}

	void BuildSceneManager::ChangeAndCreateObject(){
		//Change to Build Scene if we are not still there
		if (!IsBuildingActive())
			ShowBuildPanels();
		//Create new object
		NewObjectClicked();	
	}

	void BuildSceneManager::ExternalToggleMove()
    {
		if(prim_selected_){
			if (python_handler_->GetCurrentManipulationMode() == PythonParams::MANIP_MOVE)
				ManipExternalModeChanged(PythonParams::MANIP_FREEMOVE);
			else
				ManipExternalModeChanged(PythonParams::MANIP_MOVE);
		}
    }

    void BuildSceneManager::ExternalToggleScale()
    {
		if(prim_selected_){
			if (python_handler_->GetCurrentManipulationMode() == PythonParams::MANIP_SCALE)
				ManipExternalModeChanged(PythonParams::MANIP_FREEMOVE);
			else
				ManipExternalModeChanged(PythonParams::MANIP_SCALE);
		}
    }

    void BuildSceneManager::ExternalToggleRotate()
    {
		if(prim_selected_){
			if (python_handler_->GetCurrentManipulationMode() == PythonParams::MANIP_ROTATE)
				ManipExternalModeChanged(PythonParams::MANIP_FREEMOVE);
			else
				ManipExternalModeChanged(PythonParams::MANIP_ROTATE);
		}
    }

	void BuildSceneManager::TransferAssetWidget(bool inside){
		if(inside){
			foreach(QWidget *w, panel_widgets_){
				int inject_pos = asset_ui_.main_layout->count() - 2;
				asset_ui_.main_layout->insertWidget(inject_pos,w);
			}

			// Setup ui helper
			ui_helper_->SetupManipControls(&asset_ui_, python_handler_);

			assignWidget_->setEnabled(true);
			inside_=false;
		}else{
			foreach(QWidget *w, panel_widgets_){
				int inject_pos = object_manip_ui.main_layout->count() - 2;
				w->hide();
				object_manip_ui.main_layout->insertWidget(inject_pos, w);
			}

			// Setup ui helper
			ui_helper_->SetupManipControls(&object_manip_ui, python_handler_);

			assignWidget_->setEnabled(false);
			inside_=true;
		}
	}

    void BuildSceneManager::ManipExternalModeChanged(PythonParams::ManipulationMode mode)
    {
        python_handler_->EmitManipulationModeChange(mode);
        ui_helper_->SetManipMode(mode);

        bool show_rotate_controls = false;
        bool show_scale_controls = false;
        bool show_pos_controls = false;
        QString selected_style = "background-color: qlineargradient(spread:pad, x1:0, y1:0.165, x2:0, y2:0.864, stop:0 rgba(248, 248, 248, 255), stop:1 rgba(232, 232, 232, 255));"
                                 "border: 1px solid grey; border-radius: 0px; color: black; font-weight: bold; padding-top: 5px; padding-bottom: 4px;";
        switch (mode)
        {
            case PythonParams::MANIP_MOVE:
                asset_ui_.button_move->setStyleSheet(selected_style);
                asset_ui_.button_scale->setStyleSheet("");
                asset_ui_.button_rotate->setStyleSheet("");
                show_pos_controls = true;
                break;
            case PythonParams::MANIP_SCALE:
                asset_ui_.button_scale->setStyleSheet(selected_style);
                asset_ui_.button_move->setStyleSheet("");
                asset_ui_.button_rotate->setStyleSheet("");
                show_scale_controls = true;
                break;
            case PythonParams::MANIP_ROTATE:
                asset_ui_.button_rotate->setStyleSheet(selected_style);
                asset_ui_.button_move->setStyleSheet("");
                asset_ui_.button_scale->setStyleSheet("");
                show_rotate_controls = true;
                break;
            case PythonParams::MANIP_FREEMOVE:
                asset_ui_.button_move->setStyleSheet("");
                asset_ui_.button_scale->setStyleSheet("");
                asset_ui_.button_rotate->setStyleSheet("");
                break;
            default:
                break;
        }
        asset_ui_.rotate_frame->setVisible(show_rotate_controls);
        asset_ui_.scale_frame->setVisible(show_scale_controls);
        asset_ui_.pos_frame->setVisible(show_pos_controls);
    }
	void BuildSceneManager::ShowBuildPanels(){
		UiServiceInterface *ui = framework_->GetService<UiServiceInterface>();
		ui->ShowWidget(assignWidget_);
		ui->ShowWidget(propertyWidget_);
		ui->BringWidgetToFront("Entity-component Editor");
		ui->BringWidgetToFront("Inventory");
		ui->BringWidgetToFront("Library");
	}
//$ END_MOD $
}
