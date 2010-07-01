// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#include "WorldMapWidget.h"
#include "ImageItem.h"
#include <QGraphicsScene>

#include "MemoryLeakCheck.h"

namespace WorldMap
{
    WorldMapWidget::WorldMapWidget() : 
        update_timer_(new QTimer(this)),        
        brush_(Qt::white),
        sim_name_(QString(""))
    {
        setupUi(this);
    }
    
    WorldMapWidget::~WorldMapWidget()
    {
        if (avatar_items_.size() > 0)
        {
            for (QMap<QString, WorldMap::ImageItem *>::iterator iter = avatar_items_.begin(); iter!=avatar_items_.end(); ++iter)
            {
                SAFE_DELETE(iter.value());
            }
        }
    }

    void WorldMapWidget::ClearAllContent()
    {
        if (worldMapGraphicsView->scene())
            worldMapGraphicsView->scene()->clear();

        avatar_items_.clear();
        avatar_names_.clear();
        region_images_.clear();
        map_blocks_.clear();
    }

    void WorldMapWidget::SetMyAvatarId(QString avatar_id)
    {
        my_avatar_id_ = avatar_id;
    }

    void WorldMapWidget::SetSimName(QString simName)
    {
        sim_name_ = simName;
        DrawMap();
    }

    void WorldMapWidget::RemoveAvatar(QString avatar_id)
    {
        if (!avatar_id.isEmpty())
        {
            ImageItem *item = 0;
            if (avatar_items_.contains(avatar_id))
            {
                item = avatar_items_.value(avatar_id);
                if (item)
                    SAFE_DELETE(item);

                avatar_items_.remove(avatar_id);
            }

            if (avatar_names_.contains(avatar_id))
            {
                avatar_names_.remove(avatar_id);
            }
        }
    }
        
    void WorldMapWidget::StoreMapData(QPixmap &image, QString map_id)
    {        
        if (!region_images_.contains(map_id))
        {
            region_images_.insert(map_id, image);
            DrawMap();
        }
    }
    
    void WorldMapWidget::DrawMap()
    {
        //Try to draw current region map.
        if (sim_name_.isEmpty())
            return;

        if (region_images_.size() == 0)
            return;

        QGraphicsScene *scene = worldMapGraphicsView->scene();
        if (!scene)
            scene = new QGraphicsScene(this);

        if (map_blocks_.size() == 0)
            return;

        ProtocolUtilities::MapBlock *currentBlock;

        foreach (ProtocolUtilities::MapBlock block, map_blocks_)
        {            
            if (sim_name_ == QString(block.regionName.c_str()))
            {
                currentBlock = &block;
                break;
            }
        }

        if (currentBlock)
        {
            if (region_images_.size() > 0)
            {
                for (QMap<QString, QPixmap>::iterator iter = region_images_.begin(); iter!=region_images_.end(); ++iter)
                {
                    if (iter.key() == currentBlock->mapImageID.ToQString())
                    {
                        worldMapGraphicsView->setScene(scene);
                        worldMapGraphicsView->scene()->setSceneRect(iter.value().rect());
                        scene->setBackgroundBrush(QBrush(iter.value()));                       

                        QGraphicsTextItem *item = new QGraphicsTextItem(0, scene);                        
                        item->setHtml(sim_name_);                        
                        
                        if (!item->scene())
                            scene->addItem(item);

                        item->show();

                        worldMapGraphicsView->update();
                        scene->update();

                        break;
                    }
                }
            }
        }

    }

    void WorldMapWidget::UpdateAvatarPosition(Vector3df position, QString avatar_id, QString avatar_name)
    {
        QGraphicsScene *scene = worldMapGraphicsView->scene();
        if (!scene)
            scene = new QGraphicsScene(this);
                
        worldMapGraphicsView->setScene(scene);

        ImageItem *item = 0;
        if (avatar_items_.contains(avatar_id))
        {
            item = avatar_items_.value(avatar_id);
        }
        else
        {
            item = new ImageItem(QPixmap("./data/ui/images/worldmap/default_avatar.png"), 0, scene);
            if (!my_avatar_id_.isEmpty() && my_avatar_id_ == avatar_id)
            {
                item->SetMyAvatar(true);                
            }

            item->SetAvatarName(avatar_name);
            
            if (!item->scene()) 
                scene->addItem(item);
            
            avatar_items_.insert(avatar_id, item); //Add to items            
        }

        if (!avatar_names_.contains(avatar_id))
            avatar_names_.insert(avatar_id, avatar_name); //Add to names
        
        if (!item->scene()) 
            scene->addItem(item);            

        item->setPos(position.x, 256-position.y);
        item->SetTextPosition(position.x, 256-position.y);
        item->UpdateTextPosition();
        item->setOffset(-10, -10);
        item->show();
        scene->update();
    }
}

