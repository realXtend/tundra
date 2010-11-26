/**
*  For conditions of distribution and use, see copyright notice in license.txt
*
*  @file   LibraryModule.cpp
*  @brief  
*/

#include "StableHeaders.h"
#include "DebugOperatorNew.h"
#include "LibraryModule.h"
#include "LibraryWidget.h"

#include "WorldStream.h"
#include "EventManager.h"
#include "UiModule.h"
#include "NetworkMessages/NetInMessage.h"
#include "UiServiceInterface.h"
#include "WorldLogicInterface.h"
#include "ConsoleCommandServiceInterface.h"
#include "IOpenSimSceneService.h"
#include "OgreRenderingModule.h"
#include "NaaliUi.h"
#include "NaaliGraphicsView.h"

#include "Renderer.h"
#include "Ogre.h"
#include "SceneManager.h"
#include "UiProxyWidget.h"

#include "EC_Placeable.h"
#include "EC_Mesh.h"
#include "EC_OpenSimPrim.h"
#include "EC_NetworkPosition.h"

#include "MemoryLeakCheck.h"
#include "OgreMaterialResource.h"
#include "OgreMeshResource.h"

#include "RexTypes.h"

#include "ScriptServiceInterface.h" // call py directly
#include <QImageReader>

namespace Library
{

    LibraryModule::LibraryModule() :
        IModule(NameStatic()),
        networkStateEventCategory_(0),
        networkInEventCategory_(0),
        frameworkEventCategory_(0),
        resource_event_category_(0),
        library_widget_(0),
        raycast_pos_(0),
        mesh_file_request_(0),
        entity_(0),
        time_from_last_update_ms_(0)
    {
        supported_drop_formats_ << ".scene" << ".mesh" << ".material" << ".xml";
        
        QList<QByteArray> qt_support = QImageReader::supportedImageFormats();
        foreach (QByteArray arr, qt_support)
            supported_drop_formats_ << QString(".%1").arg(QString(arr));
    }

    LibraryModule::~LibraryModule()
    {    
        SAFE_DELETE(mesh_file_request_);
        SAFE_DELETE(entity_);
    }

    void LibraryModule::Load()
    {
    }

    void LibraryModule::PostInitialize()
    {
        frameworkEventCategory_ = framework_->GetEventManager()->QueryEventCategory("Framework");
        resource_event_category_ = framework_->GetEventManager()->QueryEventCategory("Resource");

        RegisterConsoleCommand(Console::CreateCommand("Library",
            "Shows web library.",
            Console::Bind(this, &LibraryModule::ShowWindow)));

        if (!library_widget_)
        {
            QGraphicsView *ui_view = framework_->Ui()->GraphicsView();
            library_widget_ = new LibraryWidget(ui_view);

            UiServicePtr ui = framework_->GetService<UiServiceInterface>(Service::ST_Gui).lock();                
            UiProxyWidget *lib_proxy = ui->AddWidgetToScene(library_widget_, true, true);
			//$ BEGIN_MOD $
			ui->AddWidgetToMenu(lib_proxy, tr("Library"), tr("Create"), "./data/ui/images/menus/edbutton_OBJED_normal.png");	
			//$ END_MOD $
            ui->RegisterUniversalWidget("Library", lib_proxy);

            connect(ui_view, SIGNAL(DropEvent(QDropEvent *)), SLOT(DropEvent(QDropEvent *) ));
            connect(ui_view, SIGNAL(DragEnterEvent(QDragEnterEvent *)), SLOT(HandleDragEnterEvent(QDragEnterEvent *)));
            connect(ui_view, SIGNAL(DragMoveEvent(QDragMoveEvent *)), SLOT(HandleDragMoveEvent(QDragMoveEvent *)));
        }
    }

    void LibraryModule::Update(f64 frametime)
    {
        time_from_last_update_ms_ += frametime;
        if (time_from_last_update_ms_ < 0.5)
            return;
        time_from_last_update_ms_ = 0;

        if (entity_ && mesh_file_request_)
            AssignMaterials();

        if (library_widget_ && entity_ && library_widget_->stopDownload)
        {
            SAFE_DELETE(mesh_file_request_);
            if (entity_)
                entity_ = 0;

            library_widget_->stopDownload = false;
            library_widget_->SetInfoText("Drag & drop process stopped.");
            library_widget_->HideStopButton();
        }
    }

    void LibraryModule::AssignMaterials()
    {
        //First assign pure images and ignore materials for now. Change to handle both in the same request in the future
        if (!mesh_file_request_->GetMeshImages().isEmpty())
        {
            EC_OpenSimPrim *prim = entity_->GetComponent<EC_OpenSimPrim>().get();
            MaterialMap materials = prim->Materials;

            QList<QUrl> mesh_images = mesh_file_request_->GetMeshImages();
            if (mesh_images.count() != 0)
            {
                for (int j = 0; j < mesh_images.count(); j++)
                {
                    QString id = mesh_images.at(j).toString();
                    MaterialMap::const_iterator i = materials.begin();
                    while (i != materials.end())
                    {
                        MaterialData newmaterialdata;
                        newmaterialdata.Type = RexTypes::RexAT_Texture;
                        newmaterialdata.asset_id = id.toStdString();
                        materials[j] = newmaterialdata;

                        ++i;
                    }
                }

                prim->Materials = materials;        
                prim->SendRexPrimDataUpdate();

                entity_ = 0;
                SAFE_DELETE(mesh_file_request_);

                library_widget_->SetInfoText("Drag & drop mesh is ready.");

                return;
            }
        }

        if (mesh_file_request_->GetMaterialFileRequests().isEmpty())
        {
            // No assinable materials in request queue. Consider mesh ready.
            library_widget_->SetInfoText("Drag & drop mesh is ready.");
            entity_ = 0;
            SAFE_DELETE(mesh_file_request_);

            library_widget_->HideStopButton();
            return;
        }

        if (mesh_file_request_->AreMaterialsReady() == false)
            return;

        EC_OpenSimPrim *prim = entity_->GetComponent<EC_OpenSimPrim>().get();
        QList<QString> material_ids = mesh_file_request_->GetMaterialIds();

        MaterialMap materials = prim->Materials;
        if (material_ids.count() != 0)
        {
            for (int j = 0; j < material_ids.count(); j++)
            {
                QString id = material_ids.at(j);
                MaterialMap::const_iterator i = materials.begin();
                while (i != materials.end())
                {
                    MaterialData newmaterialdata;
                    newmaterialdata.Type = RexTypes::RexAT_MaterialScript;
                    newmaterialdata.asset_id = id.toStdString();
                    materials[j] = newmaterialdata;

                    ++i;
                }
            }
        }

        prim->Materials = materials;        
        prim->SendRexPrimDataUpdate();

        entity_ = 0;
        SAFE_DELETE(mesh_file_request_);

        library_widget_->SetInfoText("Drag & drop mesh is ready.");
    }

    bool LibraryModule::HandleEvent(event_category_id_t category_id, event_id_t event_id, IEventData *data)
    {
        if (category_id == frameworkEventCategory_)
        {
            if (event_id == Foundation::NETWORKING_REGISTERED)
            {
                ProtocolUtilities::NetworkingRegisteredEvent *event_data = checked_static_cast<ProtocolUtilities::NetworkingRegisteredEvent *>(data);
                if (event_data)
                {
                    networkStateEventCategory_ = framework_->GetEventManager()->QueryEventCategory("NetworkState");
                    networkInEventCategory_ = framework_->GetEventManager()->QueryEventCategory("NetworkIn");
                    return false;
                }
            }

            if(event_id == Foundation::WORLD_STREAM_READY)
            {
                ProtocolUtilities::WorldStreamReadyEvent *event_data = checked_static_cast<ProtocolUtilities::WorldStreamReadyEvent *>(data);
                if (event_data)
                    currentWorldStream_ = event_data->WorldStream;

                networkInEventCategory_ = framework_->GetEventManager()->QueryEventCategory("NetworkIn");

                return false;
            }
        }
        
        if (category_id == resource_event_category_)
        {
            HandleResourceEvent(event_id, data);
        }

        return false;
    }

    bool LibraryModule::HandleResourceEvent(event_id_t event_id, IEventData* data)
    {
        if (event_id != Resource::Events::RESOURCE_READY)
            return false;
        
        Resource::Events::ResourceReady *res = dynamic_cast<Resource::Events::ResourceReady*>(data);
        assert(res);
        if (!res)
            return false;   

        if (!mesh_file_request_)
            return false;

        if (res->tag_ == mesh_file_request_->GetMeshRequestTag())
            currentWorldStream_->SendObjectAddPacket(mesh_file_request_->GetRayCastPos());

        if (res->resource_->GetType() == "OgreMaterial")
        {
            OgreRenderer::OgreMaterialResource* mat_res = dynamic_cast<OgreRenderer::OgreMaterialResource*>(res->resource_.get());
            if (!mat_res)
                return false;

            if (mesh_file_request_->GetMaterialFileRequests().keys().contains(res->tag_))
            {
                mesh_file_request_->AddMaterialId(QString(mat_res->GetId().c_str()));
            }   
        }

        return false;
    }

    void LibraryModule::HandleDragEnterEvent(QDragEnterEvent *e)
    {
        if (e->mimeData()->hasUrls())
        {
            QList<QUrl> urls = e->mimeData()->urls();
            QString first_url = urls.first().toString();
            if (!first_url.isEmpty())
            {
                QString extension = ".";
                extension.append(first_url.split(".").last());
                if (supported_drop_formats_.contains(extension))
                    e->accept();
            }
        }
    }

    void LibraryModule::HandleDragMoveEvent(QDragMoveEvent *e)
    {
        if (e->mimeData()->hasUrls())
        {
            QList<QUrl> urls = e->mimeData()->urls();
            QString first_url = urls.first().toString();
            if (!first_url.isEmpty())
            {
                QString extension = ".";
                extension.append(first_url.split(".").last());
                if (supported_drop_formats_.contains(extension))
                    e->accept();
            }
        }
    }

    void LibraryModule::DropEvent(QDropEvent *drop_event)
    {
        if (library_widget_->stopDownload)
        {
            SAFE_DELETE(mesh_file_request_);
            if (entity_)
                entity_ = 0;

            library_widget_->stopDownload = false;
            library_widget_->HideStopButton();
        }

        if (mesh_file_request_)
        {
            library_widget_->SetInfoText("Processing drag & drop, please wait...");
            library_widget_->ShowStopButton();
            return;
        }
            
        // Only accept url drops for now
        if (drop_event->mimeData()->hasUrls() == false) 
            return;

        // Parse all the urls from dropm mime data
        QString urlString = QUrl(drop_event->mimeData()->urls().at(0)).toString();
        QStringList urlList = urlString.split(";");

        // Do a raycast to drop position, 
        RaycastResult* cast_result = RayCast(drop_event);
        raycast_pos_ = cast_result->pos_;

        // Get drop pos in front of avatar if raycast did not hit any object
        IOpenSimSceneService *scene_service = framework_->GetService<IOpenSimSceneService>();
        if (raycast_pos_ == Vector3df::ZERO)
        {
            if (scene_service)
            {
                raycast_pos_ = scene_service->GetPosFrontOfAvatar();
                if (raycast_pos_ == Vector3df::ZERO) // still zero means something went really wrong
                {
                    LogDebug("Failed to get avatar position when raycast was a ZERO vector. Not accepting drop.");
                    return;
                }
            }
            else
            {
                LogDebug("Can't get avatar position when raycast was a ZERO vector. Not accepting drop.");
                return;
            }
        }

        // Process single url drops
        if (urlList.count() == 1)
        {
            QUrl url(urlString);
            if (urlString.endsWith(".scene"))
            {
                // Call python directly (dont want to add dependency to optional module in pythonscript module)
                // Change coords to string format for calling py
                ServiceManagerPtr manager = this->framework_->GetServiceManager();
                boost::shared_ptr<Foundation::ScriptServiceInterface> pyservice = manager->GetService<Foundation::ScriptServiceInterface>(Service::ST_PythonScripting).lock();
                if (pyservice)
                    pyservice->RunString(QString("import localscene; lc = localscene.getLocalScene(); lc.onUploadSceneFile('" + url.toString() + "', %1, %2, %3);").arg(
                        raycast_pos_.x, raycast_pos_.y, raycast_pos_.z));
            }
            else if (urlString.endsWith(".mesh"))
            {
               RequestMeshAssetAsCurrent(url);
            }
            else if (urlString.endsWith(".material"))
            {
                if (cast_result->entity_)
                {
                    EC_Mesh *mesh = cast_result->entity_->GetComponent<EC_Mesh>().get();
                    EC_OpenSimPrim *prim = cast_result->entity_->GetComponent<EC_OpenSimPrim>().get();
                    uint submesh = cast_result->submesh_;
                    if (mesh && prim)
                    {
                        MaterialMap materials = prim->Materials;
                        
                        MaterialData newmaterialdata;
                        newmaterialdata.Type = RexTypes::RexAT_MaterialScript;
                        newmaterialdata.asset_id = url.toString().toStdString();
                        materials[submesh] = newmaterialdata;

                        prim->Materials = materials;        
                        prim->SendRexPrimDataUpdate();
                    }
                }
                else
                    LogDebug("You can drag&drop .materials only to meshes.");
                
            }
            else if (urlString.endsWith(".png") || urlString.endsWith(".jpg") || urlString.endsWith(".jpeg"))
            {
                if (cast_result->entity_)
                {
                    EC_Mesh *mesh = cast_result->entity_->GetComponent<EC_Mesh>().get();
                    EC_OpenSimPrim *prim = cast_result->entity_->GetComponent<EC_OpenSimPrim>().get();
                    uint submesh = cast_result->submesh_;
                    if (mesh && prim)
                    {
                        MaterialMap materials = prim->Materials;
                        
                        MaterialData newmaterialdata;
                        newmaterialdata.Type = RexTypes::RexAT_Texture;
                        newmaterialdata.asset_id = url.toString().toStdString();
                        materials[submesh] = newmaterialdata;

                        prim->Materials = materials;        
                        prim->SendRexPrimDataUpdate();
                    }
                }
                else
                    LogDebug("You can drag&drop texture images only to meshes.");
            }
            else if (urlString.endsWith(".xml"))
            {
                if (scene_service)
                    scene_service->PublishToServer(url, raycast_pos_);
            }

            // Don´t proceed processing urlList
            return;
        }

        // For multiple url drop we accepts formats:
        //   meshUrl;materialUrl1;materialUrl2;...;materialUrlN
        //   meshUrl;textureUrl1;textureUrl2;...;textureUrlN
        foreach (QString sUrl, urlList)
        {
            QUrl url(sUrl.trimmed());
            if (url.toString().endsWith(".mesh"))
            {
                RequestMeshAssetAsCurrent(url);
            }
            else if (url.toString().endsWith(".material"))
            {
                boost::shared_ptr<OgreRenderer::Renderer> renderer = framework_->GetService<OgreRenderer::Renderer>(Service::ST_Renderer).lock();
                if (renderer)
                {
                    request_tag_t tag = renderer->RequestResource(url.toString().toStdString(), OgreRenderer::OgreMaterialResource::GetTypeStatic());
                    if (tag)
                        mesh_file_request_->SetMaterialFileRequest(tag, url);
                }
                else
                    LogDebug("Could not get rendering service, could not request .material asset from url");
            }
            else if (url.toString().endsWith(".png") || url.toString().endsWith(".jpg"))
            {
                mesh_file_request_->SetMeshImageUrl(url);
            }
        }
    }

    RaycastResult* LibraryModule::RayCast(QDropEvent *drop_event)
    {
        boost::shared_ptr<OgreRenderer::Renderer> renderer = framework_->GetService<OgreRenderer::Renderer>(Service::ST_Renderer).lock();
        if (!renderer)
        {
            LogDebug("Could not get rendering service, could not raycast");
            static RaycastResult fail_result;
            fail_result.entity_ = 0;
            fail_result.pos_ = Vector3df::ZERO;
            return &fail_result;
        }

        RaycastResult* cast_result = renderer->Raycast(drop_event->pos().x(), drop_event->pos().y());
        if (!cast_result->entity_)
            cast_result->pos_ = Vector3df::ZERO;
        return cast_result;
    }

    void LibraryModule::RequestMeshAssetAsCurrent(const QUrl& mesh_url)
    {
        // Get scene
        Scene::ScenePtr scene = framework_->GetDefaultWorldScene();

        // Unique connection, only once per each scene ptr
        connect(scene.get(), SIGNAL(EntityCreated(Scene::Entity *, AttributeChange::Type)), 
                this, SLOT(EntityCreated(Scene::Entity *, AttributeChange::Type)), Qt::UniqueConnection);

        // New mesh file request
        SAFE_DELETE(mesh_file_request_);
        mesh_file_request_ = new MeshFileRequest(raycast_pos_);
        mesh_file_request_->SetMeshFileUrl(mesh_url);

        // Get renderer service
        boost::shared_ptr<OgreRenderer::OgreRenderingModule> rendering_module = 
            framework_->GetModuleManager()->GetModule<OgreRenderer::OgreRenderingModule>().lock();
        if (!rendering_module)
        {
            LogDebug("Could not get renderer service, can't request mesh asset. Aborting drop.");
            return;
        }

        // Request the mesh file
        OgreRenderer::RendererPtr renderer = rendering_module->GetRenderer();
        request_tag_t tag = renderer->RequestResource(mesh_url.toString().toStdString(), OgreRenderer::OgreMeshResource::GetTypeStatic());
        if (tag)
            mesh_file_request_->SetMeshRequestTag(tag);
    }

    void LibraryModule::EntityCreated(Scene::Entity* entity, AttributeChange::Type change)
    {
        EC_OpenSimPrim *prim = entity->GetComponent<EC_OpenSimPrim>().get();
        EC_NetworkPosition *pos = entity->GetComponent<EC_NetworkPosition>().get();
        if (!pos)
            return;

        if (!mesh_file_request_)
            return;

        uint submesh_count = 0;
        UNREFERENCED_PARAM(submesh_count);
        QVector3D temp = pos->GetQPosition();

        Vector3df meshpos = mesh_file_request_->GetRayCastPos();
        if(meshpos.x == temp.x() && meshpos.y == temp.y())
        {
            QUrl url = mesh_file_request_->GetMeshFileUrl();
            ComponentPtr comp = entity->GetOrCreateComponent(EC_Mesh::TypeNameStatic());

            EC_Mesh *mesh = dynamic_cast<EC_Mesh*>(comp.get());
            if (mesh)
            {       
                if (prim->getMeshID().isEmpty())
                {
                    prim->setMeshID(url.toString()); 
                    if (!entity_)
                        entity_ = entity;
                }
                else
                {
                    mesh->SetMesh(prim->getMeshID().toStdString());
                    if (!entity_)
                        entity_ = entity;
                    
                }
                
                prim->SendRexPrimDataUpdate();
            }
        }

        library_widget_->SetInfoText("Waiting for materials...");

    }

Console::CommandResult LibraryModule::ShowWindow(const StringVector &params)
{

    if (!library_widget_)
    {
        QGraphicsView *ui_view = framework_->Ui()->GraphicsView();
        library_widget_ = new LibraryWidget(ui_view);

        UiServicePtr ui = framework_->GetService<UiServiceInterface>(Service::ST_Gui).lock();                
        ui->AddWidgetToScene(library_widget_);

        connect(ui_view, SIGNAL(DropEvent(QDropEvent *) ), SLOT(DropEvent(QDropEvent *) ));
    }

    library_widget_->show();

    return Console::ResultSuccess("Library widget initialized.");

}

const std::string LibraryModule::moduleName = std::string("LibraryModule");

const std::string &LibraryModule::NameStatic()
{
    return moduleName;
}

extern "C" void POCO_LIBRARY_API SetProfiler(Foundation::Profiler *profiler);
void SetProfiler(Foundation::Profiler *profiler)
{
    Foundation::ProfilerSection::SetProfiler(profiler);
}

}

using namespace Library;

POCO_BEGIN_MANIFEST(IModule)
    POCO_EXPORT_CLASS(LibraryModule)
POCO_END_MANIFEST
