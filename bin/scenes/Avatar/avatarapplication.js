// Avatar application. Will handle switching logic between avatar & freelook camera (clientside), and
// spawning avatars for clients (serverside). Note: this is not a startup script, but is meant to be
// placed in an entity in a scene that wishes to implement avatar functionality, with the application name
// AvatarApp.

var avatar_area_size = 10;
var avatar_area_x = 0;
var avatar_area_y = 5;
var avatar_area_z = 0;

if (!server.IsRunning()) {
    var inputmapper = me.GetOrCreateComponent("EC_InputMapper");
    inputmapper.SetTemporary(true);
    inputmapper.contextPriority = 102;
    inputmapper.RegisterMapping("Ctrl+Tab", "ToggleCamera", 1);

    me.Action("ToggleCamera").Triggered.connect(ClientHandleToggleCamera);
} else {
    server.UserAboutToConnect.connect(ServerHandleUserAboutToConnect);
    server.UserConnected.connect(ServerHandleUserConnected);
    server.UserDisconnected.connect(ServerHandleUserDisconnected);
    
    // If there are connected users when this script was added, add av for all of them
    var userIdList = server.GetConnectionIDs();
    if (userIdList.length > 0)
        print("[Avatar Application] Application started. Creating avatars for logged in clients.");

    for (var i=0; i < userIdList.length; i++)
    {
        var userId = userIdList[i];
        var userConnection = server.GetUserConnection(userId);
        if (userConnection != null)
            ServerHandleUserConnected(userId, userConnection);
    }
}

function ClientHandleToggleCamera() {
    // For camera switching to work, must have both the freelookcamera & avatarcamera in the scene
    var freelookcameraentity = scene.GetEntityByName("FreeLookCamera");
    var avatarcameraentity = scene.GetEntityByName("AvatarCamera");
    var freecameralistener = freelookcameraentity.GetComponent("EC_SoundListener");
    var avatarent = scene.GetEntityByName("Avatar" + client.GetConnectionID());
    var avatarlistener = avatarent.GetComponent("EC_SoundListener");
    if ((freelookcameraentity == null) || (avatarcameraentity == null))
        return;
    var freelookcamera = freelookcameraentity.camera;
    var avatarcamera = avatarcameraentity.camera;

    if (avatarcamera.IsActive()) {
        var trans = freelookcameraentity.placeable.transform;
        trans.pos = avatarcameraentity.placeable.WorldPosition();
        trans.rot = avatarcameraentity.placeable.WorldOrientation().ToEulerZYX().Mul(180.0 / Math.PI);

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

        freelookcameraentity.placeable.transform = trans;
        freelookcamera.SetActive();
        freecameralistener.active = true;
        avatarlistener.active = false;
    } else {
        avatarcamera.SetActive();
        avatarlistener.active = true;
        freecameralistener.active = false;
    }
    
    // Ask entity to check his camera state
    avatarent.Exec(1, "CheckState");
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

    scene.EmitEntityCreated(avatarEntity);
    
    if (user != null)
        print("[Avatar Application] Created avatar for " + user.GetProperty("username"));
}

function ServerHandleUserDisconnected(connectionID, user) {
    var avatarEntityName = "Avatar" + connectionID;
    var avatarEntity = scene.GetEntityByName(avatarEntityName);
    if (avatarEntity != null) {
        scene.Removeentity(avatarEntity);

        if (user != null) {
        print("[Avatar Application] User " + user.GetProperty("username") + " disconnected, destroyed avatar entity.");
        }
    }
}
