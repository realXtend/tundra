// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_WorldMap_WorldMapWidget_h
#define incl_WorldMap_WorldMapWidget_h

//#include "DebugOperatorNew.h"

#include <QWidget>
#include <QTimer>
#include <QObject>
#include "ui_WorldMapWidget.h"
#include "NetworkEvents.h"
#include "Vector3D.h"
#include "ImageItem.h"

namespace ProtocolUtilities
{        
}

namespace WorldMap
{
    class WorldMapWidget : public QWidget, private Ui::WorldMapWidget
    {
        Q_OBJECT

    public:
        WorldMapWidget();
        virtual ~WorldMapWidget();

        void StoreMapData(QPixmap &image, QString map_id);
        void UpdateAvatarPosition(Vector3df position, QString avatar_id, QString avatar_name);
        void SetMyAvatarId(QString avatar_id);
        void SetSimName(QString simName);
        void SetMapBlocks(const QList<ProtocolUtilities::MapBlock> map_blocks) { map_blocks_ = map_blocks; };
        void RemoveAvatar(QString avatar_id);
        QString GetMyAvatarId() { return my_avatar_id_; };

    protected:
        QTimer *update_timer_;

    private:                
        void StartUpdateTimer();
        void StopUpdateTimer();
        void DrawMap();
        QMap<QString, WorldMap::ImageItem *> avatar_items_;
        QMap<QString, QString> avatar_names_;
        QMap<QString, QPixmap> region_images_;
        QBrush brush_;
        QString my_avatar_id_;
        QString sim_name_;
        QList<ProtocolUtilities::MapBlock> map_blocks_;
        
        
    };
}

#endif // incl_WorldMap_WorldMapWidget_h