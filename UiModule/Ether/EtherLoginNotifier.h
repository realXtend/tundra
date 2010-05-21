// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_UiModule_EtherLoginNotifier_h
#define incl_UiModule_EtherLoginNotifier_h

#include <QObject>
#include <QPair>
#include <QMap>
#include <QString>
#include <QWebFrame>

#include "UiModuleApi.h"

namespace Foundation
{
    class Framework;
}

namespace Ether
{
    namespace Data
    {
        class AvatarInfo;
        class WorldInfo;
    }

    namespace Logic
    {
        class EtherSceneController;

        class EtherLoginNotifier : public QObject
        {
            Q_OBJECT

        public:
            EtherLoginNotifier(QObject *parent, EtherSceneController *scene_controller, Foundation::Framework *framework);            
            bool IsTeleporting() { return teleporting_; }
            void SetIsTeleporting (bool teleporting) { teleporting_ = teleporting; }
            

        public slots:
            void ParseInfoFromData(QPair<Data::AvatarInfo*, Data::WorldInfo*> data_cards);
            void EmitOpenSimLogin(QMap<QString, QString> info_map);
            void EmitRealXtendLogin(QMap<QString, QString> info_map);
            void EmitTaigaLogin(QWebFrame *web_frame);
            void EmitTaigaLogin(QString url);
            void ExitApplication();
            UI_MODULE_API void Teleport(QString start_location);
            UI_MODULE_API void ScriptTeleportAnswer(QString answer, QString region_name);            
            void ScriptTeleport();
            void SetLoginData(QString address, int port);

        private:
            EtherSceneController *scene_controller_;
            Foundation::Framework *framework_;
            QMap<QString, QString> last_info_map_;                        
            bool teleporting_;
            QString region_name_;
            QString web_auth_login_address_;

        signals:
            void StartOsLogin(QMap<QString, QString> info_map);
            void StartRexLogin(QMap<QString, QString> info_map);
            void StartTaigaLogin(QWebFrame *web_frame);
            void StartTaigaLogin(QString url);
            void Quit();
        };
    }
}
#endif
