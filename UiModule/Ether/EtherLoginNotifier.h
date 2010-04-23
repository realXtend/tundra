// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_UiModule_EtherLoginNotifier_h
#define incl_UiModule_EtherLoginNotifier_h

#include <QObject>
#include <QPair>
#include <QMap>
#include <QString>
#include <QWebFrame>

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
            EtherLoginNotifier(QObject *parent, EtherSceneController *scene_controller);

        public slots:
            void ParseInfoFromData(QPair<Data::AvatarInfo*, Data::WorldInfo*> data_cards);
            void EmitOpenSimLogin(QMap<QString, QString> info_map);
            void EmitRealXtendLogin(QMap<QString, QString> info_map);
            void EmitTaigaLogin(QWebFrame *web_frame);
            void EmitTaigaLogin(QString url);
            void ExitApplication();

        private:
            EtherSceneController *scene_controller_;

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
