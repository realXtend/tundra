// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "DebugOperatorNew.h"
#include "UiModule.h"

#include "InworldSceneController.h"
#include "Common/AnchorLayoutManager.h"
#include "View/UiProxyWidget.h"
#include "View/UiWidgetProperties.h"
#include "View/CommunicationWidget.h"
#include "Menus/MenuManager.h"

#include "MainPanel/MainPanel.h"
#include "MainPanel/SettingsWidget.h"

#include <QRectF>
#include <QGraphicsItem>
#include <QGraphicsLinearLayout>
#include <QGraphicsWidget>
#include <QGraphicsView>
#include <QGraphicsScene>

#include "MemoryLeakCheck.h"

namespace UiServices
{
    InworldSceneController::InworldSceneController(Foundation::Framework *framework, QGraphicsView *ui_view) 
        : QObject(),
          framework_(framework),
          ui_view_(ui_view),
          inworld_scene_(0),
          main_panel_widget_(0),
          communication_widget_(0),
          layout_manager_(0),
          menu_manager_(0)
    {
        if (ui_view_)
        {
            inworld_scene_ = ui_view_->scene();
            layout_manager_ = new CoreUi::AnchorLayoutManager(this, inworld_scene_);
            menu_manager_ = new CoreUi::MenuManager(this, layout_manager_);
            InitInternals();
        }
        else
            UiModule::LogError("InworldSceneController: Could not acquire UIView, skipping scene creation");
    }

    InworldSceneController::~InworldSceneController()
    {
        if (main_panel_widget_)
            SAFE_DELETE(main_panel_widget_);

        if (communication_widget_)
            SAFE_DELETE(communication_widget_);

        if (settings_widget_)
            SAFE_DELETE(settings_widget_);
        settings_proxy_widget_ = 0;
    }

    /*************** UI Scene Manager Public Services ***************/

    bool InworldSceneController::AddSettingsWidget(QWidget *settings_widget, const QString &tab_name)
    {
        if (settings_widget_)
        {
            settings_widget_->AddWidget(settings_widget, tab_name);
            return true;
        }
        else
            return false;
    }

    UiProxyWidget* InworldSceneController::AddWidgetToScene(QWidget *widget)
    {
        return AddWidgetToScene(widget, UiWidgetProperties("Unnamed Widget", ModuleWidget));
    }

    UiProxyWidget* InworldSceneController::AddWidgetToScene(QWidget *widget, const UiServices::UiWidgetProperties &widget_properties)
    {
        UiProxyWidget *proxy_widget = new UiProxyWidget(widget, widget_properties);
        if (AddProxyWidget(proxy_widget))
            return proxy_widget;
        else
            return 0;
    }

    bool InworldSceneController::AddProxyWidget(UiServices::UiProxyWidget *proxy_widget)
    {
        if (ui_view_)
        {
            // Add to scene
            proxy_widget->hide();
            inworld_scene_->addItem(proxy_widget);

            // Add to internal control list
            if (!all_proxy_widgets_in_scene_.contains(proxy_widget))
                all_proxy_widgets_in_scene_.append(proxy_widget);

            // Add to menu structure if its needed
            UiWidgetProperties properties = proxy_widget->GetWidgetProperties();
            if (properties.IsShownInToolbar())
            {
                // Small hack here with grouping by widget name, will go away when inv and avatar widgets will not be in this menu anymore
                if (properties.GetWidgetName() != "Inventory" && 
                    properties.GetWidgetName() != "Avatar Editor")
                    menu_manager_->AddMenuItem(CoreUi::MenuManager::Building, proxy_widget, properties.GetWidgetName());
                else
                    menu_manager_->AddMenuItem(CoreUi::MenuManager::Personal, proxy_widget, properties.GetWidgetName());
                connect(proxy_widget, SIGNAL( BringProxyToFrontRequest(UiProxyWidget*) ), this, SLOT( BringProxyToFront(UiProxyWidget*) ));
            }
            return true;
        }
        else
        {
            SAFE_DELETE(proxy_widget);
            return false;
        }
    }

    void InworldSceneController::RemoveProxyWidgetFromScene(UiServices::UiProxyWidget *proxy_widget)
    {
        if (ui_view_)
        {
            // Small hack here with grouping by widget name, will go away when inv and avatar widgets will not be in this menu anymore
            if (proxy_widget->GetWidgetProperties().GetWidgetName() != "Inventory" && 
                proxy_widget->GetWidgetProperties().GetWidgetName() != "Avatar Editor")
                menu_manager_->RemoveMenuItem(CoreUi::MenuManager::Building, proxy_widget);
            else
                menu_manager_->RemoveMenuItem(CoreUi::MenuManager::Personal, proxy_widget);
            inworld_scene_->removeItem(proxy_widget);
            all_proxy_widgets_in_scene_.removeOne(proxy_widget);
        }
    }

    void InworldSceneController::RemoveProxyWidgetFromScene(QWidget *widget)
    {
        UiProxyWidget *proxy_widget = dynamic_cast<UiProxyWidget*>(widget->graphicsProxyWidget());
        if (proxy_widget)
            RemoveProxyWidgetFromScene(proxy_widget);
    }

    void InworldSceneController::BringProxyToFront(UiProxyWidget *widget)
    {
        if (ui_view_)
        {
            inworld_scene_->setActiveWindow(widget);
            inworld_scene_->setFocusItem(widget, Qt::ActiveWindowFocusReason);
        }
    }

    void InworldSceneController::BringProxyToFront(QWidget *widget)
    {
        if (ui_view_)
        {
            inworld_scene_->setActiveWindow(widget->graphicsProxyWidget());
            inworld_scene_->setFocusItem(widget->graphicsProxyWidget(), Qt::ActiveWindowFocusReason);
        }
    }

    /*************** Public Functions But Use With Caution ****************/
    /** As in don't touch if you are not 100% sure on what you are doing **/

    void InworldSceneController::SetWorldChatController(QObject *controller)
    {
        if (communication_widget_)
            communication_widget_->UpdateController(controller);
    }

    void InworldSceneController::SetImWidget(UiProxyWidget *im_proxy)
    {
        if (communication_widget_)
            communication_widget_->UpdateImWidget(im_proxy);
    }

    /*************** UI Scene Manager Private functions ***************/

    void InworldSceneController::InitInternals()
    {
        // Communication core UI
        communication_widget_ = new CoreUi::CommunicationWidget();
        layout_manager_->AddCornerAnchor(communication_widget_, Qt::BottomLeftCorner, Qt::BottomLeftCorner);

        // Main Panel core UI
        main_panel_widget_ = new CoreUi::MainPanel();
        layout_manager_->AddCornerAnchor(main_panel_widget_, Qt::TopRightCorner, Qt::TopRightCorner);

        // Settings widget public service UI
        settings_widget_ = new CoreUi::SettingsWidget();
        settings_proxy_widget_ = new UiProxyWidget(settings_widget_, UiWidgetProperties("Settings", UiServices::SceneWidget));
        connect(settings_proxy_widget_, SIGNAL( BringProxyToFrontRequest(UiProxyWidget*) ), SLOT( BringProxyToFront(UiProxyWidget*) ));
        connect(settings_widget_, SIGNAL( NewUserInterfaceSettingsApplied(int, int) ), SLOT( ApplyNewProxySettings(int, int) ));

        CoreUi::MainPanelButton *control_button = main_panel_widget_->SetSettingsWidget(settings_proxy_widget_, "Settings");
        settings_proxy_widget_->SetControlButton(control_button);
        AddProxyWidget(settings_proxy_widget_);
    }

    /*************** UI Scene Manager Private slots ***************/

    void InworldSceneController::ApplyNewProxySettings(int new_opacity, int new_animation_speed)
    {
        if (main_panel_widget_)
        {
            foreach (UiProxyWidget *widget, all_proxy_widgets_in_scene_)
            {
                widget->SetUnfocusedOpacity(new_opacity);
                widget->SetShowAnimationSpeed(new_animation_speed);
            }
        }
    }
}
