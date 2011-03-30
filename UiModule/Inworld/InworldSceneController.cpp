// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#include "InworldSceneController.h"
#include "ControlPanelManager.h"
#include "Common/AnchorLayoutManager.h"
#include "Menus/MenuManager.h"
#include "Inworld/ControlPanel/SettingsWidget.h"
#include "UiProxyWidget.h"

#include "MemoryLeakCheck.h"
#include "TundraLogicModule.h"
#include "Client.h"
#include "Renderer.h"

#include <QGraphicsView>

#define DOCK_WIDTH          (300)
#define DIST_FROM_BOTTOM    (200)
#define DIST_FROM_TOP       (50)

namespace UiServices
{
    InworldSceneController::InworldSceneController(Foundation::Framework *framework, QGraphicsView *ui_view) :
          framework_(framework),
          ui_view_(ui_view),
		  docking_widget_(0)
    {
        assert(ui_view_);

        // Store scene pointer
        inworld_scene_ = ui_view_->scene();
        assert(inworld_scene_);

        // Init layout manager with scene
        layout_manager_ = new CoreUi::AnchorLayoutManager(this, inworld_scene_);
		boost::shared_ptr<TundraLogic::Client> client=framework_->GetModule<TundraLogic::TundraLogicModule>()->GetClient();
		connect(client.get(), SIGNAL(Disconnected()), SLOT(worldDisconnected()));

        // Init UI managers with layout manager
        control_panel_manager_ = new CoreUi::ControlPanelManager(this, layout_manager_);
        menu_manager_ = new CoreUi::MenuManager(this, layout_manager_);
                
        // Connect settings widget
        connect(control_panel_manager_->GetSettingsWidget(), SIGNAL(NewUserInterfaceSettingsApplied(int, int)), SLOT(ApplyNewProxySettings(int, int)));
        
        // Apply new positions to active widgets when the inworld_scene_ is resized
        connect(inworld_scene_, SIGNAL(sceneRectChanged(const QRectF)), this, SLOT(ApplyNewProxyPosition(const QRectF)));
		//OgreRenderer::Renderer *renderer = framework_->GetService<OgreRenderer::Renderer>();
		boost::shared_ptr<OgreRenderer::Renderer> renderer = framework_->GetServiceManager()->GetService<OgreRenderer::Renderer>(Service::ST_Renderer).lock();
		if(renderer)
			connect(renderer.get(),SIGNAL(resizeWindow()),this,SLOT(AddInternalWidgets()));
	}

    InworldSceneController::~InworldSceneController()
    {
		SAFE_DELETE(docking_widget_);
    }

    UiProxyWidget *InworldSceneController::AddWidgetToScene(QWidget *widget,  Qt::WindowFlags flags)
    {
        /*  QGraphicsProxyWidget maintains symmetry for the following states:
         *  state, enabled, visible, geometry, layoutDirection, style, palette,
         *  font, cursor, sizeHint, getContentsMargins and windowTitle
         */

        UiProxyWidget *proxy = new UiProxyWidget(widget, flags);
        if (!AddProxyWidget(proxy))
        {
            SAFE_DELETE(proxy);
            return 0;
        }


        // If the widget has WA_DeleteOnClose on, connect its proxy's visibleChanged()
        // signal to a slot which handles the deletion. This must be done because closing
        // proxy window in our system doesn't yield closeEvent, but hideEvent instead.
        if (widget->testAttribute(Qt::WA_DeleteOnClose))
            connect(proxy, SIGNAL(visibleChanged()), SLOT(DeleteCallingWidgetOnClose()));

        return proxy;
    }

    bool InworldSceneController::AddProxyWidget(UiProxyWidget *widget)
    {
        // Add to scene
        if (widget->isVisible())
           widget->hide();

        // If no position has been set for Qt::Dialog widget, use default one so that the window's title
        // bar - or any other critical part, doesn't go outside the view.
        if ((widget->windowFlags() & Qt::Dialog) && widget->pos() == QPointF())
            widget->setPos(10.0, 200.0);

        inworld_scene_->addItem(widget);

        // Add to internal control list
        if (!all_proxy_widgets_in_scene_.contains(widget))
        {
            all_proxy_widgets_in_scene_.append(widget);
            connect(widget, SIGNAL(destroyed(QObject*)), SLOT(ProxyDestroyed(QObject*)));
        }

        // \todo Find a proper solution to the problem
        // Proxy widget doesn't get input without main frame resisizing for unknow reason.
        // HACK begin
        widget->moveBy(1, 1);
        widget->moveBy(-1,-1);
        // HACK end

        connect(widget, SIGNAL(BringProxyToFrontRequest(QGraphicsProxyWidget*)), SLOT(BringProxyToFront(QGraphicsProxyWidget*)));
        //connect(widget, SIGNAL(ProxyMoved(QGraphicsProxyWidget*, const QPointF &)), SLOT(ProxyWidgetMoved(QGraphicsProxyWidget*, const QPointF &)));
        //connect(widget, SIGNAL(ProxyUngrabbed(QGraphicsProxyWidget*, const QPointF &)), SLOT(ProxyWidgetUngrabbed(QGraphicsProxyWidget*, const QPointF &)));
        connect(widget, SIGNAL(Closed()), SLOT(ProxyClosed()));
        connect(widget, SIGNAL(Visible(bool)), SLOT(ProxyClosed()));

        return true;
    }

	bool InworldSceneController::AddInternalWidgetToScene(QWidget *widget, Qt::Corner corner, Qt::Orientation orientation, int priority, bool persistence) 
	{
		//Create the QGraphicsProxyWidget
		QGraphicsProxyWidget *qgrap = new QGraphicsProxyWidget(0, Qt::Widget);
		qgrap->setWidget(widget);

		//Add to non_priority list if not persistent between worlds
		if (!persistence)
			non_persistent_widgets.append(qgrap);

		//Case
		switch(corner)
		{
		case Qt::BottomLeftCorner:
			{
				if (orientation == Qt::Horizontal)
				{
					if (bottomleft_horiz_.empty())
						bottomleft_horiz_.push_back(internal_element(qgrap, priority));
					else
					{
						int i = 0;
						while (i < bottomleft_horiz_.size()) {
							if (bottomleft_horiz_.at(i).second < priority)
								break;
							i++;
						}
						bottomleft_horiz_.insert(i, internal_element(qgrap, priority));
					}
				}
				else
				{
					if (bottomleft_horiz_.empty()) //HORIZ!
						bottomleft_horiz_.push_back(internal_element(qgrap, priority));
					else
					{
						int i = 0;
						while (i < bottomleft_vert_.size()) {
							if (bottomleft_vert_.at(i).second < priority)
								break;
							i++;
						}
						bottomleft_vert_.insert(i, internal_element(qgrap, priority));
					}
				}
				break;
			}
		case Qt::BottomRightCorner:
			{
				if (orientation == Qt::Horizontal)
				{
					if (bottomright_horiz_.empty())
						bottomright_horiz_.push_back(internal_element(qgrap, priority));
					else
					{
						int i = 0;
						while (i < bottomright_horiz_.size()) {
							if (bottomright_horiz_.at(i).second < priority)
								break;
							i++;
						}
						bottomright_horiz_.insert(i, internal_element(qgrap, priority));
					}
				}
				else
				{
					if (bottomright_horiz_.empty()) //HORIZ!
						bottomright_horiz_.push_back(internal_element(qgrap, priority));
					else
					{
						int i = 0;
						while (i < bottomright_vert_.size()) {
							if (bottomright_vert_.at(i).second < priority)
								break;
							i++;
						}
						bottomright_vert_.insert(i, internal_element(qgrap, priority));
					}
				}
				break;
			}
		case Qt::TopLeftCorner:
			{
				if (orientation == Qt::Horizontal)
				{
					if (topleft_horiz_.empty())
						topleft_horiz_.push_back(internal_element(qgrap, priority));
					else
					{
						int i = 0;
						while (i < topleft_horiz_.size()) {
							if (topleft_horiz_.at(i).second < priority)
								break;
							i++;
						}
						topleft_horiz_.insert(i, internal_element(qgrap, priority));
					}
				}
				else
				{
					if (topleft_horiz_.empty()) //HORIZ!
						topleft_horiz_.push_back(internal_element(qgrap, priority));
					else
					{
						int i = 0;
						while (i < topleft_vert_.size()) {
							if (topleft_vert_.at(i).second < priority)
								break;
							i++;
						}
						topleft_vert_.insert(i, internal_element(qgrap, priority));
					}
				}
				break;
			}
		default: //case Qt::Corner::TopRightCorner:
			{
				if (orientation == Qt::Horizontal)
				{
					if (topright_horiz_.empty())
						topright_horiz_.push_back(internal_element(qgrap, priority));
					else
					{
						int i = 0;
						while (i < topright_horiz_.size()) {
							if (topright_horiz_.at(i).second < priority)
								break;
							i++;
						}
						topright_horiz_.insert(i, internal_element(qgrap, priority));
					}
				}
				else
				{
					if (topright_horiz_.empty()) //HORIZ!
						topright_horiz_.push_back(internal_element(qgrap, priority));
					else
					{
						int i = 0;
						while (i < topright_vert_.size()) {
							if (topright_vert_.at(i).second < priority)
								break;
							i++;
						}
						topright_vert_.insert(i, internal_element(qgrap, priority));
					}
				}
			}
		}
		AddInternalWidgets();
		return true;
	}
	void InworldSceneController::worldDisconnected()
	{
		//Clean lists
		int i = 0;
		while(i<bottomleft_horiz_.size())
			if (non_persistent_widgets.contains(bottomleft_horiz_.at(i).first))
				bottomleft_horiz_.removeAt(i);
			else
				i++;
		//bottomleft_vert_
		i = 0;
		while(i<bottomleft_vert_.size())
			if (non_persistent_widgets.contains(bottomleft_vert_.at(i).first))
				bottomleft_vert_.removeAt(i);
			else
				i++;
		//bottomright_horiz_
		i = 0;
		while(i<bottomright_horiz_.size())
			if (non_persistent_widgets.contains(bottomright_horiz_.at(i).first))
				bottomright_horiz_.removeAt(i);
			else
				i++;
		i = 0;
		//bottomright_vert_
		while(i<bottomright_vert_.size())
			if (non_persistent_widgets.contains(bottomright_vert_.at(i).first))
				bottomright_vert_.removeAt(i);
			else
				i++;
		//topleft_horiz_
		while(i<topleft_horiz_.size())
			if (non_persistent_widgets.contains(topleft_horiz_.at(i).first))
				topleft_horiz_.removeAt(i);
			else
				i++;
		//topleft_vert_
		while(i<topleft_vert_.size())
			if (non_persistent_widgets.contains(topleft_vert_.at(i).first))
				topleft_vert_.removeAt(i);
			else
				i++;
		//topleft_vert_
		while(i<topright_horiz_.size())
			if (non_persistent_widgets.contains(topright_horiz_.at(i).first))
				topright_horiz_.removeAt(i);
			else
				i++;
		//topleft_vert_
		while(i<topright_vert_.size())
			if (non_persistent_widgets.contains(topright_vert_.at(i).first))
				topright_vert_.removeAt(i);
			else
				i++;
		AddInternalWidgets();
	}

	void InworldSceneController::AddInternalWidgets()
	{
		//Delete items from layout/ Clear it
		layout_manager_->resetLayout();
		//Get each list and add it to the main layout

		//Qt::Corner::BottomLeftCorner
		if (!bottomleft_horiz_.empty())
		{
			layout_manager_->AddCornerAnchor(bottomleft_horiz_.at(0).first, Qt::BottomLeftCorner, Qt::BottomLeftCorner);
			if (bottomleft_horiz_.size() > 1)
				for(int i = 1; i<bottomleft_horiz_.size(); i++)
					layout_manager_->AnchorWidgetsHorizontally(bottomleft_horiz_.at(i).first, bottomleft_horiz_.at(i-1).first);

			//bottomleft_vert_
			if (!bottomleft_vert_.empty()){
				layout_manager_->AnchorWidgetsVertically(bottomleft_horiz_.at(0).first, bottomleft_vert_.at(0).first);
				for(int i = 1; i<bottomleft_vert_.size(); i++)
					layout_manager_->AnchorWidgetsVertically(bottomleft_vert_.at(i-1).first, bottomleft_vert_.at(i).first);
			}
		}

		//Qt::Corner::BottomRightCorner
		if (!bottomright_horiz_.empty())
		{
			layout_manager_->AddCornerAnchor(bottomright_horiz_.at(0).first, Qt::BottomRightCorner, Qt::BottomRightCorner);
			if (bottomright_horiz_.size() > 1)
				for(int i = 1; i<bottomright_horiz_.size(); i++)
					layout_manager_->AnchorWidgetsHorizontally(bottomright_horiz_.at(i-1).first, bottomright_horiz_.at(i).first);

			//bottomright_vert_
			if (!bottomright_vert_.empty()){
				layout_manager_->AnchorWidgetsVertically(bottomright_horiz_.at(0).first, bottomright_vert_.at(0).first);
				for(int i = 1; i<bottomright_vert_.size(); i++)
					layout_manager_->AnchorWidgetsVertically(bottomright_vert_.at(i-1).first, bottomright_vert_.at(i).first);
			}
		}

		//Qt::Corner::TopLeftCorner
		if (!topleft_horiz_.empty())
		{
			layout_manager_->AddCornerAnchor(topleft_horiz_.at(0).first, Qt::TopLeftCorner, Qt::TopLeftCorner);
			if (topleft_horiz_.size() > 1)
				for(int i = 1; i<topleft_horiz_.size(); i++)
					layout_manager_->AnchorWidgetsHorizontally(topleft_horiz_.at(i).first, topleft_horiz_.at(i-1).first);

			//topleft_vert_
			if (!topleft_vert_.empty()){
				layout_manager_->AnchorWidgetsVertically(topleft_vert_.at(0).first, topleft_horiz_.at(0).first);
				for(int i = 1; i<topleft_vert_.size(); i++)
					layout_manager_->AnchorWidgetsVertically(topleft_vert_.at(i).first, topleft_vert_.at(i-1).first);
			}
		}

		//Qt::Corner::TopRightCorner
		if (!topright_horiz_.empty())
		{
			layout_manager_->AddCornerAnchor(topright_horiz_.at(0).first, Qt::TopRightCorner, Qt::TopRightCorner);
			if (topright_horiz_.size() > 1)
				for(int i = 1; i<topright_horiz_.size(); i++)
					layout_manager_->AnchorWidgetsHorizontally(topright_horiz_.at(i-1).first, topright_horiz_.at(i).first);

			//topright_vert_
			if (!topright_vert_.empty()){
				layout_manager_->AnchorWidgetsVertically(topright_vert_.at(0).first, topright_horiz_.at(0).first);
				for(int i = 1; i<topright_vert_.size(); i++)
					layout_manager_->AnchorWidgetsVertically(topright_vert_.at(i).first, topright_vert_.at(i-1).first);
			}
		}
	}

    void InworldSceneController::AddWidgetToMenu(QWidget *widget, const QString &name, const QString &menu, const QString &icon)
    {
        menu_manager_->AddMenuItem(widget, name, menu, icon);
    }

    void InworldSceneController::RemoveProxyWidgetFromScene(QGraphicsProxyWidget *widget)
    {
        inworld_scene_->removeItem(widget);
        all_proxy_widgets_in_scene_.removeOne(widget);
    }

    void InworldSceneController::RemoveProxyWidgetFromScene(QWidget *widget)
    {
        RemoveProxyWidgetFromScene(widget->graphicsProxyWidget());
    }

    void InworldSceneController::RemoveWidgetFromMenu(QWidget *widget)
    {
        menu_manager_->RemoveMenuItem(widget);
    }

    void InworldSceneController::BringProxyToFront(QGraphicsProxyWidget *widget) const
    {
        if (inworld_scene_->isActive())
        {
            inworld_scene_->setActiveWindow(widget);
            inworld_scene_->setFocusItem(widget, Qt::ActiveWindowFocusReason);
        }
    }

    void InworldSceneController::BringProxyToFront(QWidget *widget) const
    {
        if (inworld_scene_->isActive())
        {
            ShowProxyForWidget(widget);
            inworld_scene_->setActiveWindow(widget->graphicsProxyWidget());
            inworld_scene_->setFocusItem(widget->graphicsProxyWidget(), Qt::ActiveWindowFocusReason);
        }
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

    bool InworldSceneController::AddSettingsWidget(QWidget *settings_widget, const QString &tab_name) const
    {
        control_panel_manager_->GetSettingsWidget()->AddWidget(settings_widget, tab_name);
        return true;
    }

    QObject *InworldSceneController::GetSettingsObject() const
    {
        return dynamic_cast<QObject *>(control_panel_manager_->GetSettingsWidget());
    }

    void InworldSceneController::ApplyNewProxySettings(int new_opacity, int new_animation_speed)
    {
        foreach (QGraphicsProxyWidget *widget, all_proxy_widgets_in_scene_)
        {
            UiProxyWidget *proxy_widget = dynamic_cast<UiProxyWidget *>(widget);
            if (proxy_widget)
            {
                proxy_widget->SetUnfocusedOpacity(new_opacity);
                proxy_widget->SetShowAnimationSpeed(new_animation_speed);
            }
        }
    }

    void InworldSceneController::ApplyNewProxyPosition(const QRectF &new_rect)
    {
        QPointF left_distance, right_distance;
        qreal new_x, new_y;

        foreach (QGraphicsProxyWidget *widget, all_proxy_widgets_in_scene_)
        {
            if (widget->isVisible())
            {
                // Make sure widget is not docked
                if (!all_docked_proxy_widgets_.contains(widget))
                {
                    left_distance.setX(widget->x() / last_scene_rect.width() * new_rect.width());
                    left_distance.setY(widget->y() / last_scene_rect.height() * new_rect.height());

                    right_distance.setX((widget->x() + widget->size().width()) / last_scene_rect.width() * new_rect.width());
                    right_distance.setY((widget->y() + widget->size().height()) / last_scene_rect.height() * new_rect.height());

                    if (widget->size().width() < new_rect.width())
                    {
                        if (left_distance.x() > widget->size().width())
                        {
                            if (new_rect.width() > right_distance.x())
                                new_x = right_distance.x() - widget->size().width();
                            else
                                new_x = left_distance.x();
                        }
                        else
                            new_x = left_distance.x();
                    }
                    else
                        new_x = left_distance.x();

                    if (widget->size().height() < new_rect.height())
                    {
                        if (left_distance.y() > widget->size().height())
                        {
                            if (new_rect.height() > right_distance.y())
                                new_y = right_distance.y() - widget->size().height();
                            else
                                new_y = left_distance.y();
                        }
                        else
                            new_y = left_distance.y();
                    }
                    else
                        new_y = left_distance.y();

                    widget->setPos(new_x, new_y);
                }
                else
                    widget->setPos(inworld_scene_->width() - DOCK_WIDTH, widget->y());
            }
        }

        last_scene_rect = new_rect;
        DockLineup();
    }

    void InworldSceneController::ProxyWidgetMoved(QGraphicsProxyWidget* proxy_widget, const QPointF &proxy_pos)
    {
        if (!docking_widget_)
            return;

        if (proxy_pos.x() + proxy_widget->size().width() > inworld_scene_->width() - DOCK_WIDTH)
        {
            docking_widget_->setGeometry(0, 0, DOCK_WIDTH, inworld_scene_->height() - DIST_FROM_BOTTOM);
            docking_widget_proxy_->show();
            docking_widget_proxy_->setPos(inworld_scene_->width() - DOCK_WIDTH, DIST_FROM_TOP);
        }
        else
        {
            docking_widget_proxy_->hide();
            docking_widget_proxy_->setVisible(false);
        }
    }

    void InworldSceneController::ProxyWidgetUngrabbed(QGraphicsProxyWidget* proxy_widget, const QPointF &proxy_pos)
    {
        if (!docking_widget_)
            return;

        bool changes = false;
        if (proxy_pos.x() + proxy_widget->size().width() > inworld_scene_->width() - DOCK_WIDTH)
        {
            if (!all_docked_proxy_widgets_.contains(proxy_widget))
            {
                old_proxy_size.insert(proxy_widget, proxy_widget->size());
                all_docked_proxy_widgets_.append(proxy_widget);
                changes = true;
            }
        }
        else
        {
            if (all_docked_proxy_widgets_.contains(proxy_widget))
            {
                QGraphicsProxyWidget *proxy = all_docked_proxy_widgets_.at(all_docked_proxy_widgets_.indexOf(proxy_widget));
                if (proxy)
                {
                    QSizeF old_size = old_proxy_size.value(proxy);
                    proxy->setMinimumSize(old_size);
                    proxy->setGeometry(QRectF(proxy_pos.x(), proxy_pos.y(), old_size.width(), old_size.height()));
                    all_docked_proxy_widgets_.removeAll(proxy);
                    changes = true;
                }
            }
        }

        if (changes)
            DockLineup();
    }

    void InworldSceneController::DockLineup()
    {
        if (!docking_widget_)
            return;

        if (all_docked_proxy_widgets_.count() == 0)
            return;

        int top_frame_addition = 22;
        int side_frame_width = 5;
        int order_in_dock = 0;

        qreal new_x = inworld_scene_->width() - DOCK_WIDTH + side_frame_width;
        qreal new_height = (docking_widget_->height() / all_docked_proxy_widgets_.length()) - top_frame_addition - side_frame_width;
        qreal new_width = DOCK_WIDTH - side_frame_width;

        qreal last_bottom_y = 0;
        foreach (QGraphicsProxyWidget *proxy, all_docked_proxy_widgets_)
        {
            qreal new_y;
            if (order_in_dock == 0)
                new_y = DIST_FROM_TOP + top_frame_addition;
            else
                new_y = last_bottom_y;
            
            QRectF new_rect(new_x, new_y, new_width, new_height);
            proxy->setMinimumSize(new_width, new_height);
            proxy->setGeometry(new_rect);

            last_bottom_y = new_rect.bottom() + top_frame_addition + side_frame_width;
            order_in_dock++;
        }
    }

    void InworldSceneController::ProxyClosed()
    {
        if (!docking_widget_)
            return;

        for(int i = 0; i < all_docked_proxy_widgets_.length(); i++)
        {
            if (!all_docked_proxy_widgets_.at(i)->isVisible())
            {
                QRectF dock_rect;
                dock_rect.setRect(inworld_scene_->width() - all_docked_proxy_widgets_.at(i)->size().width(),
                    all_docked_proxy_widgets_.at(i)->pos().y(), old_proxy_size.value(all_docked_proxy_widgets_.at(i)).width(),
                    old_proxy_size.value(all_docked_proxy_widgets_.at(i)).height());
                all_docked_proxy_widgets_.at(i)->setMinimumSize(old_proxy_size.value(all_docked_proxy_widgets_.at(i)).width(),
                    old_proxy_size.value(all_docked_proxy_widgets_.at(i)).height());
                all_docked_proxy_widgets_.at(i)->setGeometry(dock_rect);
                all_docked_proxy_widgets_.removeAt(i);
            }
        }
        DockLineup();
        docking_widget_proxy_->hide();
        docking_widget_proxy_->setVisible(false);
    }

    void InworldSceneController::ProxyDestroyed(QObject *obj)
    {
        if (!obj)
            return;
        QGraphicsProxyWidget *destroyed = 0;
        foreach (QGraphicsProxyWidget *widget, all_proxy_widgets_in_scene_)
        {
            if (widget == obj)
                destroyed = widget;
        }
        if (destroyed)
            all_proxy_widgets_in_scene_.removeAll(destroyed);
    }

    void InworldSceneController::DeleteCallingWidgetOnClose()
    {
        QGraphicsProxyWidget *proxy = dynamic_cast<QGraphicsProxyWidget *>(sender());
        if (proxy && !proxy->isVisible())
            proxy->deleteLater();
    }

    void InworldSceneController::HandleWidgetTransfer(const QString &name, QGraphicsProxyWidget *widget)
    {
        if (!widget)
            return;
        if (!inworld_scene_->isActive())
            return;
        if (widget->scene() == inworld_scene_)
            return;
        if (name == "Console")
        {
            inworld_scene_->addItem(widget);
            return;
        }

        inworld_scene_->addItem(widget);
        widget->setPos(50,250);
        widget->hide();
    }
}
