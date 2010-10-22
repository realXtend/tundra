// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_WorldBuildingModule_OpenSimSceneService_h
#define incl_WorldBuildingModule_OpenSimSceneService_h

#include "Foundation.h"
#include "WorldBuildingModuleApi.h"
#include "IOpenSimSceneService.h"
#include "WorldStream.h"
#include "MouseEvent.h"

#include "SceneParser.h"
#include "OpenSimSceneWidget.h"

#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QUrl>

namespace WorldBuilding
{
    class WB_MODULE_API OpenSimSceneService : public IOpenSimSceneService
    {

    Q_OBJECT

    public:
        OpenSimSceneService(QObject *parent, Foundation::Framework *framework);
        virtual ~OpenSimSceneService();

        void PostInitialize();

    public slots:
        /// Service interface implementation
        virtual void PublishToServer(QUrl xml_url, Vector3df drop_position = Vector3df::ZERO);
        /// Service interface implementation
        virtual void PublishToServer(const QString &load_filename, bool adjust_pos_to_avatar, Vector3df drop_position = Vector3df::ZERO);
        /// Service interface implementation
        virtual void PublishToServer(const QByteArray &content, bool adjust_pos_to_avatar, Vector3df drop_position = Vector3df::ZERO);
        /// Service interface implementation
        virtual void StoreEntities(const QString &save_filename, QList<Scene::Entity *> entities);

        void SetWorldStream(ProtocolUtilities::WorldStreamPtr stream) { current_stream_ = stream; }
        void ResetWorldStream() { current_stream_.reset(); }

        void CheckForCapability();

    private slots:
        void MouseLeftPressed(MouseEvent *mouse);
        void SceneUploadResponse(QNetworkReply *reply);

        Vector3df GetAvatarDropPosition();

    private:
        QNetworkAccessManager *network_manager_;
        ProtocolUtilities::WorldStreamPtr current_stream_;
        Foundation::Framework *framework_;

        SceneParser *scene_parser_;
        OpenSimSceneWidget *scene_widget_;

        QString capability_name_;

        QMap<QUrl, Vector3df> url_to_position_;
    };
}
#endif