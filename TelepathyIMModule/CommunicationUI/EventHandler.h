// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Communication_EventHandler_UI_h
#define incl_Communication_EventHandler_UI_h

//#include "Foundation.h"
//#include "SceneEvents.h"
//#include "SceneManager.h"

#include <QObject>
#include <QStringList>
#include <QTimer>
#include <QMap>

#include "Vector3D.h"

namespace Foundation
{
    class Framework;
}

namespace CommunicationUI
{
    class EventHandler : public QObject
    {

    Q_OBJECT
    
    public:
        explicit EventHandler(Foundation::Framework *framework);
        virtual ~EventHandler();

    public slots:
        QMap<QString, QString> GetAvatarList();
        void UpdateAvatarPosition();

        void StartTrackingSelectedAvatar(QString avatar_id);
        void StopTrackingSelectedAvatar();

        Vector3df &GetCurrentPosition() { return current_position_; }
    private:
        QString avatar_id_;
        QTimer *spatial_sound_update_timer_;

        Foundation::Framework *framework_;
        bool tracking_;

        Vector3df current_position_;

    signals:
        void AvatarPositionUpdated(Vector3df);
        void TrackingAvatar(bool);
    };
}

#endif // incl_Communication_EventHandler_UI_h