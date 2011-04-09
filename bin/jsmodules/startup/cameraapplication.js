// A startup script that hooks to scene added & scene cleared signals, and creates a local freelook camera upon either signal.

framework.Scene().SceneAdded.connect(OnSceneAdded);

function OnSceneAdded(scenename)
{
    // If scene is the OpenSim client scene, hardcodedly do not respond
    if (scenename == "World")
        return;

    // Get pointer to scene through framework
    scene = framework.Scene().GetScene(scenename);
    scene.SceneCleared.connect(OnSceneCleared);
    CreateCamera(scene);
}

function OnSceneCleared(scene)
{
    CreateCamera(scene);
}

function CreateCamera(scene)
{
    if (scene.GetEntityByNameRaw("FreeLookCamera") != null)
        return;

    var entity = scene.CreateEntityRaw(scene.NextFreeIdLocal(), ["EC_Script"]);
    entity.SetName("FreeLookCamera");
    entity.SetTemporary(true);

    var script = entity.GetComponentRaw("EC_Script");
    script.type = "js";
    script.runOnLoad = true;
    var r = script.scriptRef;
    r.ref = "local://freelookcamera.js";
    script.scriptRef = r;

    scene.EmitEntityCreatedRaw(entity);

}
