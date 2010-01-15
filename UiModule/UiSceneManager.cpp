// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "UiModule.h"
#include "UiSceneManager.h"
#include "UiProxyWidget.h"
#include "UiWidgetProperties.h"

#include "MainPanel/MainPanel.h"

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
        container_widget_(new QGraphicsWidget())
    {
        if (ui_view_)
        {
            InitMasterLayout();
            SceneRectChanged(ui_view_->scene()->sceneRect());
            QObject::connect(ui_view_->scene(), SIGNAL(sceneRectChanged(const QRectF &)),
                this, SLOT(SceneRectChanged(const QRectF &)));
        }
        else
        {
            SAFE_DELETE(container_widget_);
            SAFE_DELETE(container_layout_);
            UiModule::LogError("UiSceneManager >> Could not acquire UIView, skipping scene creation");
        }
    }

    UiSceneManager::~UiSceneManager()
    {
        SAFE_DELETE(main_panel_);
        main_panel_proxy_widget_ = 0;
        SAFE_DELETE(container_widget_);
        container_layout_ = 0;
    }

    /*************** UI Scene Manager Services ***************/

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
            UiWidgetProperties properties = widget->getWidgetProperties();
            if (properties.IsShownAtToolbar())
                main_panel_->AddWidget(widget, properties.GetWidgetName());

            if (properties.IsFullscreen())
            {
                if (properties.GetWidgetName() == "Login")
                    login_widget_ = widget;
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

    /*************** UI Scene Manager Private functions ***************/

    void UiSceneManager::RemoveProxyWidgetFromCurrentScene(UiProxyWidget *widget)
    {
        ui_view_->scene()->removeItem((QGraphicsItem *)widget);
    }

    void UiSceneManager::InitMasterLayout()
    {
        container_layout_ = new QGraphicsLinearLayout(Qt::Vertical, container_widget_);
        container_layout_->setContentsMargins(0,0,0,0);
        container_layout_->setSpacing(0);
        container_widget_->setLayout(container_layout_);
        ui_view_->scene()->addItem(container_widget_);

        main_panel_ = new CoreUi::MainPanel(framework_);
        main_panel_proxy_widget_ = new UiProxyWidget(main_panel_->GetWidget(), UiWidgetProperties("MainPanel", true));
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
        login_widget_->hide();
        main_panel_proxy_widget_->show();

        emit Connected();
    }

    void UiSceneManager::Disconnect()
    {
        ClearContainerLayout();
        container_layout_->insertItem(0, login_widget_);
        main_panel_proxy_widget_->hide();
        login_widget_->show();

        if (ui_view_)
            SceneRectChanged(ui_view_->scene()->sceneRect());

        emit Disconnected();
    }

    void UiSceneManager::ClearContainerLayout()
    {
        for(int index = 0; index < container_layout_->count(); ++index)
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
        widget_list.removeAt(widget_list.indexOf(login_widget_));

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
                    if (proxy_widget->getWidgetProperties().GetWidgetName() == widget_name)
                        return proxy_widget;
                }
            }
        }
        return 0;
    }
}
