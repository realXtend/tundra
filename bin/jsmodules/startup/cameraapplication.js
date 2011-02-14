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

	if (scene.GetEntityByNameRaw("ObjectCamera") != null)
		return;

	var objectcameraentity = scene.CreateEntityRaw(scene.NextFreeIdLocal(), ["EC_Script"]);
	objectcameraentity.SetName("ObjectCamera");
	objectcameraentity.SetTemporary(true);
	
	var objectcamerascript = objectcameraentity.GetComponentRaw("EC_Script");
	objectcamerascript.type = "js";
	objectcamerascript.runOnLoad = true;
	var objectcameraRef = objectcamerascript.scriptRef;
	objectcameraRef.ref = "local://objectcamera.js";
	objectcamerascript.scriptRef = objectcameraRef;
	
	scene.EmitEntityCreatedRaw(objectcameraentity);

}
