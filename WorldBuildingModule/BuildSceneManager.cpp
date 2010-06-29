// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "ModuleInterface.h"
#include "BuildSceneManager.h"

#include <UiModule.h>
#include <EC_OpenSimPrim.h>

#include "BuildScene.h"
#include "AnchorLayout.h"

#include <QPixmap>
#include <QtAbstractPropertyBrowser>
#include <QtTreePropertyBrowser>

#include <QDebug>

namespace WorldBuilding
{
    BuildSceneManager::BuildSceneManager(QObject *parent, Foundation::Framework *framework) :
        framework_(framework),
        inworld_state(false),
        scene_name_("WorldBuilding"),
        property_browser_(0)
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
        object_info_ui.status_label->setVisible(!selected);
        object_info_ui.object_viewport->setVisible(selected);
        object_info_ui.server_id_title->setVisible(selected);
        object_info_ui.server_id_value->setVisible(selected);
        object_info_ui.local_id_title->setVisible(selected);
        object_info_ui.local_id_value->setVisible(selected);

        if (!selected)
            current_prim_ = 0;
    }

    void BuildSceneManager::ObjectSelected(Scene::Entity *entity)
    {
        if (!entity)
            return;

        EC_OpenSimPrim *prim = entity->GetComponent<EC_OpenSimPrim>().get();
        if (!prim)
            return;

        if (!current_prim_)
            ObjectSelected(true);

        // Update ID's
        object_info_ui.server_id_value->setText(ui_helper_.CheckUiValue(prim->getFullId()));
        object_info_ui.local_id_value->setText(ui_helper_.CheckUiValue(prim->getLocalId()));

        if (!property_browser_)
        {
            // Init browser once
            property_browser_ = ui_helper_.CreatePropertyBrowser(object_info_widget_->GetInternal(), this, prim);
            if (property_browser_)
                object_info_ui.property_browser_layout->addWidget(property_browser_);
        }
        else
        {
            // Update browser with current prim
            current_prim_ = 0;
            foreach(QString property_name, ui_helper_.editor_items.keys())
                ui_helper_.editor_items[property_name]->setValue(prim->property(property_name.toStdString().c_str()));
        }

        // Update current prim pointer
        current_prim_ = prim;
    }

    void BuildSceneManager::PrimValueChanged(QtProperty *prop, const QVariant &value)
    {
        if (!current_prim_)
            return;

        if (value.type() == QVariant::String && prop->subProperties().count() > 0)
        {
            // Keep titles as they are, the browser doesent let us
            // say to items that they cant be modified...
            if (prop->whatsThis().endsWith(" items)"))
            {
                ui_helper_.variant_manager->setValue(prop, QVariant(prop->whatsThis()));
                return;
            }
        }

        const char* prop_name = prop->propertyName().toStdString().c_str();
        qDebug() << "Property : " << prop->propertyName();
        qDebug() << "Value    : " << value;

        current_prim_->setProperty(prop_name, value);
        qDebug() << "Current prim value for prop : " << current_prim_->property(prop_name) << endl;
    }
}