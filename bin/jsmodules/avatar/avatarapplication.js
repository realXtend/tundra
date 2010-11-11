// Avatar application. Will handle switching logic between avatar & freelook camera (clientside), and
// spawning avatars for clients (serverside). Note: this is not a startup script, but is meant to be
// placed in an entity in a scene that wishes to implement avatar functionality.

var avatar_area_size = 10;
var avatar_area_x = 0;
var avatar_area_y = 0;
var avatar_area_z = 20;

var isserver = server.IsRunning();

if (isserver == false)
{
    var inputmapper = me.GetOrCreateComponentRaw("EC_InputMapper");
    inputmapper.SetTemporary(true);
    inputmapper.contextPriority = 102;
    inputmapper.RegisterMapping("Ctrl+Tab", "ToggleCamera", 1);

    me.Action("ToggleCamera").Triggered.connect(ClientHandleToggleCamera);
}
else
{
    server.UserConnected.connect(ServerHandleUserConnected);
    server.UserDisconnected.connect(ServerHandleUserDisconnected);
}

function ClientHandleToggleCamera()
{
    // For camera switching to work, must have both the freelookcamera & avatarcamera in the scene
    var freelookcameraentity = scene.GetEntityByNameRaw("FreeLookCamera");
    var avatarcameraentity = scene.GetEntityByNameRaw("AvatarCamera");
    if ((freelookcameraentity == null) || (avatarcameraentity == null))
        return;
    var freelookcamera = freelookcameraentity.GetComponentRaw("EC_OgreCamera");
    var avatarcamera = avatarcameraentity.GetComponentRaw("EC_OgreCamera");

    if (avatarcamera.IsActive())
    {
        freelookcameraentity.GetComponentRaw("EC_Placeable").transform = avatarcameraentity.GetComponentRaw("EC_Placeable").transform
        freelookcamera.SetActive();
    }
    else
    {
        avatarcamera.SetActive();
    }
}

function ServerHandleUserConnected(connectionID, username)
{
    var avatarEntityName = "Avatar" + connectionID;
    var avatarEntity = scene.CreateEntityRaw(scene.NextFreeId(), ["EC_Script", "EC_Placeable"]);
    avatarEntity.SetName(avatarEntityName);
    avatarEntity.SetDescription(username);

    var script = avatarEntity.GetComponentRaw("EC_Script");
    script.type = "js";
    script.runOnLoad = true;
    var r = script.scriptRef;
    r.ref = "./jsmodules/camera/freelookcamera.js";
    script.scriptRef = r;

    var placeable = avatarEntity.GetComponentRaw("EC_Placeable");
    // Set random starting position for avatar
    var transform = placeable.transform;
    transform.pos.x = (Math.random() - 0.5) * avatar_area_size + avatar_area_x;
    transform.pos.y = (Math.random() - 0.5) * avatar_area_size + avatar_area_y;
    transform.pos.z = avatar_area_z;
    placeable.transform = transform;

    scene.EmitEntityCreated(avatarEntity);
}

function ServerHandleUserDisconnected(connectionID)
{
    var avatarEntityName = "Avatar" + connectionID;
    var entityID = scene.GetEntityByNameRaw(avatarEntityName).Id;
    scene.RemoveEntityRaw(entityID);
}

