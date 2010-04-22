// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "DebugOperatorNew.h"
#include "UiDefines.h"

#include "InworldSceneController.h"
#include "ControlPanelManager.h"

#include "Common/AnchorLayoutManager.h"

#include "Menus/MenuManager.h"

#include "View/UiProxyWidget.h"
#include "View/UiWidgetProperties.h"
#include "View/CommunicationWidget.h"

#include "Inworld/ControlPanel/SettingsWidget.h"
#include "Inworld/ControlPanel/PersonalWidget.h"

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
          communication_widget_(0)
    {
        if (!ui_view_)
            return;

        // Store scene pointer
        inworld_scene_ = ui_view_->scene();

        // Init layout manager with scene
        layout_manager_ = new CoreUi::AnchorLayoutManager(this, inworld_scene_);

        // Init UI managers with layout manager
        control_panel_manager_ = new CoreUi::ControlPanelManager(this, layout_manager_);
        menu_manager_ = new CoreUi::MenuManager(this, layout_manager_);
        
        // Communication core UI
        communication_widget_ = new CoreUi::CommunicationWidget();
        layout_manager_->AddCornerAnchor(communication_widget_, Qt::BottomLeftCorner, Qt::BottomLeftCorner);

        // Connect settings widget
        connect(control_panel_manager_->GetSettingsWidget(), SIGNAL(NewUserInterfaceSettingsApplied(int, int)), SLOT(ApplyNewProxySettings(int, int)));
		
		//catch SceneResized(const QRectF) signal when resized and reposition active proxy widgets
		connect(layout_manager_, SIGNAL(SceneResized(const QRectF)), this, SLOT(ApplyNewProxyPosition(const QRectF)));
    }

    InworldSceneController::~InworldSceneController()
    {
        SAFE_DELETE(communication_widget_);
    }

    /*************** UI Scene Manager Public Services ***************/

    bool InworldSceneController::AddSettingsWidget(QWidget *settings_widget, const QString &tab_name) const
    {
        control_panel_manager_->GetSettingsWidget()->AddWidget(settings_widget, tab_name);
        return true;
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
        if (!inworld_scene_)
        {
            SAFE_DELETE(proxy_widget);
            return false;
        }

        // Add to scene
        if (proxy_widget->isVisible())
            proxy_widget->hide();
        inworld_scene_->addItem(proxy_widget);

        // Add to internal control list
        if (!all_proxy_widgets_in_scene_.contains(proxy_widget))
            all_proxy_widgets_in_scene_.append(proxy_widget);

        // Add to menu structure if its needed
        UiWidgetProperties properties = proxy_widget->GetWidgetProperties();
        if (properties.IsShownInToolbar())
        {
            QString widget_name = properties.GetWidgetName();
            if (widget_name == "Inventory")
                control_panel_manager_->GetPersonalWidget()->SetInventoryWidget(proxy_widget);
            else if (widget_name == "Avatar Editor")
                control_panel_manager_->GetPersonalWidget()->SetAvatarWidget(proxy_widget);
            else
                menu_manager_->AddMenuItem(CoreUi::MenuManager::Root, proxy_widget, properties);

            connect(proxy_widget, SIGNAL( BringProxyToFrontRequest(UiProxyWidget*) ), this, SLOT( BringProxyToFront(UiProxyWidget*) ));
        }
        return true;
    }

    void InworldSceneController::RemoveProxyWidgetFromScene(UiServices::UiProxyWidget *proxy_widget)
    {
        if (!inworld_scene_)
            return;
        
        QString widget_name = proxy_widget->GetWidgetProperties().GetWidgetName();
        if (widget_name != "Inventory" && widget_name != "Avatar Editor")
            menu_manager_->RemoveMenuItem(CoreUi::MenuManager::Root, proxy_widget);
        inworld_scene_->removeItem(proxy_widget);
        all_proxy_widgets_in_scene_.removeOne(proxy_widget);
    }

    void InworldSceneController::RemoveProxyWidgetFromScene(QWidget *widget)
    {
        UiProxyWidget *proxy_widget = dynamic_cast<UiProxyWidget*>(widget->graphicsProxyWidget());
        if (proxy_widget)
            RemoveProxyWidgetFromScene(proxy_widget);
    }

    void InworldSceneController::BringProxyToFront(UiProxyWidget *widget) const
    {
        if (!inworld_scene_ || !inworld_scene_->isActive())
            return;
        inworld_scene_->setActiveWindow(widget);
        inworld_scene_->setFocusItem(widget, Qt::ActiveWindowFocusReason);
    }

    void InworldSceneController::BringProxyToFront(QWidget *widget) const
    {
        if (!inworld_scene_)
            return;
        ShowProxyForWidget(widget);
        inworld_scene_->setActiveWindow(widget->graphicsProxyWidget());
        inworld_scene_->setFocusItem(widget->graphicsProxyWidget(), Qt::ActiveWindowFocusReason);
    }

    void InworldSceneController::ShowProxyForWidget(QWidget *widget) const
    {
        if (inworld_scene_)
            widget->graphicsProxyWidget()->show();
    }

    void InworldSceneController::HideProxyForWidget(QWidget *widget) const
    {
        if (inworld_scene_)
            widget->graphicsProxyWidget()->hide();
    }

    QObject *InworldSceneController::GetSettingsObject() const
    {
        return dynamic_cast<QObject *>(control_panel_manager_->GetSettingsWidget());
    }

    void InworldSceneController::SetFocusToChat() const
    {
        if (communication_widget_)
            communication_widget_->SetFocusToChat();
    }

    // Don't touch, please

    void InworldSceneController::SetWorldChatController(QObject *controller) const
    {
        if (communication_widget_)
            communication_widget_->UpdateController(controller);
    }

    void InworldSceneController::SetImWidget(UiProxyWidget *im_proxy) const
    {
        if (communication_widget_)
            communication_widget_->UpdateImWidget(im_proxy);
    }

    // Private

    void InworldSceneController::ApplyNewProxySettings(int new_opacity, int new_animation_speed) const
    {
        foreach (UiProxyWidget *widget, all_proxy_widgets_in_scene_)
        {
            widget->SetUnfocusedOpacity(new_opacity);
            widget->SetShowAnimationSpeed(new_animation_speed);
        }
    }

	//Applying new proxy position
	void InworldSceneController::ApplyNewProxyPosition(const QRectF &new_rect)
	{
		foreach (UiProxyWidget *widget, all_proxy_widgets_in_scene_)
        {
			if(widget->isVisible())
			{
				if (!new_rect.contains(widget->geometry()))
				{
					widget->setX(qMin(new_rect.right() - widget->size().width(), qMax(widget->x(), new_rect.left() + widget->size().width() )));
					widget->setY(qMin(new_rect.bottom() - widget->size().height(), qMax(widget->y(), new_rect.top()+ - widget->size().height())));
				}
			}
        }
	}
}
