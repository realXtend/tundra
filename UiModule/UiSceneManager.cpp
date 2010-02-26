// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "DebugOperatorNew.h"
#include "UiModule.h"
#include "UiSceneManager.h"
#include "UiProxyWidget.h"
#include "UiWidgetProperties.h"

#include "MainPanel/MainPanel.h"
#include "MainPanel/SettingsWidget.h"
#include "Login/TraditionalLoginWidget.h"

#include <QRectF>
#include <QGraphicsItem>
#include <QGraphicsLinearLayout>
#include <QGraphicsWidget>
#include <QGraphicsView>
#include <QGraphicsScene>

#include "MemoryLeakCheck.h"

namespace UiServices
{
    UiSceneManager::UiSceneManager(Foundation::Framework *framework, QGraphicsView *ui_view) 
        : QObject(),
          framework_(framework),
          ui_view_(ui_view),
          inworld_scene_(0),
          container_widget_(new QGraphicsWidget()),
          container_layout_(0),
          main_panel_(0),
          login_proxy_widget_(0)
    {
        if (ui_view_)
        {
            inworld_scene_ = ui_view_->scene();
            InitMasterLayout();
            SceneRectChanged(inworld_scene_->sceneRect());
            connect(inworld_scene_, SIGNAL( sceneRectChanged(const QRectF &) ),
                    this, SLOT( SceneRectChanged(const QRectF &) ));
        }
        else
        {
            SAFE_DELETE(container_widget_);
            container_layout_ = 0;
            UiModule::LogError("UiSceneManager: Could not acquire UIView, skipping scene creation");
        }
    }

    UiSceneManager::~UiSceneManager()
    {
        if (main_panel_)
            SAFE_DELETE(main_panel_);
        main_panel_proxy_widget_ = 0;

        if (settings_widget_)
            SAFE_DELETE(settings_widget_);
        main_panel_proxy_widget_ = 0;

        if (container_widget_)
            SAFE_DELETE(container_widget_);
        container_layout_ = 0;

        login_proxy_widget_ = 0;
    }

    /*************** UI Scene Manager Public Services ***************/

    bool UiSceneManager::AddSettingsWidget(QWidget *settings_widget, const QString &tab_name)
    {
        if (settings_widget_)
        {
            settings_widget_->AddWidget(settings_widget, tab_name);
            return true;
        }
        else
            return false;
    }

    UiProxyWidget* UiSceneManager::AddWidgetToScene(QWidget *widget)
    {
        return AddWidgetToScene(widget, UiWidgetProperties("Unnamed Widget", ModuleWidget));
    }

    UiProxyWidget* UiSceneManager::AddWidgetToScene(QWidget *widget, const UiServices::UiWidgetProperties &widget_properties)
    {
        UiProxyWidget *proxy_widget = new UiProxyWidget(widget, widget_properties);
        if (AddProxyWidget(proxy_widget))
            return proxy_widget;
        else
            return 0;
    }

    bool UiSceneManager::AddProxyWidget(UiServices::UiProxyWidget *proxy_widget)
    {
        if (ui_view_)
        {
            UiWidgetProperties properties = proxy_widget->GetWidgetProperties();
            if (properties.GetWidgetType() == CoreLayoutWidget)
                container_layout_->addItem(proxy_widget);
            else
            {
                proxy_widget->hide();
                inworld_scene_->addItem(proxy_widget);
                if (properties.IsShownInToolbar())
                {
                    CoreUi::MainPanelButton *control_button = main_panel_->AddWidget(proxy_widget, properties.GetWidgetName());
                    proxy_widget->SetControlButton(control_button);
                    connect(proxy_widget, SIGNAL( BringProxyToFrontRequest(UiProxyWidget*) ), this, SLOT( BringProxyToFront(UiProxyWidget*) ));
                }
            }
            return true;
        }
        else
        {
            SAFE_DELETE(proxy_widget);
            return false;
        }
    }

    void UiSceneManager::RemoveProxyWidgetFromScene(UiServices::UiProxyWidget *proxy_widget)
    {
        if (ui_view_)
        {
            if (main_panel_ && proxy_widget->GetWidgetProperties().IsShownInToolbar())
                main_panel_->RemoveWidget(proxy_widget);
            inworld_scene_->removeItem(proxy_widget);
        }
    }

    void UiSceneManager::RemoveProxyWidgetFromScene(QWidget *widget)
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

    void UiSceneManager::BringProxyToFront(UiProxyWidget *widget)
    {
        if (ui_view_)
        {
            inworld_scene_->setActiveWindow(widget);
            inworld_scene_->setFocusItem(widget, Qt::ActiveWindowFocusReason);
        }
    }

    void UiSceneManager::BringProxyToFront(QWidget *widget)
    {
        if (ui_view_)
        {
            inworld_scene_->setActiveWindow(widget->graphicsProxyWidget());
            inworld_scene_->setFocusItem(widget->graphicsProxyWidget(), Qt::ActiveWindowFocusReason);
        }
    }

    /*************** Public Functions But Use With Caution ****************/
    /** As in don't touch if you are not 100% sure on what you are doing **/

    void UiSceneManager::SetDemoLoginWidget(QWidget *widget)
    {
        if (!login_proxy_widget_)
            return;
        
        QTabWidget *tab_widget = login_proxy_widget_->widget()->findChild<QTabWidget *>("tabWidget");
        if (!tab_widget)
            return;

        CoreUi::TraditionalLoginWidget *traditional_login = dynamic_cast<CoreUi::TraditionalLoginWidget *>(tab_widget->widget(0));
        if (traditional_login)
        {
            traditional_login->GetUi().demoWorldsContainer->addWidget(widget);
            traditional_login->GetUi().DemoWorldFrame->show();
            traditional_login->GetUi().demoWorldLabel->show();
        }
    }

    void UiSceneManager::Connected()
    {
        if (login_proxy_widget_ && main_panel_proxy_widget_)
        {
            ClearContainerLayout();
            container_layout_->insertItem(0, main_panel_proxy_widget_);
            login_proxy_widget_->hide();
            main_panel_proxy_widget_->show();
        }
        emit UiStateChangeConnected();
    }

    void UiSceneManager::Disconnected()
    {
        if (main_panel_proxy_widget_ && login_proxy_widget_)
        {
            ClearContainerLayout();
            container_layout_->insertItem(0, login_proxy_widget_);
            main_panel_proxy_widget_->hide();
            login_proxy_widget_->show();
        }
        if (ui_view_)
            SceneRectChanged(inworld_scene_->sceneRect());
        emit UiStateChangeDisconnected();
    }

    /*************** UI Scene Manager Private functions ***************/

    void UiSceneManager::InitMasterLayout()
    {
        // Init layout and container widget for CoreUi widgets
        container_layout_ = new QGraphicsLinearLayout(Qt::Vertical, container_widget_);
        container_layout_->setContentsMargins(0,0,0,0);
        container_layout_->setSpacing(0);
        container_widget_->setLayout(container_layout_);
        inworld_scene_->addItem(container_widget_);

        // Init main panel
        main_panel_ = new CoreUi::MainPanel(framework_);
        main_panel_proxy_widget_ = new UiProxyWidget(main_panel_->GetWidget(), UiWidgetProperties("MainPanel", UiServices::CoreLayoutWidget));

        // Init settings widget, add control button to mainpanel
        settings_widget_ = new CoreUi::SettingsWidget();
        settings_proxy_widget_ = new UiProxyWidget(settings_widget_, UiWidgetProperties("Settings", UiServices::SceneWidget));
        CoreUi::MainPanelButton *control_button = main_panel_->SetSettingsWidget(settings_proxy_widget_, "Settings");
        settings_proxy_widget_->SetControlButton(control_button);

        AddProxyWidget(settings_proxy_widget_);

        connect(settings_proxy_widget_, SIGNAL( BringProxyToFrontRequest(UiProxyWidget*) ), 
                this, SLOT( BringProxyToFront(UiProxyWidget*) ));
        connect(settings_widget_, SIGNAL( NewUserInterfaceSettingsApplied(int, int) ),
                this, SLOT( ApplyNewProxySettings(int, int) ));
    }

    void UiSceneManager::ClearContainerLayout()
    {
        for (int index = 0; index < container_layout_->count(); ++index)
            container_layout_->removeAt(index);
    }

    /*************** UI Scene Manager Private slots ***************/

    void UiSceneManager::ApplyNewProxySettings(int new_opacity, int new_animation_speed)
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

    void UiSceneManager::SceneRectChanged(const QRectF &new_scene_rect)
    {
        if (container_widget_)
            container_widget_->setGeometry(new_scene_rect);
    }
}
