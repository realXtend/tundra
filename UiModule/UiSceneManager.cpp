// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "UiModule.h"
#include "UiSceneManager.h"
#include "UiProxyWidget.h"
#include "UiWidgetProperties.h"

#include "MainPanel/MainPanel.h"
#include "MainPanel/SettingsWidget.h"

#include <QOgreUIView.h>

#include <QRectF>
#include <QGraphicsItem>
#include <QGraphicsLinearLayout>
#include <QGraphicsWidget>

namespace UiServices
{
    UiSceneManager::UiSceneManager(Foundation::Framework *framework, QGraphicsView *ui_view) :
        QObject(),
        framework_(framework),
        ui_view_(ui_view),
        container_widget_(new QGraphicsWidget()),
        container_layout_(0),
        main_panel_(0)
    {
        if (ui_view_)
        {
            InitMasterLayout();
            SceneRectChanged(ui_view_->scene()->sceneRect());
            connect(ui_view_->scene(), SIGNAL( sceneRectChanged(const QRectF &) ),
                    this, SLOT( SceneRectChanged(const QRectF &) ));
        }
        else
        {
            SAFE_DELETE(container_widget_);
            container_layout_ = 0;
            UiModule::LogError("UiSceneManager >> Could not acquire UIView, skipping scene creation");
        }
    }

    UiSceneManager::~UiSceneManager()
    {
        if (main_panel_)
            SAFE_DELETE(main_panel_);
        main_panel_proxy_widget_ = 0;

        if (container_widget_)
            SAFE_DELETE(container_widget_);
        container_layout_ = 0;
    }

    /*************** UI Scene Manager Services ***************/

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

    UiProxyWidget* UiSceneManager::AddWidgetToCurrentScene(QWidget *widget)
    {
        return AddWidgetToCurrentScene(widget, UiWidgetProperties());
    }

    UiProxyWidget* UiSceneManager::AddWidgetToCurrentScene(QWidget *widget, const UiServices::UiWidgetProperties &widget_properties)
    {
        UiProxyWidget *proxy_widget = new UiProxyWidget(widget, widget_properties);

        if (AddProxyWidget(proxy_widget))
            return proxy_widget;
        else
            return 0;
    }

    bool UiSceneManager::AddProxyWidget(UiServices::UiProxyWidget *widget)
    {
        if (ui_view_)
        {
            UiWidgetProperties properties = widget->GetWidgetProperties();
            if (properties.IsShownAtToolbar())
            {
                CoreUi::MainPanelButton *control_button = main_panel_->AddWidget(widget, properties.GetWidgetName());
                widget->SetControlButton(control_button);
                connect(widget, SIGNAL( BringToFrontRequest(UiProxyWidget*) ), this, SLOT( BringToFront(UiProxyWidget*) ));
            }

            if (properties.IsFullscreen())
            {
                if (properties.GetWidgetName() == "Login")
                    login_proxy_widget_ = widget;
                container_layout_->addItem(widget);
            }
            else
            {
                widget->hide();
                ui_view_->scene()->addItem(widget);
            }
            return true;
        }
        else
            return false;
    }

    void UiSceneManager::RemoveProxyWidgetFromCurrentScene(UiProxyWidget *widget)
    {
        if (ui_view_)
        {
            if (main_panel_ && widget->GetWidgetProperties().IsShownAtToolbar())
                main_panel_->RemoveWidget(widget);
            ui_view_->scene()->removeItem(widget);
        }
    }

    /*************** UI Scene Manager Private functions ***************/

    void UiSceneManager::BringToFront(UiProxyWidget *widget)
    {
        if (ui_view_)
        {
            ui_view_->scene()->setActiveWindow(widget);
            ui_view_->scene()->setFocusItem(widget, Qt::ActiveWindowFocusReason);
        }
    }

    void UiSceneManager::InitMasterLayout()
    {
        // Init layout and container widget for CoreUi widgets
        container_layout_ = new QGraphicsLinearLayout(Qt::Vertical, container_widget_);
        container_layout_->setContentsMargins(0,0,0,0);
        container_layout_->setSpacing(0);
        container_widget_->setLayout(container_layout_);
        ui_view_->scene()->addItem(container_widget_);

        // Init main panel
        main_panel_ = new CoreUi::MainPanel(framework_);
        main_panel_proxy_widget_ = new UiProxyWidget(main_panel_->GetWidget(), UiWidgetProperties("MainPanel", true));

        // Init settings widget, add control button to mainpanel
        settings_widget_ = new CoreUi::SettingsWidget();

        UiWidgetProperties widget_properties("Settings", UiServices::SlideFromTop, settings_widget_->size());
        widget_properties.SetShowAtToolbar(false);
        settings_widget_proxy_widget_ = new UiProxyWidget(settings_widget_, widget_properties);
        CoreUi::MainPanelButton *control_button = main_panel_->SetSettingsWidget(settings_widget_proxy_widget_, "Settings");
        settings_widget_proxy_widget_->SetControlButton(control_button);

        AddProxyWidget(settings_widget_proxy_widget_);

        connect(settings_widget_proxy_widget_, SIGNAL( BringToFrontRequest(UiProxyWidget*) ), 
                this, SLOT( BringToFront(UiProxyWidget*) ));
        connect(settings_widget_, SIGNAL( NewUserInterfaceSettingsApplied(int, int) ),
                this, SLOT( ApplyNewProxySettings(int, int) ));
    }

    void UiSceneManager::SceneRectChanged(const QRectF &new_scene_rect)
    {
        if (container_widget_)
            container_widget_->setGeometry(new_scene_rect);
    }

    void UiSceneManager::Connect()
    {
        ClearContainerLayout();
        container_layout_->insertItem(0, main_panel_proxy_widget_);
        login_proxy_widget_->hide();
        main_panel_proxy_widget_->show();

        emit Connected();
    }

    void UiSceneManager::Disconnect()
    {
        ClearContainerLayout();
        container_layout_->insertItem(0, login_proxy_widget_);
        main_panel_proxy_widget_->hide();
        login_proxy_widget_->show();

        if (ui_view_)
            SceneRectChanged(ui_view_->scene()->sceneRect());

        emit Disconnected();
    }

    void UiSceneManager::ClearContainerLayout()
    {
        for (int index = 0; index < container_layout_->count(); ++index)
            container_layout_->removeAt(index);
    }

    QList<UiProxyWidget *> UiSceneManager::GetAllProxyWidgets()
    {
        QList<UiProxyWidget *> widget_list = QList<UiProxyWidget *>();
        if (ui_view_)
        {
            QList<QGraphicsItem *> graphics_items = ui_view_->scene()->items();
            foreach(QGraphicsItem *widget, graphics_items)
            {
                UiProxyWidget *proxy_widget = dynamic_cast<UiProxyWidget *>(widget);
                if (proxy_widget)
                    widget_list.append(proxy_widget);
            }
        }

        widget_list.removeAt(widget_list.indexOf(main_panel_proxy_widget_));
        widget_list.removeAt(widget_list.indexOf(login_proxy_widget_));

        return widget_list;
    }

    UiProxyWidget *UiSceneManager::GetProxyWidget(const QString &widget_name)
    {
        QList<UiProxyWidget *> widget_list = QList<UiProxyWidget *>();
        if (ui_view_)
        {
            QList<QGraphicsItem *> graphics_items = ui_view_->scene()->items();
            foreach(QGraphicsItem *widget, graphics_items)
            {
                UiProxyWidget *proxy_widget = dynamic_cast<UiProxyWidget *>(widget);
                if (proxy_widget)
                {
                    if (proxy_widget->GetWidgetProperties().GetWidgetName() == widget_name)
                        return proxy_widget;
                }
            }
        }
        return 0;
    }

    void UiSceneManager::ApplyNewProxySettings(int new_opacity, int new_animation_speed)
    {
        if (main_panel_)
        {
            QList<UiProxyWidget *> all_proxy_widgets_ = main_panel_->GetProxyWidgetList();
            foreach(UiProxyWidget *widget, all_proxy_widgets_)
            {
                widget->SetUnfocusedOpacity(new_opacity);
                widget->SetShowAnimationSpeed(new_animation_speed);
            }
        }
    }
}
