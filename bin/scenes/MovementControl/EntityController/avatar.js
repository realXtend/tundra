/**
 * @name EntityController/avatar.js
 * @fileOverview A simple walking avatar with physics & third person camera 
 * <p><b>Usage: </b>Add as an EC_Script to an entity to make it an avatar.
 */

engine.IncludeFile("local://RigidBodyController.js");
engine.IncludeFile("local://AvatarAnimationSync.js");
engine.IncludeFile("local://AvatarCameraSync.js");

/**
 * @namespace main namespace.
 */
var realXtend = realXtend || {};

/**
 * @namespace EntityController namespace contains functionality to control entities position and rotation. 
 */
realXtend.EntityController = realXtend.EntityController || {};

/**
 * @class A simple walking avatar with physics & third person camera 
 */
realXtend.EntityController.SimpleAvatar = function()
{
	this.motion = new Vector3df();
	this.rotate = 0;
	this.mouse_rotate_sensitivity = 0.3;
	this.avatar_mass = 10;
	//this.moveVec = new Vector3df();
	this.rotationVec = new Vector3df();
	this.rotationVec.z = 1;
	
	
	// Needed bools for logic
	this.isserver = server.IsRunning();
	this.own_avatar = false;
	this.fish_created = false;
	
	// Gestures
	this.listenGesture = false;
	
	//Create Entity Controller
//	engine.IncludeFile("local://RigidBodyController.js");
	this.avatar_rigid_body = new realXtend.EntityController.RigidBodyController();  
	
	//Create Animation sync
	this.avatar_animation_sync = new realXtend.EntityController.AvatarAnimationSync();
	this.avatar_rigid_body.AddSyncObject(this.avatar_animation_sync);
	
	// Create avatar on server, and camera & inputmapper on client
	if (this.isserver) {
	    this.ServerInitialize();
	} else {
	    this.ClientInitialize();
	}

};

//SERVER METHODS

realXtend.EntityController.SimpleAvatar.prototype.ServerInitialize = function () {	
    var avatar = me.GetOrCreateComponentRaw("EC_Avatar");
    var voice = me.GetOrCreateComponentRaw("EC_TtsVoice");
       
    var rigidbody = me.rigidbody; 

    // Set the avatar appearance. This creates the mesh & animationcontroller, once the avatar asset has loaded
    // Note: for now, you need the default_avatar.xml in your bin/data/assets folder
    // Set the avatar appearance. This creates the mesh & animationcontroller, once the avatar asset has loaded
    var r = avatar.appearanceRef;
    r.ref = "local://default_avatar.xml";
    avatar.appearanceRef = r;

    // Set voice to avatar
    voice.voice = "Spanish male";

    // Set physics properties
    var sizeVec = new Vector3df();
    sizeVec.z = 2.4;
    sizeVec.x = 0.5;
    sizeVec.y = 0.5;
	rigidbody.physicsType = 2; //Dynamic
    rigidbody.mass = this.avatar_mass;
    rigidbody.shapeType = 3; // Capsule
    rigidbody.size = sizeVec;
    rigidbody.friction = 0.3;

    var angularVec = new Vector3df();
    angularVec.z = 1;
    rigidbody.angularFactor = angularVec; // Set zero angular factor so that body stays upright
    
    var avatar_properties = me.dynamiccomponent;
    if (avatar_properties)
    {
    	avatar_properties.SetAttribute("rotationMaskX",this.rotationVec.x);
    	avatar_properties.SetAttribute("rotationMaskY",this.rotationVec.y);
    }    
};



//CLIENT METHODS

realXtend.EntityController.SimpleAvatar.prototype.ClientInitialize = function() {
    // Check if this is our own avatar
    // Note: bad security. For now there's no checking who is allowed to invoke actions
    // on an entity, and we could theoretically control anyone's avatar
    if (me.GetName() == "Avatar" + client.GetConnectionID()) {
        this.own_avatar = true;
        this.ClientCreateInputMapper();

        //Create Camera sync
        this.client_avatar_camera_sync = new realXtend.EntityController.AvatarCameraSync(7.0, 1.0);
        this.avatar_rigid_body.AddSyncObject(this.client_avatar_camera_sync);
        
        //connect actions
        me.Action("KeyMove").Triggered.connect(this, this.ClientKeyMove);
        me.Action("KeyRotate").Triggered.connect(this, this.ClientKeyRotate);
        me.Action("KeyStop").Triggered.connect(this, this.ClientKeyStop);
        me.Action("KeyStopRotate").Triggered.connect(this, this.ClientKeyStopRotate);
        
        me.Action("MouseLookX").Triggered.connect(this, this.ClientHandleLookX);        
        me.Action("CheckState").Triggered.connect(this, this.ClientHandleCheckState);
        var soundlistener = me.GetOrCreateComponentRaw("EC_SoundListener");
        soundlistener.active = true;

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
};

realXtend.EntityController.SimpleAvatar.prototype.ClientHandleLookX = function(param)
{
    if (this.client_avatar_camera_sync)
    {
    	if (this.client_avatar_camera_sync.IsTripod())
    		this.client_avatar_camera_sync.RotateZ(param);
    	else
    	{
    		var move=parseInt(param);
    		
    		move = -move * this.mouse_rotate_sensitivity;    		
    		//exec rotate action on client
    		me.Exec(1, "Rotate", ["0","0",move.toString(),"true"]);	    		
    	}
    }
    	
};

//check camera state
realXtend.EntityController.SimpleAvatar.prototype.ClientHandleCheckState = function()
{
	var inputmapper = me.inputmapper;
	if ((this.client_avatar_camera_sync != null) && (inputmapper != null)) {
        var active = this.client_avatar_camera_sync.IsCameraActive();
        if (inputmapper.enabled != active)
            inputmapper.enabled = active;
    }
};

realXtend.EntityController.SimpleAvatar.prototype.ClientCreateInputMapper = function() {
    // Create a nonsynced inputmapper
    var inputmapper = me.GetOrCreateComponentRaw("EC_InputMapper", 2, false);
    inputmapper.contextPriority = 101;
    inputmapper.takeMouseEventsOverQt = false;
    inputmapper.takeKeyboardEventsOverQt = false;
    inputmapper.modifiersEnabled = false;
    inputmapper.keyrepeatTrigger = false; // Disable repeat keyevent sending over network, not needed and will flood network
    inputmapper.executionType = 1; // Execute actions on client
    inputmapper.RegisterMapping("W", "KeyMove(forward)", 1);
    inputmapper.RegisterMapping("S", "KeyMove(back)", 1);
    inputmapper.RegisterMapping("A", "KeyMove(left)", 1);
    inputmapper.RegisterMapping("D", "KeyMove(right))", 1);
    inputmapper.RegisterMapping("Up", "KeyMove(forward)", 1);
    inputmapper.RegisterMapping("Down", "KeyMove(back)", 1);
    inputmapper.RegisterMapping("Left", "KeyRotate(left)", 1);
    inputmapper.RegisterMapping("Right", "KeyRotate(right))", 1);
    inputmapper.RegisterMapping("F", "ToggleFly()", 1);
    inputmapper.RegisterMapping("Q", "Gesture(wave)", 1);
    inputmapper.RegisterMapping("Space", "KeyMove(up)", 1);
    inputmapper.RegisterMapping("C", "KeyMove(down)", 1);
    inputmapper.RegisterMapping("W", "KeyStop(forward)", 3);
    inputmapper.RegisterMapping("S", "KeyStop(back)", 3);
    inputmapper.RegisterMapping("A", "KeyStop(left)", 3);
    inputmapper.RegisterMapping("D", "KeyStop(right)", 3);
    inputmapper.RegisterMapping("Up", "KeyStop(forward)", 3);
    inputmapper.RegisterMapping("Down", "KeyStop(back)", 3);
    inputmapper.RegisterMapping("Left", "KeyStopRotate(left)", 3);
    inputmapper.RegisterMapping("Right", "KeyStopRotate(right))", 3);
    inputmapper.RegisterMapping("Space", "KeyStop(up)", 3);
    inputmapper.RegisterMapping("C", "KeyStop(down)", 3);
    inputmapper.RegisterMapping("R", "ToggleRun()", 1);
    

    // Connect gestures
    var inputContext = inputmapper.GetInputContext();
    inputContext.GestureStarted.connect(this, this.GestureStarted);
    inputContext.GestureUpdated.connect(this, this.GestureUpdated);          
    
    // mouse click to raycast point
    inputContext.MouseLeftPressed.connect(this, this.ClientHandleMouseLeftPressed);
};

realXtend.EntityController.SimpleAvatar.prototype.ClientHandleMouseLeftPressed = function(mouseevent)
{
	//check Ctrl Modifier
	if (mouseevent.HasCtrlModifier())
	{
		var raycastResult = renderer.Raycast(mouseevent.x, mouseevent.y);
	    if (raycastResult.entity !== null)
	    {
	    	me.Exec(1, "GoTo", [raycastResult.pos.x.toString(),raycastResult.pos.y.toString(),raycastResult.pos.z.toString(), "true"]);
	    	//me.Exec(1, "LookAt", raycastResult.pos.x.toString(),raycastResult.pos.y.toString(),raycastResult.pos.z.toString());
	    }
	}
};

realXtend.EntityController.SimpleAvatar.prototype.ClientKeyMove = function(param)
{
	if (param == "forward")
		this.motion.x = 1;
	else if (param == "back")
		this.motion.x = -1;
	else if (param == "left")
		this.motion.y = 1;
	else if (param == "right")
		this.motion.y = -1;
	else if (param == "up") 
		this.motion.z = 1;
	else if (param == "down")
		this.motion.z = -1;
	
	var move = this.motion.normalize();
	me.Exec(1, "Move", move.x, move.y, move.z);
};

realXtend.EntityController.SimpleAvatar.prototype.ClientKeyRotate = function(param)
{	
	if (param == "left")
		this.rotate = 1;
	if (param == "right") 
		this.rotate = -1;	
	me.Exec(1, "Rotate", 0, 0, this.rotate);
};

realXtend.EntityController.SimpleAvatar.prototype.ClientKeyStop = function(param)
{	
	if ((param == "forward") && (this.motion.x > 0))
		this.motion.x = 0;
	else if ((param == "back") && (this.motion.x < 0))
		this.motion.x = 0;
	else if ((param == "left") && (this.motion.y > 0))
		this.motion.y = 0;
	else if ((param == "right") && (this.motion.y < 0))
		this.motion.y = 0;
	else if ((param == "up") && (this.motion.z > 0))
		this.motion.z = 0;
	else if ((param == "down") && (this.motion.z < 0))
		this.motion.z = 0;
	
	var move = this.motion.normalize();
	me.Exec(1, "Move", move.x, move.y, move.z);	
};

realXtend.EntityController.SimpleAvatar.prototype.ClientKeyStopRotate = function(param)
{	
	if ((param == "left") && (this.rotate > 0)) 
		this.rotate = 0;
	if ((param == "right") && (this.rotate < 0))
		this.rotate = 0;
	me.Exec(1, "Rotate", 0, 0, this.rotate);
};

realXtend.EntityController.SimpleAvatar.prototype.GestureStarted = function(gestureEvent)
{
	if (this.client_avatar_camera_sync)
		if (!this.client_avatar_camera_sync.IsCameraActive())
			return;
	 
    if (gestureEvent.GestureType() == Qt.PanGesture)
    {
        listenGesture = true;
        var x = new Number(gestureEvent.Gesture().offset.toPoint().x()); 
        // move on server z 
        me.Exec(1, "Rotate", 0, 0, x.toString());
        gestureEvent.Accept();
    }
    else if (gestureEvent.GestureType() == Qt.PinchGesture)
        gestureEvent.Accept();
};

realXtend.EntityController.SimpleAvatar.prototype.GestureUpdated = function(gestureEvent)
{
	if (this.client_avatar_camera_sync)
		if (!this.client_avatar_camera_sync.IsCameraActive())
			return;

    if (gestureEvent.GestureType() == Qt.PanGesture && listenGesture == true)
    {
        // Rotate avatar with X pan gesture
        delta = gestureEvent.Gesture().delta.toPoint();
        var x = new Number(delta.x());
        me.Exec(1, "Rotate", 0, 0, x.toString());

        // Start walking or stop if total Y len of pan gesture is 100
        var walking = false;
        if (me.animationcontroller.animationState == walkAnimName)
            walking = true;
        var totalOffset = gestureEvent.Gesture().offset.toPoint();
        if (totalOffset.y() < -100)
        {
            if (walking) {
                me.Exec(2, "Move", 0,0,0);
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
};

realXtend.EntityController.SimpleAvatar.prototype.CreateFish = function() {
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
            this.fish_created = true;
            var t = fishmesh.nodeTransformation;
            var scaleVec = new Vector3df();
            scaleVec.x = 0.1;
            scaleVec.y = 0.1;
            scaleVec.z = 0.1;
            t.scale = scaleVec;
            fishmesh.nodeTransformation = t;
        }
    }
};

new realXtend.EntityController.SimpleAvatar();