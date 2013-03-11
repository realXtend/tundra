// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#include "IModule.h"
#include "TundraProtocolModuleApi.h"
#include "TundraProtocolModuleFwd.h"
#include "AssetFwd.h"
#include "Math/float3.h"

#include <kNetFwd.h>
#include <kNet/Types.h>

namespace TundraLogic
{
/// Implements the Tundra protocol server and client functionality.
class TUNDRAPROTOCOL_MODULE_API TundraLogicModule : public IModule
{
    Q_OBJECT

public:
    TundraLogicModule();
    ~TundraLogicModule();

    void Load();
    void Initialize();
    void Uninitialize();
    void Update(f64 frametime);

    /// Checks whether we are a server
    bool IsServer() const;

    /// Returns pointer to KristalliProtocolModule for convenience
    KristalliProtocolModule *GetKristalliModule() const { return kristalliModule_; }

    /// Returns syncmanager
    const shared_ptr<SyncManager>& GetSyncManager() const { return syncManager_; }

    /// Returns client
    const shared_ptr<Client>& GetClient() const { return client_; }

    /// Returns server
    const shared_ptr<Server>& GetServer() const { return server_; }

public slots:
    /// Saves scene to an XML file
    /** @param asBinary If true, saves as .tbin. Otherwise saves as .txml.
        @param saveTemporaryEntities Do we want to save temporary entities.
        @param saveLocalEntities Do we want to save local entities.
        @return Was the operation successful.*/
    bool SaveScene(QString filename, bool asBinary = false, bool saveTemporaryEntities = false, bool saveLocalEntities = true);

    /// Loads scene from an XML file.
    /** @param asBinary If true, saves as .tbin. Otherwise saves as .txml.
        @param clearScene Do we want to clear existing scene contents.
        @param useEntityIDsFromFile If true, the created entities will use the Entity IDs from the original file.
            If the scene contains any previous entities with conflicting IDs, those are removed. If false, the entity
            IDs from the files are ignored, and new IDs are generated for the created entities.
        @return Was the operation successful.*/
    bool LoadScene(QString filename, bool clearScene = true, bool useEntityIDsFromFile = true);

    /// Imports a dotscene.
    /** @param asBinary If true, saves as .tbin. Otherwise saves as .txml.
        @param clearScene Do we want to clear existing scene contents.
        @param replaceExisting Do we want replace possible already existing entities with the same name.
        @return Was the operation successful.*/
    bool ImportScene(QString filename, bool clearScene = true, bool replaceExisting = true);

    /// Imports one mesh as a new entity.
    /** @param filename Source filename for the mesh.
        @param pos Position for created entity.
        @param rot Rotation for created entity.
        @param scale Scale for created entity.
        @param inspectForMaterialsAndSkeleton Load and inspect mesh for materials and skeleton.
        @return Was the operation successful. */
    bool ImportMesh(QString filename, const float3 &pos = float3(0.f,0.f,0.f), const float3 &rot = float3(0.f,0.f,0.f),
        const float3 &scale = float3(1.f,1.f,1.f), bool inspectForMaterialsAndSkeleton = true);

private slots:
    void StartupSceneTransfedSucceeded(AssetPtr asset);
    void StartupSceneTransferFailed(IAssetTransfer *transfer, QString reason);

private:
    /// Handles a Kristalli protocol message
    void HandleKristalliMessage(kNet::MessageConnection* source, kNet::packet_id_t, kNet::message_id_t id, const char* data, size_t numBytes);

    /// Loads the startup scene(s) specified by --file command line parameter.
    void LoadStartupScene();

    shared_ptr<SyncManager> syncManager_; ///< Sync manager
    shared_ptr<Client> client_; ///< Client
    shared_ptr<Server> server_; ///< Server
    KristalliProtocolModule *kristalliModule_; ///< KristalliProtocolModule pointer
    bool autoStartServer_; ///< Whether to autostart the server
    unsigned short autoStartServerPort_; ///< Autostart server port
};

}
