/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   LibraryModule.h
 *  @brief  
 */

#ifndef incl_LibraryModule_h
#define incl_LibraryModule_h

#include "LibraryModuleApi.h"
#include "IModule.h"
#include "ModuleLoggingFunctions.h"
#include "Vector3D.h"
#include "OgreMaterialResource.h"
#include "IComponent.h"
#include "MeshFileRequest.h"
#include "EC_OpenSimPrim.h"

#include <QDropEvent>
#include <QObject>

namespace OgreRenderer
{
    class Renderer;
    typedef boost::shared_ptr<OgreRenderer::Renderer> RendererPtr;
}

namespace Foundation
{
    class EventDataInterface;
    class AssetInterface;
    class AttributeInterface;
}

namespace ProtocolUtilities
{
    class ProtocolModuleInterface;
    class WorldStream;
    class NetworkEventInboundData;
    class NetInMessage;
    typedef boost::weak_ptr<ProtocolModuleInterface> ProtocolWeakPtr;
    typedef boost::shared_ptr<WorldStream> WorldStreamPtr;
}

namespace Library
{
    class LibraryWidget;
}

class UiProxyWidget;

namespace Library
{
    class LIBRARY_MODULE_API LibraryModule :  public QObject, public IModule
    {
        Q_OBJECT

    public:
        /// Default constructor.
        LibraryModule();

        /// Destructor 
        ~LibraryModule();

        /// ModuleInterfaceImpl overrides.
        void Load();
        void PostInitialize();
        void Update(f64 frametime);
        bool HandleEvent(event_category_id_t category_id, event_id_t event_id, IEventData* data);

        //! Handles a resource event. Called from LibraryModule.
        bool HandleResourceEvent(event_id_t event_id, IEventData* data);

        //! Show the library widget
        Console::CommandResult ShowWindow(const StringVector &params);

        MODULE_LOGGING_FUNCTIONS

        /// Returns name of this module. Needed for logging.
        static const std::string &NameStatic();

        /// Name of this module.
        static const std::string moduleName;

    private:
        Q_DISABLE_COPY(LibraryModule);

        /// NetworkState event category.
        event_category_id_t networkStateEventCategory_;

        /// NetworkIn event category.
        event_category_id_t networkInEventCategory_;

        /// Framework event category
        event_category_id_t frameworkEventCategory_;

        //! Id for Resource event category
        event_category_id_t resource_event_category_;

        //! WorldStream will handle those network messages that we are wishing to send.
        ProtocolUtilities::WorldStreamPtr currentWorldStream_;

        // LibraryWidget pointer
        Library::LibraryWidget *library_widget_;        

        MeshFileRequest* mesh_file_request_;

        //Last created prim pos
        Vector3df raycast_pos_;

        //Last drag&dropped entity
        Scene::Entity *entity_;

        f64 time_from_last_update_ms_;

        void AssignMaterials();

    private slots:
        void DropEvent(QDropEvent *drop_event);
        void EntityCreated(Scene::Entity* entity, AttributeChange::Type change);


    signals:
        void UploadSceneFile(QString url, int x, int y, int z);
        void CreateObject();
        

   };
}

#endif
