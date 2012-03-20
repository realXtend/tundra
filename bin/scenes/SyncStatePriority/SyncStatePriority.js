
/*  This is a example script how to manipulate the 'how and when' a clients scene sync state gets filled.
    It's done on the server and starts when a client connects. Then we have to decide what entities we release immediately
    and what do we keep as pending. Once you add a entity to the pending list it will never go automatically to the
    clients sync state, you have to explicitly add it there when your logic determines it is time.

    There are multiple ways of accomplishing prioritization and it depends largely on the scene you are running on the server.
    This script aims to show a naive approach that can be used as a base for more complex logic.
      - Initially entities are accepted or rejected from the state according to 'data.rules' booleans, these are modifiable
      - After this a simple distance based approach is activated, we detect the closest entity to the users avatar and send 
        them one by with 'data.updateRate' intervals 

    Running the scene:
      1. Tundra --server --headless --file scenes/SyncStatePriority/scene.txml --file scenes/Avatar/scene.txml
      2. Open a client by running: Tundra --config viewer.xml
      3. Server address: localhost,  Username: <your_name>, Password: <no_password_needed>, Protocol: udp and hit Connect button!

    Note: Even if you don't accept entities with EC_RigidBody to the state, 
    server will still have full knowledge of the physics and the clients avatar will 
    not drop infinitely, if its on top of a rigid when spawned.

    Feel free to play around and modify the 'data' objects variables below! */

engine.ImportExtension("qt.core");

var data                    = {};
data.rules                  = {};
data.clientStates           = {};
data.updateT                = 0.0;

data.debugPrints            = true;     // Print debug information about the ongoing logic and decisions.    
data.avatarBase             = "Avatar"; // This assumes the naming schema of data.avatarBase + clientID.toString() for avatar entities.
data.updateRate             = 0.50;     // In seconds, set <0 to disable.

data.rules.acceptScript     = true;     // Default: All entities with EC_Script are accepted.
data.rules.acceptRigid      = false;    // Default: All entities with EC_RigidBody are rejected, if does not have script component.
data.rules.acceptMesh       = false;    // Default: All entities with EC_Mesh are rejected, if they does not have rigid or script components.

function OnFrameUpdate(frametime)
{
    if (data.updateRate < 0.0)
        return;
        
    // Do logic only once per 'data.updateRate'
    data.updateT += frametime;
    if (data.updateT < data.updateRate)
        return;
    data.updateT = 0.0;

    // Iterate all connections
    var users = server.AuthenticatedUsers();
    if (users.length <= 0)
        return;

    profiler.BeginBlock("SyncPriority_Update");

    for (var i=0; i<users.length; ++i)
    {
        var connId = users[i].id;
        var sceneState = syncmanager.SceneState(connId);
        var clientState = data.clientStates[connId.toString()];

        if (sceneState == null)
            continue;
        if (!sceneState.HasPendingEntities())
        {
            if (!clientState.completedLogged)
            {
                Log("** Sync completed for client id " + connId);
                clientState.completedLogged = true;
            }
            continue;
        }
            
        var msgStart = "Client " + connId + " with " + sceneState.PendingEntityIDs().length + " pending entities --> ";

        // Detect the avatar for this connection, you can add avatar app from /scenes/Avatar to the same scene to get this working
        var connAvEnt = scene.GetEntityByName(data.avatarBase + connId.toString());
                
        // Dump logic to just release next entity to the sync state until all have been released.
        if (connAvEnt == null)
        {
            if (!clientState.avatarWarningLogged)
            {
                LogWarning("** Could not find avatar for connection for distance evaluation " + connId + " with avatar entity name '" + data.avatarBase + connId + "'");
                clientState.avatarWarningLogged = true;
            }

            var nextEntId = sceneState.NextPendingEntityID();
            LogDebug(msgStart + "Adding next entity with id " + nextEntId);
            sceneState.MarkPendingEntityDirty(nextEntId);
        }
        // A bit 'smarter' avatar position based sync state fill logic. 
        // Gets closest entity and add to the sync state. Warning: Might get heavy!
        else if (connAvEnt != null && connAvEnt.placeable != null)
        {
            var closestEntData = GetClosestEntity(connAvEnt.placeable.WorldPosition(), sceneState.PendingEntityIDs());
            if (closestEntData != null && closestEntData.id != -1)
            {
                LogDebug(msgStart + "Adding closest entity to avatar with id " + closestEntData.id + " and distance " + closestEntData.dist);
                sceneState.MarkPendingEntityDirty(closestEntData.id);
            }
            else
            {
                LogError("Error in GetClosestEntity() logic, sending rest of the pending entities to client " + connId);
                sceneState.MarkPendingEntitiesDirty();
            }
        }
        else
        {
            LogError("Avatar does not have EC_Placeable, sending rest of the pending entities to client " + connId);
            sceneState.MarkPendingEntitiesDirty();
        }
    }
    
    profiler.EndBlock()
}

function GetClosestEntity(comparePos, entityIDs)
{
    var closestDist = 100000.0;
    var closestID = -1;
    for (var i=0; i<entityIDs.length; ++i)
    {
        var ent = scene.GetEntity(entityIDs[i]);
        if (ent != null && ent.placeable != null)
        {
            var myDist = ent.placeable.WorldPosition().DistanceSq(comparePos);
            if (myDist < closestDist)
            {
                closestDist = myDist;
                closestID = ent.id;
            }
        }
    }
    
    // There might be entities in the list that don't have placeable components.
    // Lastly give them one by one to be added to the sync state.
    if (closestID == -1 && entityIDs.length > 0)
    {
        closestDist = 0.0;
        closestID = entityIDs[0];
    }
    
    return {"id" : closestID, "dist" : closestDist};
}

function OnSceneStateCreated(connPtr, sceneState)
{   
    LogDebug("** Detected a new SceneSyncState for connection " + connPtr.id);
    sceneState.AboutToDirtyEntity.connect(OnAboutToDirtyEntity);

    data.clientStates[connPtr.id.toString()] = 
    { 
        avatarWarningLogged : false, 
        completedLogged : false 
    };
}

function OnAboutToDirtyEntity(changeRequest)
{
    // If you would like to do eg. clients avatar position evaluation for this entity
    // you can get the states user connection ptr like this:
    var clientID = changeRequest.connectionID;
    var clientConnectionPtr = server.GetUserConnection(clientID);
    if (clientConnectionPtr == null)
    {
        LogError("Connection ptr not valid for change request!");
        return;
    }
        
    var ent = changeRequest.entity;
    if (ent != null)
    {
        // Always accept avatars
        if (ent.name != "")
        {
            if (ent.name.substring(0, data.avatarBase.length) == data.avatarBase)
                return;
        }

        // Apply our rules and evaluate if we should let this entity to the state.
        if (data.rules.acceptScript && ent.script != null)
        {
            LogDebug("- Accepting entity " + ent.id + " with EC_Script for client " + clientID);
            return;
        }
        else if (data.rules.acceptRigid && ent.rigidbody != null)
        {
            LogDebug("- Accepting entity " + ent.id + " with EC_Rigidbody for client " + clientID);
            return;
        }
        else if (data.rules.acceptMesh && ent.mesh != null)
        {
            LogDebug("- Accepting entity " + ent.id + " with EC_Mesh for client " + clientID);
            return;
        }

        LogDebug("-- Denying sync state addition fore entity id " + ent.id + " for client " + clientID);
        changeRequest.Reject();
    }
    else
        LogWarn("Entity with id " + changeRequest.entityId + " in change request is null!");
}

// Logging
function Log(msg)       { console.LogInfo("[SyncPriority]: " + msg); }
function LogError(msg)  { console.LogError("[SyncPriority]: " + msg); }
function LogWarn(msg)   { console.LogWarning("[SyncPriority]: " + msg); }
function LogDebug(msg)  { if (data.debugPrints) Log(msg); }

// Shutdown
function OnScriptDestroyed()
{
    try
    {
        syncmanager.SceneStateCreated.disconnect(OnSceneStateCreated);
        frame.Updated.disconnect(OnFrameUpdate);
    }
    catch(e) {}
    
    data = {};
}

// Bootstrap
if (server.IsRunning())
{
    syncmanager.SceneStateCreated.connect(OnSceneStateCreated);
    frame.Updated.connect(OnFrameUpdate);
}
