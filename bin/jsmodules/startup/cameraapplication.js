// A startup script that hooks to scene added & scene cleared signals, and creates a local freelook camera upon either signal.

framework.Scene().SceneAdded.connect(OnSceneAdded);

var scene = null;
var cameraEntityId = 0;

function OnSceneAdded(scenename)
{
    // Get pointer to scene through framework
    scene = framework.Scene().GetScene(scenename);
    scene.SceneCleared.connect(OnSceneCleared);
    scene.EntityCreated.connect(OnEntityCreated)
    CreateCamera(scene);
}

function OnSceneCleared(scene)
{
    CreateCamera(scene);
}

function OnEntityCreated(entity, change)
{
    if (entity.id == cameraEntityId)
        return; // This was the signal for our camera, ignore
    
    var oldCamera = scene.GetEntity(cameraEntityId);

    // If a freelookcamera entity is loaded from the scene, use it instead; delete the one we created
    if (entity.name == "FreeLookCamera")
    {
        if (entity.camera != null)
        {
            print("Activating loaded camera");
            entity.camera.SetActive();
        }
        scene.RemoveEntity(cameraEntityId);
    }
    // If a camera spawnpos entity is loaded, copy the transform
    if (entity.name == "FreeLookCameraSpawnPos")
    {
        if (oldCamera)
            oldCamera.placeable.transform = entity.placeable.transform;
    }
}

function CreateCamera(scene)
{
    if (scene.GetEntityByName("FreeLookCamera") != null)
        return;

    var entity = scene.CreateLocalEntity(["EC_Script", "EC_Camera", "EC_Placeable"]);
    entity.SetName("FreeLookCamera");
    entity.SetTemporary(true);

    var script = entity.GetComponent("EC_Script");
    script.type = "js";
    script.runOnLoad = true;
    var r = script.scriptRef;
    r.ref = "local://freelookcamera.js";
    script.scriptRef = r;
    
    cameraEntityId = entity.id;
}
