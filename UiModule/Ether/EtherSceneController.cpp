// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "EtherSceneController.h"

#include <QGraphicsProxyWidget>

namespace Ether
{
    namespace Logic
    {
        EtherSceneController::EtherSceneController(QObject *parent, View::EtherScene *scene, QPair<View::EllipseMenu*, View::EllipseMenu*> menus,
                                                   QRectF card_size, int top_items, int bottom_items)
            : QObject(parent),
              scene_(scene),
              ether_scene_visibility_(true),
              active_menu_(0),
              top_menu_(menus.first),
              bottom_menu_(menus.second),
              top_menu_visible_items_(top_items),
              bottom_menu_visible_items_(bottom_items),
              card_size_(card_size),
              last_active_top_card_(0),
              last_active_bottom_card_(0)
        {
            // Connect key press signals from scene
            connect(scene_, SIGNAL( UpPressed() ),
                    this, SLOT( UpPressed() ));
            connect(scene_, SIGNAL( DownPressed() ),
                    this, SLOT( DownPressed() ));
            connect(scene_, SIGNAL( RightPressed() ),
                    this, SLOT( RightPressed() ));
            connect(scene_, SIGNAL( LeftPressed() ),
                    this, SLOT( LeftPressed() ));

            // Connect item clicked signal
            connect(scene_, SIGNAL( ItemClicked(View::InfoCard*) ),
                    this, SLOT( ItemActivatedWithMouse(View::InfoCard*) ));

            // Connect resize signal from scene
            connect(scene_, SIGNAL( sceneRectChanged(const QRectF &) ),
                    this, SLOT( SceneRectChanged(const QRectF &) ));

            // Connect highlight signals from menus
            connect(top_menu_, SIGNAL( ItemHighlighted(View::InfoCard*) ),
                    this, SLOT( ActiveItemChanged(View::InfoCard*) ));
            connect(bottom_menu_, SIGNAL( ItemHighlighted(View::InfoCard*) ),
                    this, SLOT( ActiveItemChanged(View::InfoCard*) ));
        }

        void EtherSceneController::LoadAvatarCardsToScene(QMap<QUuid, View::InfoCard*> avatar_map)
        {
            // Add cards to scene, connect click signals
            foreach (View::InfoCard *card, avatar_map.values())
                scene_->addItem(card);

            // Adjust menu rect to proper size
            QRectF top_rect = scene_->sceneRect();
            top_rect.setHeight(top_rect.height()/2 - 5);

            // Start menu with rect and cards, set top as active
            top_menu_->Initialize(top_rect, avatar_map.values().toVector(), card_size_, 0.95, top_menu_visible_items_, 0.1);
            active_menu_ = top_menu_;
        }

        void EtherSceneController::LoadWorldCardsToScene(QMap<QUuid, View::InfoCard*> world_map)
        {
            // Add cards to scene, connect click signals
            foreach(View::InfoCard *card, world_map.values())
                scene_->addItem(card);

            // Adjust menu rect to proper size
            QRectF bottom_rect = scene_->sceneRect();
            bottom_rect.setY(scene_->height()/2 + 5);
            bottom_rect.setHeight(scene_->height()/2);

            // Start menu with rect and cards
            bottom_menu_->Initialize(bottom_rect, world_map.values().toVector(), card_size_, 0.95, bottom_menu_visible_items_, 0.1);
        }

        void EtherSceneController::LoadTitleWidgets()
        {
            avatar_info_widget_ = new View::ControlProxyWidget(View::ControlProxyWidget::CardControl, View::ControlProxyWidget::BottomToTop, "No Name");
            scene_->addItem(avatar_info_widget_);

            world_info_widget_ = new View::ControlProxyWidget(View::ControlProxyWidget::CardControl, View::ControlProxyWidget::TopToBottom, "No Name");
            scene_->addItem(world_info_widget_);

            connect_control_widget_ = new View::ControlProxyWidget(View::ControlProxyWidget::ActionControl, View::ControlProxyWidget::RightToLeft, "Connect");
            scene_->addItem(connect_control_widget_);

            exit_control_widget_ = new View::ControlProxyWidget(View::ControlProxyWidget::ActionControl, View::ControlProxyWidget::LeftToRight, "Exit");
            scene_->addItem(exit_control_widget_);
        }

        void EtherSceneController::SceneRectChanged(const QRectF &new_rect)
        {
            if (!ether_scene_visibility_)
                return;

            // New rects to item menus
            QRectF top_rect, bottom_rect;
            if (top_menu_)
            {
                top_rect = new_rect;
                top_rect.setHeight(top_rect.height()/2);
                top_menu_->RectChanged(top_rect);
            }
            if (bottom_menu_)
            {
                bottom_rect = new_rect;
                bottom_rect.setY(new_rect.height()/2);
                bottom_rect.setHeight(new_rect.height()/2);
                bottom_menu_->RectChanged(bottom_rect);
            }

            // Update avatar info pos
            View::InfoCard *hightlight = top_menu_->GetHighlighted();
            avatar_info_widget_->UpdateGeometry(hightlight->mapRectToScene(hightlight->boundingRect()));

            // Update world info pos
            hightlight = bottom_menu_->GetHighlighted();
            world_info_widget_->UpdateGeometry(hightlight->mapRectToScene(hightlight->boundingRect()));

            // Update action control positions
            connect_control_widget_->setPos(new_rect.width() - connect_control_widget_->rect().width(), new_rect.height()/2 - (connect_control_widget_->rect().height()/2));
            exit_control_widget_->setPos(0, new_rect.height()/2 - (exit_control_widget_->rect().height()/2));
        }

        void EtherSceneController::UpPressed()
        {
            active_menu_ = top_menu_;
            if (last_active_top_card_)
                ActiveItemChanged(last_active_top_card_);
        }

        void EtherSceneController::DownPressed()
        {
            active_menu_ = bottom_menu_;
            if (last_active_bottom_card_)
                ActiveItemChanged(last_active_bottom_card_);
        }

        void EtherSceneController::ItemActivatedWithMouse(View::InfoCard *clicked_item)
        {
            if (clicked_item->arragementType() == View::InfoCard::TopToBottom)
                top_menu_->SetFocusToCard(clicked_item);
            else
                bottom_menu_->SetFocusToCard(clicked_item);
        }

        void EtherSceneController::ActiveItemChanged(View::InfoCard *card)
        {
            // Clear all active item animations
            if (last_active_top_card_)
                last_active_top_card_->IsActiveItem(false);
            if (last_active_bottom_card_)
                last_active_bottom_card_->IsActiveItem(false);

            card->IsActiveItem(true);

            if (card->arragementType() == View::InfoCard::BottomToTop)
            {
                last_active_bottom_card_ = card;
                world_info_widget_->UpdateContollerCard(last_active_bottom_card_);
            }
            else if (card->arragementType() == View::InfoCard::TopToBottom)
            {
                last_active_top_card_ = card;
                avatar_info_widget_->UpdateContollerCard(last_active_top_card_);
            }
        }
    }
}
