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
    const boost::shared_ptr<SyncManager>& GetSyncManager() const { return syncManager_; }

    /// Returns client
    const boost::shared_ptr<Client>& GetClient() const { return client_; }

    /// Returns server
    const boost::shared_ptr<Server>& GetServer() const { return server_; }

public slots:
    /// Saves scene to an XML file
    /// @param asBinary If true, saves as .tbin. Otherwise saves as .txml.
    void SaveScene(QString filename, bool asBinary = false, bool saveTemporaryEntities = false, bool saveLocalEntities = true);

    /// Loads scene from an XML file.
    void LoadScene(QString filename, bool clearScene = true, bool useEntityIDsFromFile = true);

    /// Imports a dotscene.
    void ImportScene(QString filename, bool clearScene = true, bool replace = true);

    /// Imports one mesh as a new entity.
    void ImportMesh(QString filename, const float3 &pos = float3(0.f,0.f,0.f), const float3 &rot = float3(0.f,0.f,0.f),
        const float3 &scale = float3(1.f,1.f,1.f), bool inspectForMaterialsAndSkeleton = true);

private slots:
    void StartupSceneTransfedSucceeded(AssetPtr asset);
    void StartupSceneTransferFailed(IAssetTransfer *transfer, QString reason);

private:
    /// Handles a Kristalli protocol message
    void HandleKristalliMessage(kNet::MessageConnection* source, kNet::packet_id_t, kNet::message_id_t id, const char* data, size_t numBytes);

    /// Loads the startup scene(s) specified by --file command line parameter.
    void LoadStartupScene();
    void LoadStartupScene(const QString &file);

    boost::shared_ptr<SyncManager> syncManager_; ///< Sync manager
    boost::shared_ptr<Client> client_; ///< Client
    boost::shared_ptr<Server> server_; ///< Server
    KristalliProtocolModule *kristalliModule_; ///< KristalliProtocolModule pointer
    bool autoStartServer_; ///< Whether to autostart the server
    unsigned short autoStartServerPort_; ///< Autostart server port
};

}
