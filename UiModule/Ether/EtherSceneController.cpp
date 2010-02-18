// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "EtherSceneController.h"

#include <QGraphicsProxyWidget>

namespace Ether
{
    namespace Logic
    {
        EtherSceneController::EtherSceneController(QObject *parent, Data::DataManager *data_manager, View::EtherScene *scene, 
                                                   QPair<View::EtherMenu*, View::EtherMenu*> menus, QRectF card_size, int top_items, int bottom_items)
            : QObject(parent),
              data_manager_(data_manager),
              scene_(scene),
              active_menu_(0),
              top_menu_(menus.first),
              bottom_menu_(menus.second),
              top_menu_visible_items_(top_items),
              bottom_menu_visible_items_(bottom_items),
              menu_cap_size_(10),
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
            connect(scene_, SIGNAL( EnterPressed() ),
                    this, SLOT( TryStartLogin() ));

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

            // Init widget pointers to 0 for null cheching on startup
            avatar_info_widget_ = 0;
            world_info_widget_ = 0;
            connect_control_widget_ = 0;
            exit_control_widget_ = 0;
            action_proxy_widget_ = 0;
        }

        void EtherSceneController::LoadAvatarCardsToScene(QMap<QUuid, View::InfoCard*> avatar_map, int visible_top_items, bool add_to_scene)
        {
            top_menu_visible_items_ = visible_top_items;

            // Add cards to scene if requested
            if (add_to_scene)
            {
                foreach (View::InfoCard *card, avatar_map.values())
                    scene_->addItem(card);
            }

            // Adjust menu rect to proper size
            QRectF top_rect = scene_->sceneRect();
            top_rect.setHeight(top_rect.height()/2 - 5);

            // Start menu with rect and cards, set top as active
            top_menu_->Initialize(top_rect, avatar_map.values().toVector(), card_size_, 0.95, top_menu_visible_items_, 0.05, menu_cap_size_);
            
            // Recalculate the positions if this was a update, 
            // not adding widgets to scene
            if (!add_to_scene)
                RecalculateMenus();

            // For the first run we set top as the active menu
            if (!active_menu_)
                active_menu_ = top_menu_;
        }

        void EtherSceneController::NewAvatarToScene(View::InfoCard *new_card, QMap<QUuid, View::InfoCard*> avatar_map, int visible_top_items)
        {
            top_menu_visible_items_ = visible_top_items;
            scene_->addItem(new_card);

            // Adjust menu rect to proper size
            QRectF top_rect = scene_->sceneRect();
            top_rect.setHeight(top_rect.height()/2 - 5);

            avatar_map.remove(new_card->id());
            QVector<View::InfoCard*> avatar_vector = avatar_map.values().toVector();
            avatar_vector.push_front(new_card);
            
            top_menu_->Initialize(top_rect, avatar_vector, card_size_, 0.95, top_menu_visible_items_, 0.05, menu_cap_size_);
            active_menu_ = top_menu_;

            RecalculateMenus();
        }

        void EtherSceneController::LoadWorldCardsToScene(QMap<QUuid, View::InfoCard*> world_map, int visible_bottom_items, bool add_to_scene)
        {
            bottom_menu_visible_items_ = visible_bottom_items;

            // Add cards to scene if requested
            if (add_to_scene)
            {
                foreach (View::InfoCard *card, world_map.values())
                    scene_->addItem(card);
            }

            // Adjust menu rect to proper size
            QRectF bottom_rect = scene_->sceneRect();
            bottom_rect.setY(scene_->height()/2 + 5);
            bottom_rect.setHeight(scene_->height()/2);

            // Start menu with rect and cards
            bottom_menu_->Initialize(bottom_rect, world_map.values().toVector(), card_size_, 0.95, bottom_menu_visible_items_, 0.05, menu_cap_size_);
            
            // Recalculate the positions if this was a update, 
            // not adding widgets to scene
            if (!add_to_scene)
                RecalculateMenus();
        }

        void EtherSceneController::NewWorldToScene(View::InfoCard *new_card, QMap<QUuid, View::InfoCard*> world_map, int visible_bottom_items)
        {
            bottom_menu_visible_items_ = visible_bottom_items;
            scene_->addItem(new_card);

            // Adjust menu rect to proper size
            QRectF bottom_rect = scene_->sceneRect();
            bottom_rect.setY(scene_->height()/2 + 5);
            bottom_rect.setHeight(scene_->height()/2);

            world_map.remove(new_card->id());
            QVector<View::InfoCard*> world_vector = world_map.values().toVector();
            world_vector.push_front(new_card);

            // Start menu with rect and cards
            bottom_menu_->Initialize(bottom_rect, world_vector, card_size_, 0.95, bottom_menu_visible_items_, 0.05, menu_cap_size_);
            active_menu_ = bottom_menu_;

            RecalculateMenus();
        }

        void EtherSceneController::LoadActionWidgets()
        {
            // Action widget to show info and edit cards info
            action_proxy_widget_ = new View::ActionProxyWidget(data_manager_);
            connect(action_proxy_widget_, SIGNAL( ActionInProgress(bool) ), SLOT( ActionWidgetInProgress(bool) ));
            connect(action_proxy_widget_, SIGNAL( RemoveAvatar(Data::AvatarInfo *) ), SLOT ( RemoveAvatar(Data::AvatarInfo *) ));
            connect(action_proxy_widget_, SIGNAL( RemoveWorld(Data::WorldInfo *) ), SLOT ( RemoveWorld(Data::WorldInfo *) ));
            scene_->addItem(action_proxy_widget_);

            // Avatar info frame
            avatar_info_widget_ = new View::ControlProxyWidget(View::ControlProxyWidget::CardControl, View::ControlProxyWidget::BottomToTop, "No Name");
            avatar_info_widget_->SetActionWidget(action_proxy_widget_);
            scene_->addItem(avatar_info_widget_);

            // World info frame
            world_info_widget_ = new View::ControlProxyWidget(View::ControlProxyWidget::CardControl, View::ControlProxyWidget::TopToBottom, "No Name");
            world_info_widget_->SetActionWidget(action_proxy_widget_);
            scene_->addItem(world_info_widget_);

            //// Connect button
            //connect_control_widget_ = new View::ControlProxyWidget(View::ControlProxyWidget::ActionControl, View::ControlProxyWidget::RightToLeft, "Connect");
            //connect(connect_control_widget_, SIGNAL( ActionRequest() ), SLOT( TryStartLogin() ));
            //scene_->addItem(connect_control_widget_);

            //// Exit button
            //exit_control_widget_ = new View::ControlProxyWidget(View::ControlProxyWidget::ActionControl, View::ControlProxyWidget::LeftToRight, "Exit");
            //scene_->addItem(exit_control_widget_);
        }

        void EtherSceneController::RecalculateMenus()
        {
            SceneRectChanged(scene_->sceneRect());
        }

        void EtherSceneController::SceneRectChanged(const QRectF &new_rect)
        {
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

            //// Update action control positions
            //connect_control_widget_->setPos(new_rect.width() - connect_control_widget_->rect().width(), new_rect.height()/2 - (connect_control_widget_->rect().height()/2));
            //exit_control_widget_->setPos(0, new_rect.height()/2 - (exit_control_widget_->rect().height()/2));

            // Update action widget
            action_proxy_widget_->UpdateGeometry(new_rect);
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

        void EtherSceneController::ActionWidgetInProgress(bool action_ongoing)
        {
            scene_->SupressKeyEvents(action_ongoing);
        }

        void EtherSceneController::UpdateAvatarInfoWidget()
        {
            if (last_active_top_card_)
                avatar_info_widget_->UpdateContollerCard(last_active_top_card_);
        }

        void EtherSceneController::RemoveAvatar(Data::AvatarInfo *avatar_info)
        {
            bool removed = data_manager_->RemoveAvatar(avatar_info);
            if (removed && last_active_top_card_)
            {
                last_active_top_card_->close();
                scene_->removeItem(last_active_top_card_);
                emit ObjectRemoved(avatar_info->id());
            }
        }

        void EtherSceneController::UpdateWorldInfoWidget()
        {
            if (last_active_bottom_card_)
                world_info_widget_->UpdateContollerCard(last_active_bottom_card_);
        }

        void EtherSceneController::RemoveWorld(Data::WorldInfo *world_info)
        {
            bool removed = data_manager_->RemoveWorld(world_info);
            if (removed && last_active_bottom_card_)
            {
                last_active_bottom_card_->close();
                scene_->removeItem(last_active_bottom_card_);
                emit ObjectRemoved(world_info->id());
            }
        }

        void EtherSceneController::TryStartLogin()
        {
            QPair<View::InfoCard*, View::InfoCard*> selected_cards;
            selected_cards.first = top_menu_->GetHighlighted();
            selected_cards.second = bottom_menu_->GetHighlighted();

            emit LoginRequest(selected_cards);
        }
    }
}
