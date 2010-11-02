// A startup script that hooks to scene added & scene cleared signals, and creates a local freelook camera upon either signal.

framework.SceneAdded.connect(OnSceneAdded);

function OnSceneAdded(scenename)
{
    // If scene is the OpenSim client scene, hardcodedly do not respond
    if (scenename == "World")
        return;

    // Get pointer to scene through framework
    scene = framework.Scene(scenename);
    scene.SceneCleared.connect(OnSceneCleared);
    CreateCamera(scene);
}

function OnSceneCleared(scene)
{
    CreateCamera(scene);
}

function CreateCamera(scene)
{
    var entity = scene.CreateEntityRaw(scene.NextFreeIdLocal(), ["EC_Script"]);
    entity.SetName("FreeLookCamera");
    entity.SetTemporary(true);

    var script = entity.GetComponentRaw("EC_Script");
    script.type = "js";
    script.scriptRef = "./jsmodules/camera/freelookcamera.js";
    script.Run();
}
