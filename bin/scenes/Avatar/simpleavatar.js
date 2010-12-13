// A simple walking avatar with physics & third person camera

var rotate_speed = 150.0;
var mouse_rotate_sensitivity = 0.3;
var move_force = 15.0;
var damping_force = 3.0;
var walk_anim_speed = 0.5;
var avatar_camera_distance = 7.0;
var avatar_camera_height = 1.0;

var motion_x = 0;
var motion_y = 0;
var rotate = 0;

var isserver = server.IsRunning();
var own_avatar = false;

// Create avatar on server, and camera & inputmapper on client
if (isserver)
    ServerInitialize();
else
    ClientInitialize();

function ServerInitialize()
{
    var avatar = me.GetOrCreateComponentRaw("EC_Avatar");
    var rigidbody = me.GetOrCreateComponentRaw("EC_RigidBody");

    // Set the avatar appearance. This creates the mesh & animationcontroller, once the avatar asset has loaded
    // Note: for now, you need the default_avatar.xml in your bin/data/assets folder
    avatar.appearanceId = "local://default_avatar.xml"

    // Set physics properties
    var sizeVec = new Vector3df();
    sizeVec.z = 2;
    sizeVec.x = 0.5;
    sizeVec.y = 0.5;
    rigidbody.mass = 10;
    rigidbody.shapeType = 3; // Capsule
    rigidbody.size = sizeVec;
    var zeroVec = new Vector3df();
    rigidbody.angularFactor = zeroVec; // Set zero angular factor so that body stays upright

    // Hook to physics update
    rigidbody.GetPhysicsWorld().Updated.connect(ServerUpdatePhysics);

    // Hook to tick update for continuous rotation update
    frame.Updated.connect(ServerUpdate);

    // Connect actions
    me.Action("Move").Triggered.connect(ServerHandleMove);
    me.Action("Stop").Triggered.connect(ServerHandleStop);
    me.Action("Rotate").Triggered.connect(ServerHandleRotate);
    me.Action("StopRotate").Triggered.connect(ServerHandleStopRotate);
    me.Action("MouseLookX").Triggered.connect(ServerHandleMouseLookX);
}

function ServerUpdate(frametime)
{
    if (rotate != 0)
    {
        var rotateVec = new Vector3df();
        rotateVec.z = -rotate_speed * rotate * frametime;
        me.rigidbody.Rotate(rotateVec);
    }

    CommonUpdateAnimation(frametime);
}

function ServerUpdatePhysics(frametime)
{
    var placeable = me.placeable;
    var rigidbody = me.rigidbody;

    // Apply motion force
    // If diagonal motion, normalize
    if ((motion_x != 0) || (motion_y != 0))
    {
        var mag = 1.0 / Math.sqrt(motion_x * motion_x + motion_y * motion_y);
        var impulseVec = new Vector3df();
        impulseVec.x = mag * move_force * motion_x;
        impulseVec.y = -mag * move_force * motion_y;
        impulseVec = placeable.GetRelativeVector(impulseVec);
        rigidbody.ApplyImpulse(impulseVec);
    }

    // Apply damping. Only do this if the body is active, because otherwise applying forces
    // to a resting object wakes it up
    if (rigidbody.IsActive())
    {
        var dampingVec = rigidbody.GetLinearVelocity();
        dampingVec.x = -damping_force * dampingVec.x;
        dampingVec.y = -damping_force * dampingVec.y;
        dampingVec.z = 0;
        rigidbody.ApplyImpulse(dampingVec);
    }
}

function ServerHandleMove(param)
{
    // It is possible to query from whom the action did come from
    //var sender = server.GetActionSender();
    //if (sender)
    //    print("Move action from " + sender.GetName());

    if (param == "forward")
        motion_x = 1;
    if (param == "back")
        motion_x = -1;
    if (param == "right")
        motion_y = 1;
    if (param == "left")
        motion_y = -1;

    ServerSetAnimationState();
}

function ServerHandleStop(param)
{
    if ((param == "forward") && (motion_x == 1))
        motion_x = 0;
    if ((param == "back") && (motion_x == -1))
        motion_x = 0;
    if ((param == "right") && (motion_y == 1))
        motion_y = 0;
    if ((param == "left") && (motion_y == -1))
        motion_y = 0;

    ServerSetAnimationState();
}

function ServerHandleRotate(param)
{
    if (param == "left")
        rotate = -1;
    if (param == "right")
        rotate = 1;
}

function ServerHandleStopRotate(param)
{
    if ((param == "left") && (rotate == -1))
        rotate = 0;
    if ((param == "right") && (rotate == 1))
        rotate = 0;
}

function ServerHandleMouseLookX(param)
{
    var move = parseInt(param);
    var rotateVec = new Vector3df();
    rotateVec.z = -mouse_rotate_sensitivity * move;
    me.rigidbody.Rotate(rotateVec);
}

function ServerSetAnimationState()
{
    var animname = "Stand";
    if ((motion_x != 0) || (motion_y != 0))
        animname = "Walk";
    var animcontroller = me.animationcontroller;
    if (animcontroller != null)
        animcontroller.animationState = animname;
}

function ClientInitialize()
{
    // Check if this is our own avatar
    // Note: bad security. For now there's no checking who is allowed to invoke actions
    // on an entity, and we could theoretically control anyone's avatar
    if (me.GetName() == "Avatar" + client.GetConnectionID())
    {
        own_avatar = true;
        ClientCreateInputMapper();
        ClientCreateAvatarCamera();
    }

    // Hook to tick update to update visual effects (both own and others' avatars)
    frame.Updated.connect(ClientUpdate);
}

function ClientUpdate(frametime)
{
    // Tie enabled state of inputmapper to the enabled state of avatar camera
    if (own_avatar)
    {
        var avatarcameraentity = scene.GetEntityByNameRaw("AvatarCamera");
        var inputmapper = me.inputmapper;
        if ((avatarcameraentity != null) && (inputmapper != null))
        {
            var active = avatarcameraentity.ogrecamera.IsActive();
            if (inputmapper.enabled != active)
                inputmapper.enabled = active;
        }
        ClientUpdateAvatarCamera(frametime);
    }

    CommonUpdateAnimation(frametime);
}

function ClientCreateInputMapper()
{
    // Create a nonsynced inputmapper
    var inputmapper = me.GetOrCreateComponentRaw("EC_InputMapper", 2, false);
    inputmapper.contextPriority = 101;
    inputmapper.takeMouseEventsOverQt = true;
    inputmapper.modifiersEnabled = false;
    inputmapper.executionType = 2; // Execute actions on server
    inputmapper.RegisterMapping("W", "Move(forward)", 1);
    inputmapper.RegisterMapping("S", "Move(back)", 1);
    inputmapper.RegisterMapping("A", "Move(left)", 1);
    inputmapper.RegisterMapping("D", "Move(right))", 1);
    inputmapper.RegisterMapping("Up", "Move(forward)", 1);
    inputmapper.RegisterMapping("Down", "Move(back)", 1);
    inputmapper.RegisterMapping("Left", "Rotate(left)", 1);
    inputmapper.RegisterMapping("Right", "Rotate(right))", 1);
    inputmapper.RegisterMapping("W", "Stop(forward)", 3);
    inputmapper.RegisterMapping("S", "Stop(back)", 3);
    inputmapper.RegisterMapping("A", "Stop(left)", 3);
    inputmapper.RegisterMapping("D", "Stop(right)", 3);
    inputmapper.RegisterMapping("Up", "Stop(forward)", 3);
    inputmapper.RegisterMapping("Down", "Stop(back)", 3);
    inputmapper.RegisterMapping("Left", "StopRotate(left)", 3);
    inputmapper.RegisterMapping("Right", "StopRotate(right))", 3);
}

function ClientCreateAvatarCamera()
{
    if (scene.GetEntityByNameRaw("AvatarCamera") != null)
        return;

    var cameraentity = scene.CreateEntityRaw(scene.NextFreeIdLocal());
    cameraentity.SetName("AvatarCamera");
    cameraentity.SetTemporary(true);

    var camera = cameraentity.GetOrCreateComponentRaw("EC_OgreCamera");
    var placeable = cameraentity.GetOrCreateComponentRaw("EC_Placeable");

    camera.AutoSetPlaceable();
    camera.SetActive();

    // Set initial position
    ClientUpdateAvatarCamera();
}

function ClientUpdateAvatarCamera()
{
    var cameraentity = scene.GetEntityByNameRaw("AvatarCamera");
    if (cameraentity == null)
        return;
    var cameraplaceable = cameraentity.placeable;
    var avatarplaceable = me.placeable;

    var cameratransform = cameraplaceable.transform;
    var avatartransform = avatarplaceable.transform;
    var offsetVec = new Vector3df();
    offsetVec.x = -avatar_camera_distance;
    offsetVec.z = avatar_camera_height;
    offsetVec = avatarplaceable.GetRelativeVector(offsetVec);
    cameratransform.pos.x = avatartransform.pos.x + offsetVec.x;
    cameratransform.pos.y = avatartransform.pos.y + offsetVec.y;
    cameratransform.pos.z = avatartransform.pos.z + offsetVec.z;
    // Note: this is not nice how we have to fudge the camera rotation to get it to show the right things
    cameratransform.rot.x = 90;
    cameratransform.rot.z = avatartransform.rot.z - 90;

    cameraplaceable.transform = cameratransform;
}

function CommonUpdateAnimation(frametime)
{
    var animcontroller = me.animationcontroller;
    var rigidbody = me.rigidbody;
    if ((animcontroller == null) || (rigidbody == null))
        return;
    var animname = animcontroller.animationState;
    if (animname != "")
        animcontroller.EnableExclusiveAnimation(animname, true, 0.25, 0.25, false);
    // If walk animation is playing, adjust its speed according to the avatar rigidbody velocity
    if (animcontroller.IsAnimationActive("Walk"))
    {
        // Note: on client the rigidbody does not exist, so the velocity is only a replicated attribute
        var velocity = rigidbody.linearVelocity;
        var walkspeed = Math.sqrt(velocity.x * velocity.x + velocity.y * velocity.y) * walk_anim_speed;
        animcontroller.SetAnimationSpeed("Walk", walkspeed);
    }
}
