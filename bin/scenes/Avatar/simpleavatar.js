// !ref: local://crosshair.js
// !ref: local://default_avatar.xml

// A simple walking avatar with physics & third person camera
engine.IncludeFile("local://crosshair.js");
var rotate_speed = 150.0;
var mouse_rotate_sensitivity = 0.3;
var move_force = 15.0;
var fly_speed_factor = 0.25;
var damping_force = 3.0;
var walk_anim_speed = 0.5;
var avatar_mass = 10;

// Tracking motion with entity actions
var motion_x = 0;
var motion_y = 0;
var motion_z = 0;
var rotate = 0;

// Needed bools for logic
var isserver = server.IsRunning();
var own_avatar = false;
var flying = false;
var falling = false;
var fish_created = false;
var tripod = false;
var first_person = false;
var crosshair = null;
var is_mouse_look_locked_x = true;

// Animation detection
var standAnimName = "Stand";
var walkAnimName = "Walk";
var flyAnimName = "Fly";
var hoverAnimName = "Hover";
var sitAnimName = "SitOnGround";
var waveAnimName = "Wave";
var animsDetected = false;
var listenGesture = false;

// Camera variables
var checks_per_second = 30.0;
var time_since_check = 0;
var visibility_detection_enabled = true;
var avatar_camera_default_distance = 7.0;
var avatar_camera_distance = avatar_camera_default_distance;
var avatar_camera_preferred_distance = avatar_camera_distance;
var avatar_camera_height = 1.0;


// Create avatar on server, and camera & inputmapper on client
if (isserver) {
    ServerInitialize();
} else {
    ClientInitialize();
}

function ServerInitialize() {
    var avatar = me.GetOrCreateComponentRaw("EC_Avatar");
    var rigidbody = me.GetOrCreateComponentRaw("EC_RigidBody");

    // Set the avatar appearance. This creates the mesh & animationcontroller, once the avatar asset has loaded
    var r = avatar.appearanceRef;
    r.ref = "local://default_avatar.xml";
    avatar.appearanceRef = r;

    // Set physics properties
    var sizeVec = new Vector3df();
    sizeVec.z = 2.4;
    sizeVec.x = 0.5;
    sizeVec.y = 0.5;
    rigidbody.mass = avatar_mass;
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
    me.Action("ToggleFly").Triggered.connect(ServerHandleToggleFly);
    me.Action("Rotate").Triggered.connect(ServerHandleRotate);
    me.Action("StopRotate").Triggered.connect(ServerHandleStopRotate);
    me.Action("MouseLookX").Triggered.connect(ServerHandleMouseLookX);
    me.Action("Gesture").Triggered.connect(ServerHandleGesture);
    me.Action("Teleport").Triggered.connect(ServerHandleTeleport);

    rigidbody.PhysicsCollision.connect(ServerHandleCollision);
}

function ServerUpdate(frametime) {
    if (!animsDetected) {
        CommonFindAnimations();
    }

    if (rotate != 0) {
        var rotateVec = new Vector3df();
        rotateVec.z = -rotate_speed * rotate * frametime;
        me.rigidbody.Rotate(rotateVec);
    }

    CommonUpdateAnimation(frametime);
}

function ServerHandleTeleport(coords)
{
    var placeable = me.GetComponentRaw("EC_Placeable");
    var rigidbody = me.GetComponentRaw("EC_RigidBody");
    var xyz = coords.split(",");
    var newpos = placeable.transform;

    newpos.pos.x = parseFloat(xyz[0]);
    newpos.pos.y = parseFloat(xyz[1]);
    newpos.pos.z = parseFloat(xyz[2]);

    if (flying)
        me.placeable.transform = newpos;
    else
    {
        rigidbody.mass = 0;
        me.rigidbody = rigidbody;
        me.placeable.transform = newpos;
        rigidbody = me.rigidbody;
        rigidbody.mass = avatar_mass;
        me.rigidbody = rigidbody;
    }
}

function ServerHandleCollision(ent, pos, normal, distance, impulse, newCollision) {
    if (falling && newCollision) {
        falling = false;
        ServerSetAnimationState();
    }
}

function ServerUpdatePhysics(frametime) {
    var placeable = me.placeable;
    var rigidbody = me.rigidbody;

    if (!flying) {
        // Apply motion force
        // If diagonal motion, normalize
        if ((motion_x != 0) || (motion_y != 0)) {
            var mag = 1.0 / Math.sqrt(motion_x * motion_x + motion_y * motion_y);
            var impulseVec = new Vector3df();
            impulseVec.x = mag * move_force * motion_x;
            impulseVec.y = -mag * move_force * motion_y;
            impulseVec = placeable.GetRelativeVector(impulseVec);
            rigidbody.ApplyImpulse(impulseVec);
        }

        // Apply damping. Only do this if the body is active, because otherwise applying forces
        // to a resting object wakes it up
        if (rigidbody.IsActive()) {
            var dampingVec = rigidbody.GetLinearVelocity();
            dampingVec.x = -damping_force * dampingVec.x;
            dampingVec.y = -damping_force * dampingVec.y;
            dampingVec.z = 0;
            // Jump and wait for us to
            // come down before allowing new jump
            if (motion_z == 1 && !falling) {
                dampingVec.z = 75;
                motion_z = 0;
                falling = true;
            }
            rigidbody.ApplyImpulse(dampingVec);
        }
    } else {
        // Manually move the avatar placeable when flying
        // this has the downside of no collisions.
        // Feel free to reimplement properly with mass enabled.
        var av_placeable = me.placeable;
        var av_transform = av_placeable.transform;

        // Make a vector where we have moved
        var moveVec = new Vector3df();
        moveVec.x = motion_x * fly_speed_factor;
        moveVec.y = -motion_y * fly_speed_factor;
        moveVec.z = motion_z * fly_speed_factor;

        // Apply that with av looking direction to the current position
        var offsetVec = av_placeable.GetRelativeVector(moveVec);
        av_transform.pos.x = av_transform.pos.x + offsetVec.x;
        av_transform.pos.y = av_transform.pos.y + offsetVec.y;
        av_transform.pos.z = av_transform.pos.z + offsetVec.z;

        // This may look confusing. Its kind of a hack to tilt the avatar
        // when flying to the sides when you turn with A and D.
        // At the same time we need to lift up the Z of the av accorting to the angle of tilt
        if (motion_x != 0) {
            if (motion_y > 0 && av_transform.rot.x <= 5) {
                av_transform.rot.x = av_transform.rot.x + motion_y/2;
        }
            if (motion_y < 0 && av_transform.rot.x >= -5) {
                av_transform.rot.x = av_transform.rot.x + motion_y/2;
        }
            if (motion_y != 0 && av_transform.rot.x > 0) {
                av_transform.pos.z = av_transform.pos.z + (av_transform.rot.x * 0.0045); // magic number
        }
        if (motion_y != 0 && av_transform.rot.x < 0) {
                av_transform.pos.z = av_transform.pos.z + (-av_transform.rot.x * 0.0045); // magic number
        }
        }
        if (motion_y == 0 && av_transform.rot.x > 0) {
            av_transform.rot.x = av_transform.rot.x - 0.5;
    }
    if (motion_y == 0 && av_transform.rot.x < 0) {
            av_transform.rot.x = av_transform.rot.x + 0.5;
    }

        av_placeable.transform = av_transform;
    }
}

function ServerHandleMove(param) {
    // It is possible to query from whom the action did come from
    //var sender = server.GetActionSender();
    //if (sender)
    //    print("Move action from " + sender.GetName());

    if (param == "forward") {
        motion_x = 1;
    }
    if (param == "back") {
        motion_x = -1;
    }
    if (param == "right") {
        motion_y = 1;
    }
    if (param == "left") {
        motion_y = -1;
    }
    if (param == "up") {
        motion_z = 1;
    }
    if (param == "down") {
        motion_z = -1;
    }

    ServerSetAnimationState();
}

function ServerHandleStop(param) {
    if ((param == "forward") && (motion_x == 1)) {
        motion_x = 0;
    }
    if ((param == "back") && (motion_x == -1)) {
        motion_x = 0;
    }
    if ((param == "right") && (motion_y == 1)) {
        motion_y = 0;
    }
    if ((param == "left") && (motion_y == -1)) {
        motion_y = 0;
    }
    if ((param == "up") && (motion_z == 1)) {
        motion_z = 0;
    }
    if ((param == "down") && (motion_z == -1)) {
        motion_z = 0;
    }
    if (param == "all") {
        motion_x = 0;
        motion_y = 0;
        motion_z = 0;
    }

    ServerSetAnimationState();
}

function ServerHandleToggleFly() {
    var rigidbody = me.rigidbody;

    flying = !flying;
    if (flying) {
        rigidbody.mass = 0;
    } else {
        // Reset the x rot if left
        var av_placeable = me.placeable;
        var av_transform = av_placeable.transform;
        if (av_transform.rot.x != 0) {
            av_transform.rot.x = 0;
            av_placeable.transform = av_transform;
        }

        // Set mass back for collisions
        rigidbody.mass = avatar_mass;
        // Push avatar a bit to the fly direction
        // so the motion does not just stop to a wall
        var moveVec = new Vector3df();
        moveVec.x = motion_x * 120;
        moveVec.y = -motion_y * 120;
        moveVec.z = motion_z * 120;
        var pushVec = av_placeable.GetRelativeVector(moveVec);
        rigidbody.ApplyImpulse(pushVec);
    }
    ServerSetAnimationState();
}

function ServerHandleRotate(param) {
    if (param == "left") {
        rotate = -1;
    }
    if (param == "right") {
        rotate = 1;
    }
}

function ServerHandleStopRotate(param) {
    if ((param == "left") && (rotate == -1)) {
        rotate = 0;
    }
    if ((param == "right") && (rotate == 1)) {
        rotate = 0;
    }
    if (param == "all") {
        rotate = 0;
    }
}

function ServerHandleMouseLookX(param) {
    var move = parseInt(param);
    var rotateVec = new Vector3df();
    rotateVec.z = -mouse_rotate_sensitivity * move;
    me.rigidbody.Rotate(rotateVec);
}

function ServerHandleGesture(gestureName) {
    var animName = "";
    if (gestureName == "wave") {
        animName = waveAnimName;
    }
    if (animName == "") {
        return;
    }

    // Update the variable to sync to client if changed
    var animcontroller = me.animationcontroller;
    if (animcontroller != null) {
        if (animcontroller.animationState != animName) {
            animcontroller.animationState = animName;
        }
    }
}

function ServerSetAnimationState() {
    // Not flying: Stand, Walk or Crouch
    var animName = standAnimName;
    if ((motion_x != 0) || (motion_y != 0)) {
        animName = walkAnimName;
    } else if (motion_z == -1 && !falling) {
        animName = sitAnimName;
    }

    // Flying: Fly if moving in x-axis, otherwise hover
    if (flying || falling) {
        animName = flyAnimName;
        if (motion_x == 0)
            animName = hoverAnimName;
    }

    if (animName == "") {
        return;
    }

    // Update the variable to sync to client if changed
    var animcontroller = me.animationcontroller;
    if (animcontroller != null) {
        if (animcontroller.animationState != animName) {
            animcontroller.animationState = animName;
        }
    }
}

function ClientInitialize() {
    // Check if this is our own avatar
    // Note: bad security. For now there's no checking who is allowed to invoke actions
    // on an entity, and we could theoretically control anyone's avatar
    if (me.GetName() == "Avatar" + client.GetConnectionID()) {
        own_avatar = true;
        ClientCreateInputMapper();
        ClientCreateAvatarCamera();
        crosshair = new Crosshair(/*bool useLabelInsteadOfCursor*/ false);
        var soundlistener = me.GetOrCreateComponentRaw("EC_SoundListener");
        soundlistener.active = true;

        me.Action("MouseScroll").Triggered.connect(ClientHandleMouseScroll);
        me.Action("Zoom").Triggered.connect(ClientHandleKeyboardZoom);
        me.Action("ToggleTripod").Triggered.connect(ClientHandleToggleTripod);
        me.Action("MouseLookX").Triggered.connect(ClientHandleTripodLookX);
        me.Action("MouseLookY").Triggered.connect(ClientHandleTripodLookY);
        me.Action("CheckState").Triggered.connect(ClientCheckState);
        
        // Inspect the login avatar url property
        var avatarurl = client.GetLoginProperty("avatarurl");
        if (avatarurl && avatarurl.length > 0)
        {
            var avatar = me.GetOrCreateComponentRaw("EC_Avatar");
            var r = avatar.appearanceRef;
            r.ref = avatarurl;
            avatar.appearanceRef = r;
            debug.Log("Avatar from login parameters enabled: " + avatarurl);
        }
    }
    else
    {
        // Make hovering name tag for other clients
        var clientName = me.GetComponentRaw("EC_Name");
        if (clientName != null) {
            // Description holds the actual login name
            if (clientName.description != "") {
                var hoveringWidget = me.GetOrCreateComponentRaw("EC_HoveringWidget", 2, false);
                if (hoveringWidget != null) {
                    hoveringWidget.SetNetworkSyncEnabled(false);
                    hoveringWidget.SetTemporary(true);
                    hoveringWidget.InitializeBillboards();
                    hoveringWidget.SetButtonsDisabled(true);
                    hoveringWidget.SetText(clientName.description);
                    hoveringWidget.SetFontSize(100);
                    hoveringWidget.SetTextHeight(200);
                    hoveringWidget.Show();
                }
            }
        }
    }

    // Hook to tick update to update visual effects (both own and others' avatars)
    frame.Updated.connect(ClientUpdate);
}

function IsCameraActive()
{
    var cameraentity = scene.GetEntityByNameRaw("AvatarCamera");
    if (cameraentity == null)
        return false;
    var camera = cameraentity.ogrecamera;
    return camera.IsActive();
}

function ClientHandleToggleTripod()
{
    var cameraentity = scene.GetEntityByNameRaw("AvatarCamera");
    if (cameraentity == null)
        return;

    var camera = cameraentity.ogrecamera;
    if (camera.IsActive() == false)
    {
        first_person = false;
        crosshair.hide();
        tripod = false;
        return;
    }

    if (tripod == false)
        tripod = true;
    else
        tripod = false;
}

function ClientHandleTripodLookX(param)
{
    if (tripod)
    {
        var cameraentity = scene.GetEntityByNameRaw("AvatarCamera");
        if (cameraentity == null)
            return;
        var cameraplaceable = cameraentity.placeable;
        var cameratransform = cameraplaceable.transform;

        var move = parseInt(param);
        cameratransform.rot.z -= mouse_rotate_sensitivity * move;
        cameraplaceable.transform = cameratransform;
    }
}

// Note: this function also handles mouse look Y for third person mode
function ClientHandleTripodLookY(param)
{
    if (!tripod && is_mouse_look_locked_x)
        return;

    var cameraentity = scene.GetEntityByNameRaw("AvatarCamera");
    if (cameraentity == null)
        return;
    var cameraplaceable = cameraentity.placeable;
    var cameratransform = cameraplaceable.transform;

    var move = parseInt(param);
    cameratransform.rot.x -= mouse_rotate_sensitivity * move;

    if (cameratransform.rot.x < 0)
        cameratransform.rot.x = 0;
    if (cameratransform.rot.x > 180)
        cameratransform.rot.x = 180;

    cameraplaceable.transform = cameratransform;
}

function ClientUpdate(frametime)
{
    // Tie enabled state of inputmapper to the enabled state of avatar camera
    if (own_avatar) {
        var avatarcameraentity = scene.GetEntityByNameRaw("AvatarCamera");
        var inputmapper = me.inputmapper;
        if ((avatarcameraentity != null) && (inputmapper != null)) {
            var active = avatarcameraentity.ogrecamera.IsActive();
            if (inputmapper.enabled != active) {
                inputmapper.enabled = active;
                // If we went offline, stop all movement/rotation
                // so eg if you have W down you donw continue walking forward
                // without the release for W being ever sent (as we disable the mapper right here)
                if (!active) {
                    me.Exec(2, "Stop", "all");
                    me.Exec(2, "StopRotate", "all");
                }
            }
        }
        ClientUpdateAvatarCamera(frametime);
    }

    if (!animsDetected) {
        CommonFindAnimations();
    }
    CommonUpdateAnimation(frametime);

    // Uncomment this to attach a fish to the avatar's head
    //if (!fish_created)
    //    CreateFish();
}

function ClientCreateInputMapper() {
    // Create a nonsynced inputmapper
    var inputmapper = me.GetOrCreateComponentRaw("EC_InputMapper", 2, false);
    inputmapper.contextPriority = 101;
    inputmapper.takeMouseEventsOverQt = false;
    inputmapper.takeKeyboardEventsOverQt = false;
    inputmapper.modifiersEnabled = false;
    inputmapper.keyrepeatTrigger = false; // Disable repeat keyevent sending over network, not needed and will flood network
    inputmapper.executionType = 2; // Execute actions on server
    inputmapper.RegisterMapping("W", "Move(forward)", 1);
    inputmapper.RegisterMapping("S", "Move(back)", 1);
    inputmapper.RegisterMapping("A", "Move(left)", 1);
    inputmapper.RegisterMapping("D", "Move(right))", 1);
    inputmapper.RegisterMapping("Up", "Move(forward)", 1);
    inputmapper.RegisterMapping("Down", "Move(back)", 1);
    inputmapper.RegisterMapping("Left", "Rotate(left)", 1);
    inputmapper.RegisterMapping("Right", "Rotate(right))", 1);
    inputmapper.RegisterMapping("F", "ToggleFly()", 1);
    inputmapper.RegisterMapping("Q", "Gesture(wave)", 1);
    inputmapper.RegisterMapping("Space", "Move(up)", 1);
    inputmapper.RegisterMapping("C", "Move(down)", 1);
    inputmapper.RegisterMapping("W", "Stop(forward)", 3);
    inputmapper.RegisterMapping("S", "Stop(back)", 3);
    inputmapper.RegisterMapping("A", "Stop(left)", 3);
    inputmapper.RegisterMapping("D", "Stop(right)", 3);
    inputmapper.RegisterMapping("Up", "Stop(forward)", 3);
    inputmapper.RegisterMapping("Down", "Stop(back)", 3);
    inputmapper.RegisterMapping("Left", "StopRotate(left)", 3);
    inputmapper.RegisterMapping("Right", "StopRotate(right))", 3);
    inputmapper.RegisterMapping("Space", "Stop(up)", 3);
    inputmapper.RegisterMapping("C", "Stop(down)", 3);

    // Connect gestures
    var inputContext = inputmapper.GetInputContext();
    inputContext.GestureStarted.connect(GestureStarted);
    inputContext.GestureUpdated.connect(GestureUpdated);
    inputContext.MouseMove.connect(ClientHandleMouseMove);

    // Local camera matter for mouse scroll
    var inputmapper = me.GetOrCreateComponentRaw("EC_InputMapper", "CameraMapper", 2, false);
    inputmapper.SetNetworkSyncEnabled(false);
    inputmapper.contextPriority = 100;
    inputmapper.takeMouseEventsOverQt = true;
    inputmapper.modifiersEnabled = false;
    inputmapper.executionType = 1; // Execute actions locally
    inputmapper.RegisterMapping("T", "ToggleTripod", 1);
    inputmapper.RegisterMapping("+", "Zoom(in)", 1);
    inputmapper.RegisterMapping("-", "Zoom(out)", 1);
}

function ClientCreateAvatarCamera() {
    if (scene.GetEntityByNameRaw("AvatarCamera") != null) {
        return;
    }

    var cameraentity = scene.CreateEntityRaw(scene.NextFreeIdLocal());
    cameraentity.SetName("AvatarCamera");
    cameraentity.SetTemporary(true);

    var camera = cameraentity.GetOrCreateComponentRaw("EC_OgreCamera");
    var placeable = cameraentity.GetOrCreateComponentRaw("EC_Placeable");

    camera.AutoSetPlaceable();
    camera.SetActive();

    // Note: this is not nice how we have to fudge the camera rotation to get it to show the right things
    var cameratransform = placeable.transform;
    cameratransform.rot.x = 90;
    placeable.transform = cameratransform;

    // Set initial position
    ClientUpdateAvatarCamera();
}

function GestureStarted(gestureEvent)
{
    if (!IsCameraActive())
        return;
    if (gestureEvent.GestureType() == Qt.PanGesture)
    {
        listenGesture = true;
        var x = new Number(gestureEvent.Gesture().offset.toPoint().x());
        me.Exec(2, "MouseLookX", x.toString());
        gestureEvent.Accept();
    }
    else if (gestureEvent.GestureType() == Qt.PinchGesture)
        gestureEvent.Accept();
}

function GestureUpdated(gestureEvent)
{
    if (!IsCameraActive())
        return;

    if (gestureEvent.GestureType() == Qt.PanGesture && listenGesture == true)
    {
        // Rotate avatar with X pan gesture
        delta = gestureEvent.Gesture().delta.toPoint();
        var x = new Number(delta.x());
        me.Exec(2, "MouseLookX", x.toString());

        // Start walking or stop if total Y len of pan gesture is 100
        var walking = false;
        if (me.animationcontroller.animationState == walkAnimName)
            walking = true;
        var totalOffset = gestureEvent.Gesture().offset.toPoint();
        if (totalOffset.y() < -100)
        {
            if (walking) {
                me.Exec(2, "Stop", "forward");
                me.Exec(2, "Stop", "back");
            } else
                me.Exec(2, "Move", "forward");
            listenGesture = false;
        }
        else if (totalOffset.y() > 100)
        {
            if (walking) {
                me.Exec(2, "Stop", "forward");
                me.Exec(2, "Stop", "back");
            } else
                me.Exec(2, "Move", "back");
            listenGesture = false;
        }
        gestureEvent.Accept();
    }
    else if (gestureEvent.GestureType() == Qt.PinchGesture)
    {
        var scaleChange = gestureEvent.Gesture().scaleFactor - gestureEvent.Gesture().lastScaleFactor;
        if (scaleChange > 0.1 || scaleChange < -0.1)
            ClientHandleMouseScroll(scaleChange * 100);
        gestureEvent.Accept();
    }
}

function ClientHandleKeyboardZoom(direction) {
    if (direction == "in") {
        ClientHandleMouseScroll(10);
    } else if (direction == "out") {
        ClientHandleMouseScroll(-10);
    }
}

function ClientHandleMouseScroll(relativeScroll)
{
    if (!IsCameraActive())
        return;

    var moveAmount = 0;
    if (relativeScroll < 0 && avatar_camera_default_distance < 500) {
        if (relativeScroll < -50)
            moveAmount = 2;
        else
            moveAmount = 1;
    } else if (relativeScroll > 0 && avatar_camera_default_distance > 0) {
        if (relativeScroll > 50)
            moveAmount = -2
        else
            moveAmount = -1;
    }
    if (moveAmount != 0)
    {   
        avatar_camera_default_distance = avatar_camera_default_distance + moveAmount;
        // Clamp distance  to be between 1 and 500
        if (avatar_camera_default_distance < -0.5)
            avatar_camera_default_distance = -0.5;
        else if (avatar_camera_default_distance > 500)
            avatar_camera_default_distance = 500;
            
        if (avatar_camera_default_distance <= 0)
        {
            first_person = true;
            crosshair.show();
        }
        else
        {
            first_person = false;
            crosshair.hide();
        }
        
        ClientCheckState();
    }
}

// Finds visible distance for avatar camera
// \note To get smoother camera movement we need to detect objects
//       before they occlude the view. This should be done with
//       with region query instead of raycasting.
function FindVisibleCameraDistance() {
    if(first_person || flying || !visibility_detection_enabled)
        return;
        
    var cameraentity = scene.GetEntityByNameRaw("AvatarCamera");
    var avatarplaceable = me.placeable;
    var cameratransform = cameraentity.placeable.transform;
    var avatartransform = me.placeable.transform;
    
    // Give offset to the source of the ray so we don't hit our own avatar when moving.
    var avatarposition = new Vector3df();
    avatarposition.x = -0.1;
    avatarposition.z = 0.7;
    avatarposition = avatarplaceable.GetRelativeVector(avatarposition);
    avatarposition.x += avatartransform.pos.x;
    avatarposition.y += avatartransform.pos.y;
    avatarposition.z += avatartransform.pos.z;
    
    // We don't need to calculate the actual 'default' position for the 
    // camera since all we care about is the direction.
    var cameraposition = new Vector3df();
    cameraposition.x = cameratransform.pos.x;
    cameraposition.y = cameratransform.pos.y;
    cameraposition.z = avatarposition.z;
    
    var raycastResult = renderer.RaycastFromTo(avatarposition, cameraposition);
    if(raycastResult.entity != null) {
        avatar_camera_preferred_distance = distance(avatarposition, raycastResult.pos);
        if(avatar_camera_preferred_distance <= 0.1) {
            avatar_camera_preferred_distance = 0.1;
        } else if(avatar_camera_preferred_distance >= avatar_camera_default_distance) {
            avatar_camera_preferred_distance = avatar_camera_default_distance;
        }
    }
}

function distance(v1, v2) {
    var a = Math.pow((v1.x - v2.x), 2);
    var b = Math.pow((v1.y - v2.y), 2);
    var c = Math.pow((v1.z - v2.z), 2);
    return Math.sqrt(a + b + c);
}

// Moves the actual distance of the camera towards the 'preferred' visible distance.
// This provides smoothing to camera movement between visible distances.
function AdjustCameraDistance() {
    if(first_person) {
        avatar_camera_distance = avatar_camera_default_distance;
        return
    }
    if(Math.abs(avatar_camera_preferred_distance - avatar_camera_distance) < 0.4) {
        avatar_camera_distance = avatar_camera_preferred_distance;
        return
    }
    if(avatar_camera_preferred_distance > avatar_camera_distance) {
        avatar_camera_distance += (avatar_camera_preferred_distance - avatar_camera_distance) / 25;
    } else if(avatar_camera_preferred_distance < avatar_camera_distance) {
        avatar_camera_distance -= (avatar_camera_distance - avatar_camera_preferred_distance) / 5;
    }
}

function ClientUpdateAvatarCamera(frametime) {
    if (!tripod)
    {
        if(frametime == null)
            frametime = 0;
    
        var cameraentity = scene.GetEntityByNameRaw("AvatarCamera");
        if (cameraentity == null)
            return;
        var cameraplaceable = cameraentity.placeable;
        var avatarplaceable = me.placeable;
        
        time_since_check += frametime;
        if(time_since_check >= 1/checks_per_second) {
            FindVisibleCameraDistance();
            time_since_check = 0; 
        }
        AdjustCameraDistance();

        var cameratransform = cameraplaceable.transform;
        var avatartransform = avatarplaceable.transform;
        var offsetVec = new Vector3df();
        offsetVec.x = -avatar_camera_distance;
        offsetVec.z = avatar_camera_height;
        offsetVec = avatarplaceable.GetRelativeVector(offsetVec);
        cameratransform.pos.x = avatartransform.pos.x + offsetVec.x;
        cameratransform.pos.y = avatartransform.pos.y + offsetVec.y;
        cameratransform.pos.z = avatartransform.pos.z + offsetVec.z;
        cameratransform.rot.z = avatartransform.rot.z - 90;
        cameraplaceable.transform = cameratransform;
    }
}

function ClientCheckState()
{    
    var cameraentity = scene.GetEntityByNameRaw("AvatarCamera");
    var avatar_placeable = me.GetComponentRaw("EC_Placeable");
    
    // If ent got destroyed or something fatal, return cursor
    if (cameraentity == null || avatar_placeable == null)
    {
        if (crosshair.isActive())
            crosshair.hide();
        return;
    }
        
    if (!first_person)
    {
        if (crosshair.isActive())
            crosshair.hide();
        
        /* 
        This wont work, would be nice to hide the mesh
        but it sync to other clients. Seems not doable to make
        this kind of local change in js?!
        
        if (!avatar_placeable.visible)
        {
            avatar_placeable.SetUpdateMode(2);
            avatar_placeable.visible = true;
            avatar_placeable.SetUpdateMode(0);
        }
        */
        return;
    }
    else
    {
        // We might be in 1st person mode but camera might not be active
        // hide curson and show av
        if (!cameraentity.ogrecamera.IsActive())
        {
            if (crosshair.isActive())
                crosshair.hide();
                
            /* 
            This wont work, would be nice to hide/show the mesh
            but it sync to other clients. Seems not doable to make
            this kind of local change in js?!
            
            if (!avatar_placeable.visible)
            {
                avatar_placeable.SetUpdateMode(2);
                avatar_placeable.visible = true;
                avatar_placeable.SetUpdateMode(0);
            }
            */
        }
        else
        {
            // 1st person mode and camera is active
            // show curson and av
            if (!crosshair.isActive())
                crosshair.show();
            
            /* 
            This wont work, would be nice to hide/show the mesh
            but it sync to other clients. Seems not doable to make
            this kind of local change in js?!
            
            if (avatar_placeable.visible)
            {
                avatar_placeable.SetUpdateMode(2);
                avatar_placeable.visible = false;
                avatar_placeable.SetUpdateMode(0);
            }
            */
        }
    }
}

function ClientHandleMouseMove(mouseevent)
{
    if (mouseevent.IsRightButtonDown())
        LockMouseMove(mouseevent.relativeX, mouseevent.relativeY);
    else
        is_mouse_look_locked_x = true;
    ClientCheckState();
    
    if (mouseevent.IsItemUnderMouse())
    {
        // If there is a graphics widget here disable first person mode
        if (first_person)
        {
            ClientHandleMouseScroll(-1); 
            ClientCheckState();
            return;
        }
    }
    
    if (!first_person)
    {
        // \note Right click look also hides/shows cursor, so this is to ensure that the cursor is visible in non-fps mode
        if (!crosshair.isUsingLabel)
        {
            if (input.IsMouseCursorVisible())
            {
                var cursor = QApplication.overrideCursor;
                if (cursor == null)
                    return;
                if (crosshair.cursor.pixmap() == cursor.pixmap())
                    QApplication.restoreOverrideCursor();
            }
        }
        return;
    }

    if (input.IsMouseCursorVisible())
    {
        input.SetMouseCursorVisible(false);
        if (!crosshair.isUsingLabel)
            QApplication.setOverrideCursor(crosshair.cursor);
    }

    var cameraentity = scene.GetEntityByNameRaw("AvatarCamera");
    if (cameraentity == null)
        return;
        
    // Dont move av rotation if we are not the active cam
    if (!cameraentity.ogrecamera.IsActive())
        return;

    var cameraplaceable = cameraentity.placeable;
    var cameratransform = cameraplaceable.transform;

    if (mouseevent.relativeX != 0)
        me.Exec(2, "MouseLookX", String(mouse_rotate_sensitivity*2 * parseInt(mouseevent.relativeX)));
    if (mouseevent.relativeY != 0)
        cameratransform.rot.x -= (mouse_rotate_sensitivity/3) * parseInt(mouseevent.relativeY);
        
    // Dont let the 1st person flip vertically, 180 deg view angle
    if (cameratransform.rot.x < 0)
        cameratransform.rot.x = 0;
    if (cameratransform.rot.x > 180)
        cameratransform.rot.x = 180;

    cameraplaceable.transform = cameratransform;
}

function LockMouseMove(x,y)
{
    if (Math.abs(y) > Math.abs(x))
        is_mouse_look_locked_x = false;
    else
        is_mouse_look_locked_x = true;
}

function CommonFindAnimations() {
    var animcontrol = me.animationcontroller;
    var availableAnimations = animcontrol.GetAvailableAnimations();
    if (availableAnimations.length > 0) {
        // Detect animation names
        var searchAnims = [standAnimName, walkAnimName, flyAnimName, hoverAnimName, sitAnimName, waveAnimName];
        for(var i=0; i<searchAnims.length; i++) {
            var animName = searchAnims[i];
            if (availableAnimations.indexOf(animName) == -1) {
                // Disable this animation by setting it to a empty string
                print("Could not find animation for:", animName, " - disabling animation");
                searchAnims[i] = "";
            }
        }

        // Assign the possible empty strings for
        // not found anims back to the variables
        standAnimName = searchAnims[0];
        walkAnimName = searchAnims[1];
        flyAnimName = searchAnims[2];
        hoverAnimName = searchAnims[3];
        sitAnimName = searchAnims[4];

        animsDetected = true;
    }
}

function CommonUpdateAnimation(frametime) {
    if (!animsDetected) {
        return;
    }

    var animcontroller = me.animationcontroller;
    var rigidbody = me.rigidbody;
    if ((animcontroller == null) || (rigidbody == null)) {
        return;
    }

    var animName = animcontroller.animationState;

    // Enable animation, skip with headless server
    if (animName != "" && !framework.IsHeadless()) {
        // Do custom speeds for certain anims
        if (animName == hoverAnimName) {
            animcontroller.SetAnimationSpeed(animName, 0.25);
    }
        if (animName == sitAnimName) { // Does not affect the anim speed on jack at least?!
            animcontroller.SetAnimationSpeed(animName, 0.5);
    }
        if (animName == waveAnimName) {
            animcontroller.SetAnimationSpeed(animName, 0.75);
    }
        // Enable animation
        if (!animcontroller.IsAnimationActive(animName)) {
            // Gestures with non exclusive
            if (animName == waveAnimName) {
                animcontroller.EnableAnimation(animName, false, 0.25, 0.25, false);
            // Normal anims exclude others
        } else {
                animcontroller.EnableExclusiveAnimation(animName, true, 0.25, 0.25, false);
        }
        }
    }

    // If walk animation is playing, adjust its speed according to the avatar rigidbody velocity
    if (animName != ""  && animcontroller.IsAnimationActive(walkAnimName)) {
        // Note: on client the rigidbody does not exist, so the velocity is only a replicated attribute
        var velocity = rigidbody.linearVelocity;
        var walkspeed = Math.sqrt(velocity.x * velocity.x + velocity.y * velocity.y) * walk_anim_speed;
        animcontroller.SetAnimationSpeed(walkAnimName, walkspeed);
    }
}

function CreateFish() {
    // Note: attaching meshes to bone of another mesh is strictly client-only! It does not replicate.
    // Therefore this needs to be run locally on every client
    var avatarmesh = me.GetComponentRaw("EC_Mesh", "");
    // Do not act until the actual avatar has been created
    if ((avatarmesh) && (avatarmesh.HasMesh())) {
        // Create a local mesh component into the same entity
        var fishmesh = me.GetOrCreateComponentRaw("EC_Mesh", "fish", 2, false);
        var r = fishmesh.meshRef;
        if (r.ref != "local://fish.mesh") {
            r.ref = "local://fish.mesh";
            fishmesh.meshRef = r;
        }

        // Then we must wait until the fish mesh component has actually loaded the mesh asset
        if (fishmesh.HasMesh()) {
            fishmesh.AttachMeshToBone(avatarmesh, "Bip01_Head");
            fish_created = true;
            var t = fishmesh.nodeTransformation;
            var scaleVec = new Vector3df();
            scaleVec.x = 0.1;
            scaleVec.y = 0.1;
            scaleVec.z = 0.1;
            t.scale = scaleVec;
            fishmesh.nodeTransformation = t;
        }
    }
}
