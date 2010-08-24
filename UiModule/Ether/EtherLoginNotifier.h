// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_UiModule_EtherLoginNotifier_h
#define incl_UiModule_EtherLoginNotifier_h

#include <QObject>
#include <QPair>
#include <QMap>
#include <QString>

#include "UiModuleApi.h"

class QWebFrame;

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
            bool IsTeleporting() const { return teleporting_; }
            void SetIsTeleporting (bool teleporting) { teleporting_ = teleporting; }

        public slots:
            void ParseInfoFromData(QPair<Data::AvatarInfo*, Data::WorldInfo*> data_cards);

            void EmitLogin(const QMap<QString, QString> &info_map);
            void EmitLogin(QWebFrame *web_frame);
            void EmitLogin(const QString &url);

            void EmitDisconnectRequest();
            void ExitApplication();

            void EmitLoginStarted();
            void EmitLoginFailed(const QString &message);
            void EmitLoginSuccessful();

            UI_MODULE_API void Teleport(QString start_location);
            UI_MODULE_API void ScriptTeleportAnswer(QString answer, QString region_name);
            void ScriptTeleport();

        signals:
            /// Emitted when login info for OpenSim/RealXtend login is ready.
            void StartLogin(const QMap<QString, QString> &info_map);

            /// Emitted when web frame for Taiga web login is ready.
            void StartLogin(QWebFrame *web_frame);

            /// Emitted when login URL for Taiga web login is ready.
            void StartLogin(const QString &url);

            void Disconnect();
            void Quit();

            void LoginStarted();
            void LoginFailed(const QString &message);
            void LoginSuccessful();

        private:
            EtherSceneController *scene_controller_;
            Foundation::Framework *framework_;
            QMap<QString, QString> last_info_map_;
            bool teleporting_;
            QString region_name_;
        };
    }
}
#endif
