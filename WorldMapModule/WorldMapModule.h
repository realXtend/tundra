/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   WorldMapModule.h
 *  @brief  Simple OpenSim world map module. Listens to region handshake and map block replies. 
 *          Draws current region map and avatar positions to widget.
 */

#ifndef incl_WorldMapModule_h
#define incl_WorldMapModule_h

#include "WorldMapModuleApi.h"
#include "IModule.h"
#include "ModuleLoggingFunctions.h"

#include <QObject>

class RexUUID;

namespace Foundation
{
    class TextureInterface;
}

namespace Resource
{
    namespace Events
    {
        class ResourceReady;
    }
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

namespace UiServices
{
    class UiModule;
}

namespace WorldMap
{
    class WorldMapWidget;
}

QT_BEGIN_NAMESPACE
class QColor;
class QFile;
class QImage;
QT_END_NAMESPACE

namespace WorldMap
{
    class WORLDMAP_MODULE_API WorldMapModule :  public QObject, public IModule
    {
        Q_OBJECT

    public:
        /// Default constructor.
        WorldMapModule();

        /// Destructor 
        ~WorldMapModule();

        /// ModuleInterfaceImpl overrides.
        void Load();
        void PostInitialize();
        void Update(f64 frametime);
        bool HandleEvent(event_category_id_t category_id, event_id_t event_id, IEventData* data);

        //! Handles MapBlock network message.
        //! \param data Network event data.
        bool HandleOSNE_MapBlock(ProtocolUtilities::NetworkEventInboundData *data);

        //! Handles RegionHandshake network message.
        //! \param data Network event data.
        bool HandleOSNE_RegionHandshake(ProtocolUtilities::NetworkEventInboundData *data);

        //! Handles an asset event. Called from WorldMapModule.
        bool HandleResourceEvent(event_id_t event_id, IEventData* data);

        //! Show the world map widget
        Console::CommandResult ShowWindow(const StringVector &params);

        MODULE_LOGGING_FUNCTIONS

        /// Returns name of this module. Needed for logging.
        static const std::string &NameStatic();

        /// Name of this module.
        static const std::string moduleName;

    private:
        Q_DISABLE_COPY(WorldMapModule);

        virtual void UpdateAvatarPositions();
        f64 time_from_last_update_ms_;
        QImage ConvertToQImage(Foundation::TextureInterface &tex);

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

        /// WorldMapWidget pointer
        WorldMap::WorldMapWidget *worldmap_widget_;        

        //! Asset_tags for map image requests.
        std::vector<request_tag_t> map_asset_requests_;

   };
}

#endif
