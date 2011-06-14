
/**
 * @namespace main namespace.
 */
var realXtend = realXtend || {};

/**
 * @namespace EntityController namespace contains functionality to control entities position and rotation. 
 */
realXtend.EntityController = realXtend.EntityController || {};

//Control for realXtend.EntityController.AvatarCameraSync on Client side
realXtend.EntityController.AvatarCameraSync = function(cdistance, cheight)
{
	var isserver = server.IsRunning();
	if (isserver) 
		return 0;
	
	// distance = 7
	// height = 1
	this.avatar_camera_distance = cdistance;
	this.avatar_camera_height = cheight;
	this.tripod = false;
	this.CreateInputMapper();
	this.CreateAvatarCamera();
};

realXtend.EntityController.AvatarCameraSync.prototype.ToggleCameraTripod = function()
{
	if (this.tripod == false)
		this.tripod = true;
	else
		this.tripod = false;
};

realXtend.EntityController.AvatarCameraSync.prototype.IsTripod = function()
{
	return (this.tripod == true);
};

realXtend.EntityController.AvatarCameraSync.prototype.CreateAvatarCamera = function()
{
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
    
    //connect actions
    me.Action("MouseScroll").Triggered.connect(this, this.SetCameraDistance);
    me.Action("Zoom").Triggered.connect(this, this.HandleKeyboardZoom);
    me.Action("ToggleTripod").Triggered.connect(this, this.ToggleCameraTripod);
    me.Action("MouseLookY").Triggered.connect(this, this.HandleLookY);
    
    // Set initial position
    this.Update(0);
};

realXtend.EntityController.AvatarCameraSync.prototype.CreateInputMapper = function()
{
	//Local camera matter for mouse scroll
	var inputmapper = me.GetOrCreateComponentRaw("EC_InputMapper", "CameraMapper", 2, false);
	inputmapper.SetNetworkSyncEnabled(false);
	inputmapper.contextPriority = 100;
	inputmapper.takeMouseEventsOverQt = true;
	inputmapper.modifiersEnabled = false;
	inputmapper.executionType = 1; // Execute actions locally
	inputmapper.RegisterMapping("T", "ToggleTripod", 1);
	inputmapper.RegisterMapping("+", "Zoom(in)", 1);
	inputmapper.RegisterMapping("-", "Zoom(out)", 1);
	
};

realXtend.EntityController.AvatarCameraSync.prototype.IsCameraActive = function()
{
    var cameraentity = scene.GetEntityByNameRaw("AvatarCamera");
    if (cameraentity == null)
        return false;
    var camera = cameraentity.ogrecamera;
    return camera.IsActive();
};


realXtend.EntityController.AvatarCameraSync.prototype.RotateX = function(param)
{
    if (this.tripod)
    {
        var cameraentity = scene.GetEntityByNameRaw("AvatarCamera");
        if (cameraentity == null)
            return;
        var cameraplaceable = cameraentity.placeable;
        var cameratransform = cameraplaceable.transform;

        var move = parseInt(param);
        cameratransform.rot.x -= move;
        cameraplaceable.transform = cameratransform;
    }
};

realXtend.EntityController.AvatarCameraSync.prototype.RotateY = function(param)
{
    if (this.tripod)
    {
        var cameraentity = scene.GetEntityByNameRaw("AvatarCamera");
        if (cameraentity == null)
            return;
        var cameraplaceable = cameraentity.placeable;
        var cameratransform = cameraplaceable.transform;

        var move = parseInt(param);
        cameratransform.rot.y -= move;
        cameraplaceable.transform = cameratransform;
    }
};

realXtend.EntityController.AvatarCameraSync.prototype.RotateZ = function(param)
{
    if (this.tripod)
    {
        var cameraentity = scene.GetEntityByNameRaw("AvatarCamera");
        if (cameraentity == null)
            return;
        var cameraplaceable = cameraentity.placeable;
        var cameratransform = cameraplaceable.transform;

        var move = parseInt(param);
        cameratransform.rot.z -= move;
        cameraplaceable.transform = cameratransform;
    }
};

realXtend.EntityController.AvatarCameraSync.prototype.SetCameraDistance = function(distance)
{
    if (!this.IsCameraActive())
        return;
    var moveAmount = 0;
    if (distance < 0 && this.avatar_camera_distance < 500) {
        if (distance < -50)
            moveAmount = 2;
        else
            moveAmount = 1;
    } else if (distance > 0 && this.avatar_camera_distance > 1) {
        if (distance > 50)
            moveAmount = -2;
        else
            moveAmount = -1;
    }
    if (moveAmount != 0)
    {
        // Add movement
        this.avatar_camera_distance += moveAmount;
        // Clamp distance  to be between 1 and 500
        if (this.avatar_camera_distance < 1)
            this.avatar_camera_distance = 1;
        else if (this.avatar_camera_distance > 500)
            this.avatar_camera_distance = 500;
    }
};

realXtend.EntityController.AvatarCameraSync.prototype.Update = function(frametime, params)
{	
    if (!this.tripod)
    {
        var cameraentity = scene.GetEntityByNameRaw("AvatarCamera");
        if (cameraentity == null)
            return;
        var cameraplaceable = cameraentity.placeable;
        var avatarplaceable = me.placeable;

        var cameratransform = cameraplaceable.transform;
        var avatartransform = avatarplaceable.transform;
        var offsetVec = new Vector3df();
        offsetVec.x = -this.avatar_camera_distance;
        offsetVec.z = this.avatar_camera_height;
        offsetVec = avatarplaceable.GetRelativeVector(offsetVec);
        cameratransform.pos.x = avatartransform.pos.x + offsetVec.x;
        cameratransform.pos.y = avatartransform.pos.y + offsetVec.y;
        cameratransform.pos.z = avatartransform.pos.z + offsetVec.z;
        // Note: this is not nice how we have to fudge the camera rotation to get it to show the right things
        cameratransform.rot.x = 90;
        cameratransform.rot.z = avatartransform.rot.z - 90;

        cameraplaceable.transform = cameratransform;
    }
};


realXtend.EntityController.AvatarCameraSync.prototype.HandleKeyboardZoom = function(direction) {
    if (direction == "in") {
    	this.SetCameraDistance(10);
    } else if (direction == "out") {
    	this.SetCameraDistance(-10);
    }
};

realXtend.EntityController.AvatarCameraSync.prototype.HandleLookY = function(param)
{	
    if (this.IsTripod())
    	this.RotateX(param);
};