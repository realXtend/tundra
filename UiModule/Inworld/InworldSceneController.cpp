// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "DebugOperatorNew.h"
#include "UiModule.h"

#include "InworldSceneController.h"
#include "Common/AnchorLayoutManager.h"
#include "View/UiProxyWidget.h"
#include "View/UiWidgetProperties.h"
#include "View/CommunicationWidget.h"

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
          main_panel_(0),
          communication_widget_(0)
    {
        if (ui_view_)
        {
            inworld_scene_ = ui_view_->scene();
            layout_manager_ = new CoreUi::AnchorLayoutManager(this, inworld_scene_);
            InitInternals();
        }
        else
            UiModule::LogError("InworldSceneController: Could not acquire UIView, skipping scene creation");
    }

    InworldSceneController::~InworldSceneController()
    {
        if (main_panel_)
            SAFE_DELETE(main_panel_);
        main_panel_proxy_widget_ = 0;

        if (settings_widget_)
            SAFE_DELETE(settings_widget_);
        settings_proxy_widget_ = 0;

        if (communication_widget_)
            SAFE_DELETE(communication_widget_);
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
            proxy_widget->hide();
            inworld_scene_->addItem(proxy_widget);

            UiWidgetProperties properties = proxy_widget->GetWidgetProperties();
            if (properties.IsShownInToolbar())
            {
                CoreUi::MainPanelButton *control_button = main_panel_->AddWidget(proxy_widget, properties.GetWidgetName());
                proxy_widget->SetControlButton(control_button);
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
            if (main_panel_ && proxy_widget->GetWidgetProperties().IsShownInToolbar())
                main_panel_->RemoveWidget(proxy_widget);
            inworld_scene_->removeItem(proxy_widget);
        }
    }

    void InworldSceneController::RemoveProxyWidgetFromScene(QWidget *widget)
    {
        if (ui_view_)
        {
            QList<UiProxyWidget *> proxies(main_panel_->GetProxyWidgetList());
            foreach (UiProxyWidget *proxy, proxies)
            {
                QGraphicsProxyWidget *qproxy = dynamic_cast<QGraphicsProxyWidget *>(proxy);
                if (qproxy && qproxy == widget->graphicsProxyWidget())
                    if (main_panel_ && proxy->GetWidgetProperties().IsShownInToolbar())
                        main_panel_->RemoveWidget(proxy);
            }

            inworld_scene_->removeItem(widget->graphicsProxyWidget());
        }
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

    void InworldSceneController::SetDemoLoginWidget(QWidget *widget)
    {
        //// Remove this code
        //if (!login_proxy_widget_)
        //    return;
        //
        //QTabWidget *tab_widget = login_proxy_widget_->widget()->findChild<QTabWidget *>("tabWidget");
        //if (!tab_widget)
        //    return;

        //CoreUi::TraditionalLoginWidget *traditional_login = dynamic_cast<CoreUi::TraditionalLoginWidget *>(tab_widget->widget(0));
        //if (traditional_login)
        //{
        //    traditional_login->GetUi().demoWorldsContainer->addWidget(widget);
        //    traditional_login->GetUi().DemoWorldFrame->show();
        //    traditional_login->GetUi().demoWorldLabel->show();
        //}
    }

    /*************** UI Scene Manager Private functions ***************/

    void InworldSceneController::InitInternals()
    {
        // Init main panel
        main_panel_ = new CoreUi::MainPanel(framework_);
        main_panel_proxy_widget_ = new UiProxyWidget(main_panel_->GetWidget(), UiWidgetProperties("MainPanel", UiServices::CoreLayoutWidget));
        layout_manager_->AddCornerAnchor(main_panel_proxy_widget_, Qt::TopLeftCorner, Qt::TopLeftCorner);
        layout_manager_->AddCornerAnchor(main_panel_proxy_widget_, Qt::TopRightCorner, Qt::TopRightCorner);

        // Init settings widget, add control button to mainpanel
        settings_widget_ = new CoreUi::SettingsWidget();
        settings_proxy_widget_ = new UiProxyWidget(settings_widget_, UiWidgetProperties("Settings", UiServices::SceneWidget));
        connect(settings_proxy_widget_, SIGNAL( BringProxyToFrontRequest(UiProxyWidget*) ), SLOT( BringProxyToFront(UiProxyWidget*) ));
        connect(settings_widget_, SIGNAL( NewUserInterfaceSettingsApplied(int, int) ), SLOT( ApplyNewProxySettings(int, int) ));

        // Do this part better after refactor!
        CoreUi::MainPanelButton *control_button = main_panel_->SetSettingsWidget(settings_proxy_widget_, "Settings");
        settings_proxy_widget_->SetControlButton(control_button);
        AddProxyWidget(settings_proxy_widget_);

        // Communication core UI
        communication_widget_ = new CoreUi::CommunicationWidget();
        layout_manager_->AddCornerAnchor(communication_widget_, Qt::BottomLeftCorner, Qt::BottomLeftCorner);

    }

    /*************** UI Scene Manager Private slots ***************/

    void InworldSceneController::ApplyNewProxySettings(int new_opacity, int new_animation_speed)
    {
        if (main_panel_)
        {
            QList<UiProxyWidget *> all_proxy_widgets_ = main_panel_->GetProxyWidgetList();
            foreach (UiProxyWidget *widget, all_proxy_widgets_)
            {
                widget->SetUnfocusedOpacity(new_opacity);
                widget->SetShowAnimationSpeed(new_animation_speed);
            }
        }
    }
}
