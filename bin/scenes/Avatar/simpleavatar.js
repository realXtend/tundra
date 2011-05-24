// !ref: local://default_avatar.avatar
// !ref: local://crosshair.js

if (!server.IsRunning() && !framework.IsHeadless())
{
    engine.ImportExtension("qt.core");
    engine.ImportExtension("qt.gui");
    engine.IncludeFile("local://crosshair.js");
}

// A simple walking avatar with physics & 1st/3rd person camera
var rotate_speed = 100.0;
var mouse_rotate_sensitivity = 0.2;
var move_force = 15.0;
var fly_speed_factor = 0.25;
var damping_force = 3.0;
var walk_anim_speed = 0.5;
var avatar_camera_height = 1.0;
var avatar_mass = 10;

// Tracking motion with entity actions
var motion_x = 0;
var motion_y = 0;
var motion_z = 0;

// Clientside yaw, pitch & rotation state
var yaw = 0;
var pitch = 0;
var rotate = 0;

// Needed bools for logic
var isserver = server.IsRunning();
var own_avatar = false;
var flying = false;
var falling = false;
var crosshair = null;

// Animation detection
var standAnimName = "Stand";
var walkAnimName = "Walk";
var flyAnimName = "Fly";
var hoverAnimName = "Hover";
var animList = [standAnimName, walkAnimName, flyAnimName, hoverAnimName];

var animsDetected = false;
var listenGesture = false;

// Create avatar on server, and camera & inputmapper on client
if (isserver) {
    ServerInitialize();
} else {
    ClientInitialize();
}

function ServerInitialize() {

    // Create the avatar component & set the avatar appearance. The avatar component will create the mesh & animationcontroller, once the avatar asset has loaded
    var avatar = me.GetOrCreateComponent("EC_Avatar");
    var r = avatar.appearanceRef;
    r.ref = "local://default_avatar.avatar";
    avatar.appearanceRef = r;

    // Create rigid body component and set physics properties
    var rigidbody = me.GetOrCreateComponent("EC_RigidBody");
    var sizeVec = new Vector3df();
    sizeVec.x = 0.5;
    sizeVec.y = 2.4;
    sizeVec.z = 0.5;
    rigidbody.mass = avatar_mass;
    rigidbody.shapeType = 3; // Capsule
    rigidbody.size = sizeVec;
    var zeroVec = new Vector3df();
    rigidbody.angularFactor = zeroVec; // Set zero angular factor so that body stays upright

    // Create dynamic component attributes for disabling/enabling functionality, and for camera distance / 1st/3rd mode
    var attrs = me.dynamiccomponent;
    attrs.CreateAttribute("bool", "enableWalk");
    attrs.CreateAttribute("bool", "enableJump");
    attrs.CreateAttribute("bool", "enableFly");
    attrs.CreateAttribute("bool", "enableRotate");
    attrs.CreateAttribute("bool", "enableAnimation");
    attrs.CreateAttribute("bool", "enableZoom");
    attrs.CreateAttribute("real", "cameraDistance");
    attrs.SetAttribute("enableWalk", true);
    attrs.SetAttribute("enableJump", true);
    attrs.SetAttribute("enableFly", true);
    attrs.SetAttribute("enableRotate", true);
    attrs.SetAttribute("enableAnimation", true);
    attrs.SetAttribute("enableZoom", true);
    attrs.SetAttribute("cameraDistance", 7.0);

    // Create an inactive proximitytrigger, so that other proximitytriggers can detect the avatar
    // var proxtrigger = me.GetOrCreateComponent("EC_ProximityTrigger");
    // proxtrigger.active = false;

    // Hook to physics update
    rigidbody.GetPhysicsWorld().Updated.connect(ServerUpdatePhysics);

    // Hook to tick update for animation update
    frame.Updated.connect(ServerUpdate);

    // Connect actions. These come from the client side inputmapper
    me.Action("Move").Triggered.connect(ServerHandleMove);
    me.Action("Stop").Triggered.connect(ServerHandleStop);
    me.Action("ToggleFly").Triggered.connect(ServerHandleToggleFly);
    me.Action("SetRotation").Triggered.connect(ServerHandleSetRotation);

    rigidbody.PhysicsCollision.connect(ServerHandleCollision);
}

function ServerUpdate(frametime) {
    var attrs = me.dynamiccomponent;

    if (!animsDetected) {
        CommonFindAnimations();
    }

    // If walk enable was toggled off, make sure the motion state is cleared
    if (!attrs.GetAttribute("enableWalk"))
    {
        motion_x = 0;
        motion_z = 0;
    }
    
    // If flying enable was toggled off, but we are still flying, disable now
    if ((flying) && (!attrs.GetAttribute("enableFly")))
        ServerSetFlying(false);

    CommonUpdateAnimation(frametime);
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
    var attrs = me.dynamiccomponent;

    if (!flying) {
        // Apply motion force
        // If diagonal motion, normalize
        if ((motion_x != 0) || (motion_z != 0)) {
            var mag = 1.0 / Math.sqrt(motion_x * motion_x + motion_z * motion_z);
            var impulseVec = new Vector3df();
            impulseVec.x = mag * move_force * motion_x;
            impulseVec.z = -mag * move_force * motion_z;
            impulseVec = placeable.GetRelativeVector(impulseVec);
            rigidbody.ApplyImpulse(impulseVec);
        }

        // Apply jump
        if (motion_y == 1 && !falling) {
            if (attrs.GetAttribute("enableJump")) {
                var jumpVec = new Vector3df();
                jumpVec.y = 75;
                motion_y = 0;
                falling = true;
                rigidbody.ApplyImpulse(jumpVec);
            }
        }

        // Apply damping. Only do this if the body is active, because otherwise applying forces
        // to a resting object wakes it up
        if (rigidbody.IsActive()) {
            var dampingVec = rigidbody.GetLinearVelocity();
            dampingVec.x = -damping_force * dampingVec.x;
            dampingVec.y = 0;
            dampingVec.z = -damping_force * dampingVec.z;
            rigidbody.ApplyImpulse(dampingVec);
        }
    } else {
        // Manually move the avatar placeable when flying
        // this has the downside of no collisions.
        // Feel free to reimplement properly with mass enabled.
        var av_transform = placeable.transform;

        // Make a vector where we have moved
        var moveVec = new Vector3df();
        moveVec.x = motion_x * fly_speed_factor;
        moveVec.y = motion_y * fly_speed_factor;
        moveVec.z = -motion_z * fly_speed_factor;

        // Apply that with av looking direction to the current position
        var offsetVec = placeable.GetRelativeVector(moveVec);
        av_transform.pos.x = av_transform.pos.x + offsetVec.x;
        av_transform.pos.y = av_transform.pos.y + offsetVec.y;
        av_transform.pos.z = av_transform.pos.z + offsetVec.z;

        placeable.transform = av_transform;
    }
}

function ServerHandleMove(param) {
    var attrs = me.dynamiccomponent;

    if (attrs.GetAttribute("enableWalk")) {
        if (param == "forward") {
            motion_z = 1;
        }
        if (param == "back") {
            motion_z = -1;
        }
        if (param == "right") {
            motion_x = 1;
        }
        if (param == "left") {
            motion_x = -1;
        }
    }

    if (param == "up") {
        motion_y = 1;
    }
    if (param == "down") {
        motion_y = -1;
    }

    ServerSetAnimationState();
}

function ServerHandleStop(param) {
    if ((param == "forward") && (motion_z == 1)) {
        motion_z = 0;
    }
    if ((param == "back") && (motion_z == -1)) {
        motion_z = 0;
    }
    if ((param == "right") && (motion_x == 1)) {
        motion_x = 0;
    }
    if ((param == "left") && (motion_x == -1)) {
        motion_x = 0;
    }
    if ((param == "up") && (motion_y == 1)) {
        motion_y = 0;
    }
    if ((param == "down") && (motion_y == -1)) {
        motion_y = 0;
    }

    ServerSetAnimationState();
}

function ServerHandleToggleFly() {
    ServerSetFlying(!flying);
}

function ServerSetFlying(newFlying) {
    var attrs = me.dynamiccomponent;
    if (!attrs.GetAttribute("enableFly"))
        newFlying = false;

    if (flying == newFlying)
        return;

    var rigidbody = me.rigidbody;
    flying = newFlying;
    if (flying) {
        rigidbody.mass = 0;
    } else {
        // Reset the x rot if left
        var placeable = me.placeable;
        var av_transform = placeable.transform;
        if (av_transform.rot.x != 0) {
            av_transform.rot.x = 0;
            placeable.transform = av_transform;
        }

        // Set mass back for collisions
        rigidbody.mass = avatar_mass;
        // Push avatar a bit to the fly direction
        // so the motion does not just stop to a wall
        var moveVec = new Vector3df();
        moveVec.x = motion_x * 120;
        moveVec.y = motion_y * 120;
        moveVec.z = -motion_z * 120;
        var pushVec = placeable.GetRelativeVector(moveVec);
        rigidbody.ApplyImpulse(pushVec);
    }
    ServerSetAnimationState();
}

function ServerHandleSetRotation(param) {
    var attrs = me.dynamiccomponent;
    if (attrs.GetAttribute("enableRotate")) {
        var rotateVec = new Vector3df();
        rotateVec.y = parseFloat(param);
        me.rigidbody.SetRotation(rotateVec);
    }
}

function ServerSetAnimationState() {
    // Not flying: Stand, Walk or Crouch
    var animName = standAnimName;
    if ((motion_x != 0) || (motion_z != 0)) {
        animName = walkAnimName;
    }

    // Flying: Fly if moving forward or back, otherwise hover
    if (flying || falling) {
        animName = flyAnimName;
        if (motion_z == 0)
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
        crosshair = new Crosshair(/*bool useLabelInsteadOfCursor*/ true);
        var soundlistener = me.GetOrCreateComponent("EC_SoundListener");
        soundlistener.active = true;

        me.Action("MouseScroll").Triggered.connect(ClientHandleMouseScroll);
        me.Action("Zoom").Triggered.connect(ClientHandleKeyboardZoom);
        me.Action("Rotate").Triggered.connect(ClientHandleRotate);
        me.Action("StopRotate").Triggered.connect(ClientHandleStopRotate);

        // Inspect the login avatar url property
        var avatarurl = client.GetLoginProperty("avatarurl");
        if (avatarurl && avatarurl.length > 0)
        {
            var avatar = me.GetOrCreateComponent("EC_Avatar");
            var r = avatar.appearanceRef;
            r.ref = "local://default_avatar.xml";
            avatar.appearanceRef = r;
            print("Avatar from login parameters enabled:", avatarAssetRef);
        }
    }
    else
    {
        // Make hovering name tag for other clients
        var clientName = me.GetComponent("EC_Name");
        if (clientName != null) {
            // Description holds the actual login name
            if (clientName.description != "") {
                var name_tag = me.GetOrCreateComponent("EC_HoveringText", 2, false);
                if (name_tag != null) {
                    name_tag.SetNetworkSyncEnabled(false);
                    name_tag.SetTemporary(true);
                    name_tag.text = clientName.description;
                    var pos = name_tag.position;
                    pos.y = 1.3;
                    name_tag.position = pos;
                    name_tag.fontSize = 90;
                    var color = new Color(0.2, 0.2, 0.2, 1.0);
                    name_tag.backgroundColor = color;
                    var font_color = new Color(1.0, 1.0, 1.0, 1.0);                
                    name_tag.fontColor = font_color;
                }
            }
        }
    }

    // Hook to tick update to update visual effects (both own and others' avatars)
    frame.Updated.connect(ClientUpdate);
}

function IsCameraActive()
{
    var cameraentity = scene.GetEntityByName("AvatarCamera");
    if (cameraentity == null)
        return false;
    var camera = cameraentity.camera;
    return camera.IsActive();
}

function ClientUpdate(frametime)
{
    // Tie enabled state of inputmapper to the enabled state of avatar camera
    if (own_avatar) {
        var avatarcameraentity = scene.GetEntityByName("AvatarCamera");
        var inputmapper = me.inputmapper;
        if ((avatarcameraentity != null) && (inputmapper != null)) {
            var active = avatarcameraentity.camera.IsActive();
            if (inputmapper.enabled != active) {
                inputmapper.enabled = active;
            }
        }
        ClientUpdateRotation(frametime);
        ClientUpdateAvatarCamera(frametime);
    }

    if (!animsDetected) {
        CommonFindAnimations();
    }
    CommonUpdateAnimation(frametime);
}

function ClientCreateInputMapper() {
    // Create a nonsynced inputmapper
    var inputmapper = me.GetOrCreateComponent("EC_InputMapper", 2, false);
    inputmapper.contextPriority = 101;
    inputmapper.takeMouseEventsOverQt = false;
    inputmapper.takeKeyboardEventsOverQt = false;
    inputmapper.modifiersEnabled = false;
    inputmapper.keyrepeatTrigger = false; // Disable repeat keyevent sending over network, not needed and will flood network
    inputmapper.executionType = 2; // Execute actions on server

    // Key pressed -actions
    inputmapper.RegisterMapping("W", "Move(forward)", 1); // 1 = keypress
    inputmapper.RegisterMapping("S", "Move(back)", 1);
    inputmapper.RegisterMapping("A", "Move(left)", 1);
    inputmapper.RegisterMapping("D", "Move(right))", 1);
    inputmapper.RegisterMapping("Up", "Move(forward)", 1);
    inputmapper.RegisterMapping("Down", "Move(back)", 1);
    inputmapper.RegisterMapping("F", "ToggleFly()", 1);
    inputmapper.RegisterMapping("Space", "Move(up)", 1);
    inputmapper.RegisterMapping("C", "Move(down)", 1);

    // Key released -actions
    inputmapper.RegisterMapping("W", "Stop(forward)", 3); // 3 = keyrelease
    inputmapper.RegisterMapping("S", "Stop(back)", 3);
    inputmapper.RegisterMapping("A", "Stop(left)", 3);
    inputmapper.RegisterMapping("D", "Stop(right)", 3);
    inputmapper.RegisterMapping("Up", "Stop(forward)", 3);
    inputmapper.RegisterMapping("Down", "Stop(back)", 3);
    inputmapper.RegisterMapping("Space", "Stop(up)", 3);
    inputmapper.RegisterMapping("C", "Stop(down)", 3);

    // Connect mouse gestures
    var inputContext = inputmapper.GetInputContext();
    inputContext.GestureStarted.connect(GestureStarted);
    inputContext.GestureUpdated.connect(GestureUpdated);
    inputContext.MouseMove.connect(ClientHandleMouseMove);

    // Local mapper for mouse scroll and rotate
    var inputmapper = me.GetOrCreateComponent("EC_InputMapper", "CameraMapper", 2, false);
    inputmapper.SetNetworkSyncEnabled(false);
    inputmapper.contextPriority = 100;
    inputmapper.takeMouseEventsOverQt = true;
    inputmapper.modifiersEnabled = false;
    inputmapper.executionType = 1; // Execute actions locally
    inputmapper.RegisterMapping("+", "Zoom(in)", 1);
    inputmapper.RegisterMapping("-", "Zoom(out)", 1);
    inputmapper.RegisterMapping("Left", "Rotate(left)", 1);
    inputmapper.RegisterMapping("Right", "Rotate(right))", 1);
    inputmapper.RegisterMapping("Left", "StopRotate(left)", 3);
    inputmapper.RegisterMapping("Right", "StopRotate(right))", 3);
}

function ClientCreateAvatarCamera() 
{
    var cameraentity = scene.GetEntityByName("AvatarCamera");
    if (cameraentity == null)
    {
        cameraentity = scene.CreateEntity(scene.NextFreeIdLocal());
        cameraentity.SetName("AvatarCamera");
        cameraentity.SetTemporary(true);
    }

    var camera = cameraentity.GetOrCreateComponent("EC_Camera");
    var placeable = cameraentity.GetOrCreateComponent("EC_Placeable");

    camera.AutoSetPlaceable();
    camera.SetActive();
    
    var parentRef = placeable.parentRef;
    parentRef.ref = me; // Parent camera to avatar, always
    placeable.parentRef = parentRef;

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

        var attrs = me.dynamiccomponent;
        if (attrs.GetAttribute("enableRotate")) {
            var x = new Number(gestureEvent.Gesture().offset.toPoint().x());
            yaw += x;
            me.Exec(2, "SetRotation", yaw.toString());
        }
        
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
        yaw += delta.x;
        me.Exec(2, "SetRotation", yaw.toString());

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
    var attrs = me.dynamiccomponent;
    // Check that zoom is allowed
    if (!attrs.GetAttribute("enableZoom"))
        return;

    var avatar_camera_distance = attrs.GetAttribute("cameraDistance");

    if (!IsCameraActive())
        return;

    var moveAmount = 0;
    if (relativeScroll < 0 && avatar_camera_distance < 500) {
        if (relativeScroll < -50)
            moveAmount = 2;
        else
            moveAmount = 1;
    } else if (relativeScroll > 0 && avatar_camera_distance > 0) {
        if (relativeScroll > 50)
            moveAmount = -2
        else
            moveAmount = -1;
    }
    if (moveAmount != 0)
    {
        // Add movement
        avatar_camera_distance = avatar_camera_distance + moveAmount;
        // Clamp distance to be between -0.5 and 500
        if (avatar_camera_distance < -0.5)
            avatar_camera_distance = -0.5;
        else if (avatar_camera_distance > 500)
            avatar_camera_distance = 500;

        attrs.SetAttribute("cameraDistance", avatar_camera_distance);
    }
}

function ClientHandleRotate(param) {
    if (param == "left") {
        rotate = -1;
    }
    if (param == "right") {
        rotate = 1;
    }
}

function ClientHandleStopRotate(param) {
    if ((param == "left") && (rotate == -1)) {
        rotate = 0;
    }
    if ((param == "right") && (rotate == 1)) {
        rotate = 0;
    }
}

function ClientUpdateRotation(frametime) {
    var attrs = me.dynamiccomponent;
    // Check that rotation is allowed
    if (!attrs.GetAttribute("enableRotate"))
        return;

    if (rotate != 0) {
        yaw -= rotate_speed * rotate * frametime;
        me.Exec(2, "SetRotation", yaw.toString());
    }
}

function ClientUpdateAvatarCamera() {

    // Check 1st/3rd person mode toggle
    ClientCheckState();

    var attrs = me.dynamiccomponent;
    avatar_camera_distance = attrs.GetAttribute("cameraDistance");
    var first_person = avatar_camera_distance < 0;

    var cameraentity = scene.GetEntityByName("AvatarCamera");
    if (cameraentity == null)
        return;
    var cameraplaceable = cameraentity.placeable;

    if (!first_person)
        pitch = 0;
    var cameratransform = cameraplaceable.transform;
    cameratransform.rot = new Vector3df(pitch, 0, 0);
    cameratransform.pos = new Vector3df(0, avatar_camera_height, avatar_camera_distance);

    // Track the head bone in 1st person
    if ((first_person) && (me.mesh != null))
    {
        me.mesh.ForceSkeletonUpdate();
        var headPos = me.mesh.GetBoneDerivedPosition("Bip01_Head");
        headPos.z -= 0.5;
        headPos.y -= 0.7;
        cameratransform.pos = headPos;
    }

    cameraplaceable.transform = cameratransform;
}

function ClientCheckState()
{
    var attrs = me.dynamiccomponent;
    var first_person = attrs.GetAttribute("cameraDistance") < 0;

    var cameraentity = scene.GetEntityByName("AvatarCamera");
    var avatar_placeable = me.GetComponent("EC_Placeable");

    if (crosshair == null)
        return;
    // If ent got destroyed or something fatal, return cursor
    if (cameraentity == null || avatar_placeable == null) {
        if (crosshair.isActive()) {
            crosshair.hide();
        }
        return;
    }

    if (!first_person) {
        if (crosshair.isActive()) {
            crosshair.hide();
        }
        return;
    }
    else
    {
        // We might be in 1st person mode but camera might not be active
        // hide curson and show av
        if (!cameraentity.camera.IsActive()) {
            if (crosshair.isActive()) {
                crosshair.hide();
            }
        }
        else {
            // 1st person mode and camera is active
            // show curson and av
            if (!crosshair.isActive()) {
                crosshair.show();
            }
        }
    }
}

function ClientHandleMouseMove(mouseevent)
{
    var attrs = me.dynamiccomponent;
    var first_person = attrs.GetAttribute("cameraDistance") < 0;

    if ((first_person) && (input.IsMouseCursorVisible()))
    {
        input.SetMouseCursorVisible(false);
        if (!crosshair.isUsingLabel)
            QApplication.setOverrideCursor(crosshair.cursor);
    }
        
    // Do not rotate if not allowed
    if (!attrs.GetAttribute("enableRotate"))
        return;

    // Do not rotate in third person if right mousebutton not held down
    if ((!first_person) && (input.IsMouseCursorVisible()))
        return;

    var cameraentity = scene.GetEntityByName("AvatarCamera");
    if (cameraentity == null)
        return;

    // Dont move av rotation if we are not the active cam
    if (!cameraentity.camera.IsActive())
        return;

    if (mouseevent.relativeX != 0)
    {
        // Rotate avatar or camera
        yaw -= mouse_rotate_sensitivity * parseInt(mouseevent.relativeX);
        me.Exec(2, "SetRotation", yaw.toString());
    }

    if (mouseevent.relativeY != 0)
    {
        // Look up/down
        var attrs = me.dynamiccomponent;
        pitch -= mouse_rotate_sensitivity * parseInt(mouseevent.relativeY);

        // Dont let the 1st person flip vertically, 180 deg view angle
        if (pitch < -90)
            pitch = -90;
        if (pitch > 90)
            pitch = 90;
    }
}

function CommonFindAnimations() {
    var animcontrol = me.animationcontroller;
    var availableAnimations = animcontrol.GetAvailableAnimations();
    if (availableAnimations.length > 0) {
        // Detect animation names
        for(var i=0; i<animList.length; i++) {
            var animName = animList[i];
            if (availableAnimations.indexOf(animName) == -1) {
                // Disable this animation by setting it to a empty string
                print("Could not find animation for:", animName, " - disabling animation");
                animList[i] = "";
            }
        }

        // Assign the possible empty strings for
        // not found anims back to the variables
        standAnimName = animList[0];
        walkAnimName = animList[1];
        flyAnimName = animList[2];
        hoverAnimName = animList[3];

        animsDetected = true;
    }
}

function CommonUpdateAnimation(frametime) {
    // This function controls the known move animations, such as walk, fly, hover and stand,
    // which are replicated through the animationState attribute of the AnimationController.
    // Only one such move animation can be active at a time.
    // Other animations, such as for gestures, can be freely enabled/disabled by other scripts.

    var attrs = me.dynamiccomponent;

    if (!animsDetected) {
        return;
    }

    var animcontroller = me.animationcontroller;
    var rigidbody = me.rigidbody;
    if ((animcontroller == null) || (rigidbody == null)) {
        return;
    }

    if (!attrs.GetAttribute("enableAnimation"))
    {
        // When animations disabled, forcibly disable all running move animations
        // Todo: what if custom scripts want to run the move anims as well?
        for (var i = 0; i < animList.length; ++i) {
            if (animList[i] != "")
                animcontroller.DisableAnimation(animList[i], 0.25);
        }
        return;
    }

    var animName = animcontroller.animationState;

    // Enable animation, skip with headless server
    if (animName != "" && !framework.IsHeadless()) {
        // Do custom speeds for certain anims
        if (animName == hoverAnimName) {
            animcontroller.SetAnimationSpeed(animName, 0.25);
        }
        // Enable animation
        if (!animcontroller.IsAnimationActive(animName)) {
            animcontroller.EnableAnimation(animName, true, 0.25, false);
        }
        // Disable other move animations
        for (var i = 0; i < animList.length; ++i) {
            if ((animList[i] != animName) && (animList[i] != "") && (animcontroller.IsAnimationActive(animList[i])))
                animcontroller.DisableAnimation(animList[i], 0.25);
        }
    }

    // If walk animation is playing, adjust its speed according to the avatar rigidbody velocity
    if (animName != ""  && animcontroller.IsAnimationActive(walkAnimName)) {
        var velocity = rigidbody.linearVelocity;
        var walkspeed = Math.sqrt(velocity.x * velocity.x + velocity.z * velocity.z) * walk_anim_speed;
        animcontroller.SetAnimationSpeed(walkAnimName, walkspeed);
    }
}
