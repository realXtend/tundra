
engine.ImportExtension("qt.core");

/* This is a example script how to manipulate the 'how and when' a clients scene sync state gets filled.
   It's done on the server and start when a client connects we have to decide what entities we release immediately
   and what do we keep as pending. Once you add a entity to the pending list it will never go automatically to the
   clients sync state, you have to explicitly add it there when your logic determines it is time.
   There are multiple ways of accomplishing prioritization and it depends largely on the scene you are running on the server.
   This scripts aim is to show a few generic ways how to do it.
   
   Note: Even if you don't accept entities with EC_RigidBody to the state, 
   server will still have full knowledge of the physics and the clients avatar will 
   not drop infinitely, if its on top of a rigid when spawned.
   
   Feel free to play around and modify the 'data' objects variables below! 
*/

var data                    = {};
data.rules                  = {};
data.updateT                = 0.0;

data.debugPrints            = true;     // Print debug information about the ongoing logic and decisions.    
data.dumpLogic              = true;     // Toggle dump logic on/off. Only turn dump logic off if your scene has avatar application running!
data.avatarBase             = "Avatar"; // This assumes the naming schema of data.avatarBase + clientID.toString() for avatar entities.
data.updateRate             = 0.5;      // In seconds, set <0 to disable.

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

    profiler.BeginBlock("SyncPriority_Update");

    // Iterate all connections
    var connectionIds = server.GetConnectionIDs();
    if (connectionIds.length <= 0)
        return;

    for (var i=0; i<connectionIds.length; ++i)
    {
        var connId = connectionIds[i];
        var sceneState = syncmanager.SceneState(connId);

        if (sceneState == null)
            continue;
        if (!sceneState.HasPendingEntities())
            continue;
            
        var msgStart = "Client " + connId + " with " + sceneState.PendingEntityIDs().length + " pending entities --> ";
                
        // Dump logic to just release one entity to 
        // the sync state until all have been released.
        if (data.dumpLogic == true)
        {
            var nextID = sceneState.NextPendingEntityID();
            LogDebug(msgStart + "Adding next entity with id " + nextID);
            sceneState.MarkPendingEntityDirty(nextID);
        }
        // A bit 'smarter' avatar position based sync state fill logic. 
        // Gets closest entity and add to the sync state. Warning: Might get heavy!
        else
        {
            var connAvEnt = scene.GetEntityByName(data.avatarBase + connId.toString());
            if (connAvEnt != null && connAvEnt.placeable != null)
            {
                var closestEntData = GetClosestEntity(connAvEnt.placeable.WorldPosition(), sceneState.PendingEntityIDs());
                if (closestEntData != null && closestEntData.id != -1)
                {
                    LogDebug(msgStart + "Adding closest entity with id " + closestEntData.id + " and distance " + closestEntData.dist)
                    sceneState.MarkPendingEntityDirty(closestEntData.id);
                }
                else 
                    data.dumpLogic = true; // Oh my, panic!
            }
            else
            {
                LogError("Could not find avatar for connection " + connId + " with avatar entity name '" + data.avatarBase + connId + "' Falling back to dump logic immediately!");
                data.dumpLogic = true;
            }
        }
    }
    
    profiler.EndBlock();
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
    LogDebug("Detected a new SceneSyncState for connection " + connPtr.id);
    sceneState.AboutToDirtyEntity.connect(OnAboutToDirtyEntity);
    sceneState.AboutToDirtyComponent.connect(AboutToDirtyComponent);
}

function OnAboutToDirtyEntity(changeRequest)
{
    // If you would like to do eg. clients avatar position evaluation for this entity
    // you can get the states user connection ptr like this:
    var clientID = changeRequest.connectionID;
    var clientConnectionPtr = server.GetUserConnection(clientID);
    if (clientConnectionPtr == null)
        LogWarn("Oh no connection ptr not valid for change request!");
        
    var ent = changeRequest.entity;
    if (ent != null)
    {
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
        changeRequest.accepted = false;
    }
    else
        LogWarn("Entity with id " + changeRequest.entityId + " in change request is null!");
}

function AboutToDirtyComponent(changeRequest)
{
    OnAboutToDirtyEntity(changeRequest); // Apply same logic as entity evaluation
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
