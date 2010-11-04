// A simple walking avatar with physics

var motion_x = 0;
var motion_y = 0;
var rotate = 0;

var rotate_speed = 150.0;
var mouse_rotate_sensitivity = 0.3;
var move_force = 15.0;
var damping_force = 3.0;
var walk_anim_speed = 0.5;
var avatar_camera_distance = 7.0;
var avatar_camera_height = 1.0;
var avatar_camera_active = true;

// Create components
var avatar = me.GetOrCreateComponentRaw("EC_Avatar");
var placeable = me.GetOrCreateComponentRaw("EC_Placeable");
var rigidbody = me.GetOrCreateComponentRaw("EC_RigidBody");

// Set initial position
//var transform = placeable.transform;
//transform.position.x = 0;
//transform.position.y = 0;
//transform.position.z = 20;
//placeable.transform = transform;

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

// Set the avatar appearance. This creates the mesh & animationcontroller, once the avatar asset has loaded
// Note: for now, you need the default_avatar.xml in your bin/data/assets folder
avatar.appearanceId = "file://default_avatar.xml"

// Hook to update tick and physics world update tick
frame.Updated.connect(Update);
rigidbody.GetPhysicsWorld().Updated.connect(UpdatePhysics);

// Connect actions (handled serverside)
me.Action("Move").Triggered.connect(HandleMove);
me.Action("Stop").Triggered.connect(HandleStop);
me.Action("Rotate").Triggered.connect(HandleRotate);
me.Action("StopRotate").Triggered.connect(HandleStopRotate);
me.Action("MouseLookX").Triggered.connect(HandleMouseLookX);
me.Action("ToggleCamera").Triggered.connect(HandleToggleCamera);

// Create & hook input mapper. To be only done clientside, for the client's own avatar
CreateInputMapper();
CreateAvatarCamera();

function Update(frametime)
{       
    var placeable = me.GetComponentRaw("EC_Placeable");
    var rigidbody = me.GetComponentRaw("EC_RigidBody");

    if (rotate != 0)
    {
        var rotateVec = new Vector3df();
        rotateVec.z = -rotate_speed * rotate * frametime;
        rigidbody.Rotate(rotateVec);
    }
    
    UpdateAnimation(frametime);
    UpdateAvatarCamera(frametime);
}

function UpdatePhysics(frametime)
{
    // This should be a serverside function
    var placeable = me.GetComponentRaw("EC_Placeable");
    var rigidbody = me.GetComponentRaw("EC_RigidBody");

    // Apply motion forces
    // Todo: should apply as one and normalize so that diagonal strafing isn't faster
    if (motion_x != 0)
    {
        var impulseVec = new Vector3df();
        impulseVec.x = move_force * motion_x;
        impulseVec = placeable.GetRelativeVector(impulseVec);
        rigidbody.ApplyImpulse(impulseVec);
    }
    if (motion_y != 0)
    {
        var impulseVec = new Vector3df();
        impulseVec.y = -move_force * motion_y;
        impulseVec = placeable.GetRelativeVector(impulseVec);
        rigidbody.ApplyImpulse(impulseVec);
    }

    // Apply damping to velocity. Only do this if the body is active, because otherwise applying forces
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

function HandleMove(param)
{
    if (param == "forward")
        motion_x = 1;
    if (param == "back")
        motion_x = -1;
    if (param == "right")
        motion_y = 1;
    if (param == "left")
        motion_y = -1;
        
    SetAnimationState();
}

function HandleStop(param)
{
    if ((param == "forward") && (motion_x == 1))
        motion_x = 0;
    if ((param == "back") && (motion_x == -1))
        motion_x = 0;
    if ((param == "right") && (motion_y == 1))
        motion_y = 0;
    if ((param == "left") && (motion_y == -1))
        motion_y = 0;

    SetAnimationState();
}

function HandleRotate(param)
{
    if (param == "left")
        rotate = -1;
    if (param == "right")
        rotate = 1;
}

function HandleStopRotate(param)
{
    if ((param == "left") && (rotate == -1))
        rotate = 0;
    if ((param == "right") && (rotate == 1))
        rotate = 0;
}

function HandleMouseLookX(param)
{
    if (avatar_camera_active == false)
        return;

    var move = parseInt(param);
    var rigidbody = me.GetComponentRaw("EC_RigidBody");
    var rotateVec = new Vector3df();
    rotateVec.z = -mouse_rotate_sensitivity * move;
    rigidbody.Rotate(rotateVec);
}

function HandleToggleCamera()
{
    // For camera switching to work, must have both the freelookcamera & avatarcamera in the scene
    var freelookcameraentity = scene.GetEntityByNameRaw("FreeLookCamera");
    var avatarcameraentity = scene.GetEntityByNameRaw("AvatarCamera");
    if ((freelookcameraentity == null) || (avatarcameraentity == null))
        return;
    avatar_camera_active = !avatar_camera_active;

    var inputmapper = me.GetComponentRaw("EC_InputMapper");
    if (avatar_camera_active == false)
    {
        // Remove the movement controls from inputmapper
        DisableMoveControls(inputmapper);
        // Match the freelook camera's transform with the avatar camera
        freelookcameraentity.GetComponentRaw("EC_Placeable").transform = avatarcameraentity.GetComponentRaw("EC_Placeable").transform
        freelookcameraentity.GetComponentRaw("EC_OgreCamera").SetActive();
    }
    else
    {
        // Re-enable movement controls
        EnableMoveControls(inputmapper);
        avatarcameraentity.GetComponentRaw("EC_OgreCamera").SetActive();
    }
}

function SetAnimationState()
{
    // This should be a serverside function
    var animname = "Stand";
    if ((motion_x != 0) || (motion_y != 0))
        animname = "Walk";
    var animcontroller = me.GetComponentRaw("EC_AnimationController");
    if (animcontroller != null)
        animcontroller.animationState = animname;
}

function UpdateAnimation(frametime)
{
    // This should be a client only- function
    var animcontroller = me.GetComponentRaw("EC_AnimationController");
    var rigidbody = me.GetComponentRaw("EC_RigidBody");
    if ((animcontroller == null) || (rigidbody == null))
        return;
    var animname = animcontroller.animationState;
    if (animname != "")
        animcontroller.EnableExclusiveAnimation(animname, true, 0.5, 0.5, false);
    // If walk animation is playing, adjust its speed according to the avatar rigidbody velocity
    if (animcontroller.IsAnimationActive("Walk"))
    {
        // Note: on client the rigidbody does not exist, so the velocity is only a replicated attribute
        var velocity = rigidbody.linearVelocity;
        var walkspeed = Math.sqrt(velocity.x * velocity.x + velocity.y * velocity.y) * walk_anim_speed;
        animcontroller.SetAnimationSpeed("Walk", walkspeed);
    }
}

function CreateInputMapper()
{
    var inputmapper = me.GetOrCreateComponentRaw("EC_InputMapper");
    inputmapper.contextPriority = 101;
    inputmapper.takeMouseEventsOverQt = true;
    inputmapper.modifiersEnabled = false;
    inputmapper.RegisterMapping("Tab", "ToggleCamera", 1);

    EnableMoveControls(inputmapper);
}

function EnableMoveControls(inputmapper)
{
    inputmapper.RegisterMapping("W", "Move(forward)", 1);
    inputmapper.RegisterMapping("S", "Move(back)", 1);
    inputmapper.RegisterMapping("A", "Rotate(left)", 1);
    inputmapper.RegisterMapping("D", "Rotate(right))", 1);
    inputmapper.RegisterMapping("W", "Stop(forward)", 3);
    inputmapper.RegisterMapping("S", "Stop(back)", 3);
    inputmapper.RegisterMapping("A", "StopRotate(left)", 3);
    inputmapper.RegisterMapping("D", "StopRotate(right)", 3);

}

function DisableMoveControls(inputmapper)
{
    inputmapper.RemoveMapping("W", 1);
    inputmapper.RemoveMapping("S", 1);
    inputmapper.RemoveMapping("A", 1);
    inputmapper.RemoveMapping("D", 1);
    inputmapper.RemoveMapping("W", 3);
    inputmapper.RemoveMapping("S", 3);
    inputmapper.RemoveMapping("A", 3);
    inputmapper.RemoveMapping("D", 3);
}

function CreateAvatarCamera()
{
    if (scene.GetEntityByNameRaw("AvatarCamera") != null)
        return;

    var cameraentity = scene.CreateEntityRaw(scene.NextFreeIdLocal(), ["EC_Script"]);
    cameraentity.SetName("AvatarCamera");
    cameraentity.SetTemporary(true);
    
    var camera = cameraentity.GetOrCreateComponentRaw("EC_OgreCamera");
    var placeable = cameraentity.GetOrCreateComponentRaw("EC_Placeable");

    camera.AutoSetPlaceable();
    camera.SetActive();

    // Set initial position
    UpdateAvatarCamera();
}

function UpdateAvatarCamera()
{
    var cameraentity = scene.GetEntityByNameRaw("AvatarCamera");
    if (cameraentity == null)
        return;
    var cameraplaceable = cameraentity.GetComponentRaw("EC_Placeable");
    var avatarplaceable = me.GetComponentRaw("EC_Placeable");

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