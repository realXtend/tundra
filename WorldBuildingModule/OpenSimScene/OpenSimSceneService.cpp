// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "WorldBuildingModule.h"
#include "SceneManager.h"

#include "OpenSimSceneService.h"
#include "RenderServiceInterface.h"
#include "UiServiceInterface.h"
#include "WorldLogicInterface.h"

#include "EC_Placeable.h"

#include "OgreVector3.h"
#include "OgreSceneNode.h"
#include "OgreEntity.h"

#include <QFile>

namespace WorldBuilding
{
    OpenSimSceneService::OpenSimSceneService(QObject *parent, Foundation::Framework *framework) :
        framework_(framework),
        network_manager_(new QNetworkAccessManager(this)),
        scene_parser_(new SceneParser(this, framework)),
        scene_widget_(new OpenSimSceneWidget()),
        capability_name_("UploadNaaliScene")
    {
        ResetWorldStream();

        connect(network_manager_, SIGNAL(finished(QNetworkReply*)), SLOT(SceneUploadResponse(QNetworkReply*)));
        connect(scene_widget_, SIGNAL(PublishFromFile(const QString&, bool)), SLOT(PublishToServer(const QString &, bool)));
        connect(scene_widget_, SIGNAL(ExportToFile(const QString&, QList<Scene::Entity *>)), SLOT(StoreEntities(const QString &, QList<Scene::Entity *>)));
    }

    OpenSimSceneService::~OpenSimSceneService()
    {
        scene_widget_->widget()->deleteLater();
    }

    void OpenSimSceneService::CheckForCapability()
    {
        bool enabled = false;
        if (current_stream_)
            if (!current_stream_->GetCapability(capability_name_).isEmpty())
                enabled = true;
        scene_widget_->ShowFunctionality(enabled);
    }

    void OpenSimSceneService::PostInitialize()
    {
        UiServiceInterface *ui = framework_->GetService<UiServiceInterface>();
        if (ui)
        {
            ui->AddWidgetToScene(scene_widget_);
            ui->AddWidgetToMenu(scene_widget_, "OpenSim Scene Tool", "Server Tools", "./data/ui/images/menus/edbutton_LSCENE_normal.png");
            ui->RegisterUniversalWidget("OpenSim Scene", scene_widget_);
        }
        else
            WorldBuildingModule::LogWarning("OpenSimSceneService: Failed to add OpenSim Scene Tool to scene and menu");
    }

    void OpenSimSceneService::MouseLeftPressed(MouseEvent *mouse_event)
    {
        if (!scene_widget_->Exporting())
            return;

        Foundation::RenderServiceInterface *renderer = framework_->GetService<Foundation::RenderServiceInterface>();
        if (renderer)
        {
            Foundation::RaycastResult result = renderer->Raycast(mouse_event->x, mouse_event->y);
            if (!result.entity_)
                return;
            // If adding fails it means its already in the list, then we remove it
            const QString ent_id = QString::number(result.entity_->GetId());
            if (!scene_widget_->AddEntityToList(ent_id, result.entity_))
                scene_widget_->RemoveEntityFromList(ent_id, result.entity_);
        }
    }

    void OpenSimSceneService::PublishToServer(const QString &load_filename, bool adjust_pos_to_avatar)
    {
        QFile scene_content(load_filename);
        if (!scene_content.open(QIODevice::ReadOnly))
        {
            WorldBuildingModule::LogInfo("OpenSimSceneService: Could not open selected scene file.");
            return;
        }
        else
        {
            PublishToServer(scene_content.readAll(), adjust_pos_to_avatar);
            scene_content.close();
        }
    }

    void OpenSimSceneService::PublishToServer(const QByteArray &content, bool adjust_pos_to_avatar)
    {
        if (current_stream_)
        {
            QString upload_cap = current_stream_->GetCapability(capability_name_);
            if (upload_cap.isEmpty())
            {
                WorldBuildingModule::LogInfo("OpenSimSceneService: UploadNaaliScene capability missing, cant upload scene.");
                return;
            }
            
            QByteArray publish_data;
            if (adjust_pos_to_avatar)
            {
                Foundation::WorldLogicInterface *world_logic = framework_->GetService<Foundation::WorldLogicInterface>();
                if (!world_logic)
                {
                    WorldBuildingModule::LogInfo("OpenSimSceneService: Could not get world logic to resolve avatar position, cant upload scene.");
                    return;
                }

                Scene::EntityPtr user = world_logic->GetUserAvatarEntity();
                if (!user)
                {
                    WorldBuildingModule::LogInfo("OpenSimSceneService: Could not get avatar entity to resolve avatar position, cant upload scene.");
                    return;
                }

                EC_Placeable *av_placeable = user->GetComponent<EC_Placeable>().get();
                if (!av_placeable)
                {
                    WorldBuildingModule::LogInfo("OpenSimSceneService: Could not get avatar placeable component, cant upload scene.");
                    return;
                }

                // Get a pos in front of avatar
                Ogre::Vector3 av_pos = av_placeable->GetLinkSceneNode()->_getDerivedPosition();
                Vector3df av_vector(av_pos.x, av_pos.y, av_pos.z);
                av_vector += (av_placeable->GetOrientation() * Vector3df(3.0f, 0.0f, 3.0f));

                publish_data = scene_parser_->ParseAndAdjust(content, av_vector);
                if (publish_data.isEmpty())
                {
                    WorldBuildingModule::LogInfo("OpenSimSceneService: Something went wrong when adjusting pos to xml, aborting upload.");
                    return;
                }
            }
            else
                publish_data = content;

            QNetworkRequest request;
            request.setUrl(QUrl(upload_cap));
            request.setRawHeader("ImportMethod", "Upload");
            request.setRawHeader("RegionX", QString::number(current_stream_->GetInfo().regionX).toAscii());
            request.setRawHeader("RegionY", QString::number(current_stream_->GetInfo().regionY).toAscii());
            network_manager_->post(request, publish_data);
        }
        else
            WorldBuildingModule::LogInfo("OpenSimSceneService: No valid world stream available, cant upload scene.");
    }

    void OpenSimSceneService::StoreEntities(const QString &save_filename, QList<Scene::Entity *> entities)
    {
        scene_parser_->ExportToFile(save_filename, entities);
    }

    void OpenSimSceneService::SceneUploadResponse(QNetworkReply *reply)
    {
        if (reply->error() != QNetworkReply::NoError)
            WorldBuildingModule::LogInfo(QString("OpenSimSceneService: Network error while publishing scene: %1").arg(reply->errorString()).toStdString().c_str());
        reply->close();
        reply->deleteLater();
    }
}