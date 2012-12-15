// Avatar application. Will handle switching logic between avatar & freelook camera (clientside), and
// spawning avatars for clients (serverside). Note: this is not a startup script, but is meant to be
// placed in an entity in a scene that wishes to implement avatar functionality, with the application name
// AvatarApp.

var avatar_area_size = 10;
var avatar_area_x = 0;
var avatar_area_y = 5;
var avatar_area_z = 0;
var inputContext = null;

function OnScriptDestroyed()
{
    if (inputContext)
        input.UnregisterInputContextRaw(inputContext.Name());
}

if (server.IsRunning())
{
    server.UserAboutToConnect.connect(ServerHandleUserAboutToConnect);
    server.UserConnected.connect(ServerHandleUserConnected);
    server.UserDisconnected.connect(ServerHandleUserDisconnected);
    
    // If there are connected users when this script was added, add av for all of them
    var users = server.AuthenticatedUsers();
    if (users.length > 0)
        print("[Avatar Application] Application started. Creating avatars for logged in clients.");

    for(var i=0; i < users.length; i++)
        ServerHandleUserConnected(users[i].id, users[i]);
}
else // client
{
    inputContext = input.RegisterInputContextRaw("AvatarApplication", 102);
    inputContext.KeyPressed.connect(function(e) { if (e.HasCtrlModifier() && e.keyCode == Qt.Key_Tab) me.Exec(1, "ToggleCamera") } );
    me.Action("ToggleCamera").Triggered.connect(ClientHandleToggleCamera);
}

function ClientHandleToggleCamera() {
    // For camera switching to work, must have both the FreeLookCamera & AvatarCamera in the scene
    var freeLookCameraEntity = scene.EntityByName("FreeLookCamera");
    var avatarCameraEntity = scene.EntityByName("AvatarCamera");
    var freeCameraListener = freeLookCameraEntity.GetComponent("EC_SoundListener");
    var avatarEnt = scene.EntityByName("Avatar" + client.connectionId);
    var avatarListener = avatarEnt.GetComponent("EC_SoundListener");
    if (freeLookCameraEntity == null || avatarCameraEntity == null)
        return;
    var freeLookCamera = freeLookCameraEntity.camera;
    var avatarCamera = avatarCameraEntity.camera;

    if (avatarCamera.IsActive()) {
        var trans = freeLookCameraEntity.placeable.transform;
        trans.pos = avatarCameraEntity.placeable.WorldPosition();
        trans.SetOrientation(avatarCameraEntity.placeable.WorldOrientation());

        // If there is roll in the rotation, adjust it away
        if (trans.rot.z > 170.0)
        {
            trans.rot.x -= 180.0;
            trans.rot.z = 0;
            trans.rot.y = -90.0 - (90.0 + trans.rot.y);
        }
        if (trans.rot.z < -170.0)
        {
            trans.rot.x += 180.0;
            trans.rot.z = 0;
            trans.rot.y = -90.0 - (90.0 + trans.rot.y);
        }

        freeLookCameraEntity.placeable.transform = trans;
        freeLookCamera.SetActive();
        freeCameraListener.active = true;
        avatarListener.active = false;
    } else {
        avatarCamera.SetActive();
        avatarListener.active = true;
        freeCameraListener.active = false;
    }
    
    // Ask entity to check his camera state
    avatarEnt.Exec(1, "CheckState");
}

function ServerHandleUserAboutToConnect(connectionID, user) {
    // Uncomment to test access control
    //if (user.GetProperty("password") != "xxx")
    //    user.DenyConnection();
}

function ServerHandleUserConnected(connectionID, user) {
    var avatarEntityName = "Avatar" + connectionID;

    // Create necessary components to the avatar entity:
    // - Script for the main avatar script simpleavatar.js
    // - Placeable for position
    // - AnimationController for skeletal animation control
    // - DynamicComponent for holding disabled/enabled avatar features
    var avatarEntity = scene.CreateEntity(scene.NextFreeId(), ["EC_Script", "EC_Placeable", "EC_AnimationController", "EC_DynamicComponent"]);
    avatarEntity.SetTemporary(true); // We never want to save the avatar entities to disk.
    avatarEntity.SetName(avatarEntityName);
    
    if (user != null)
        avatarEntity.SetDescription(user.GetProperty("username"));

    var script = avatarEntity.script;
    script.className = "AvatarApp.SimpleAvatar";

    // Simpleavatar.js implements the basic avatar movement and animation.
    // Also load an additional script object to the same entity (ExampleAvatarAddon.js) to demonstrate adding features to the avatar.
    var script2 = avatarEntity.GetOrCreateComponent("EC_Script", "Addon", 0, true);
    script2.className = "AvatarApp.ExampleAvatarAddon";

    // Set random starting position for avatar
    var placeable = avatarEntity.placeable;
    var transform = placeable.transform;
    transform.pos.x = (Math.random() - 0.5) * avatar_area_size + avatar_area_x;
    transform.pos.y = avatar_area_y;
    transform.pos.z = (Math.random() - 0.5) * avatar_area_size + avatar_area_z;
    placeable.transform = transform;

    if (user != null)
        print("[Avatar Application] Created avatar for " + user.GetProperty("username"));
}

function ServerHandleUserDisconnected(connectionID, user) {
    var avatarEntityName = "Avatar" + connectionID;
    var avatarEntity = scene.EntityByName(avatarEntityName);
    if (avatarEntity != null) {
        scene.RemoveEntity(avatarEntity.id);

        if (user != null) {
            print("[Avatar Application] User " + user.GetProperty("username") + " disconnected, destroyed avatar entity.");
        }
    }
}
