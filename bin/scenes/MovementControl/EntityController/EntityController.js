/**
 * @name EntityController/EntityController.js
 * @fileOverview EntityController.js contains implementation of EntityController class, which should
 * be specialized for static and dynamic physic objects.
 * <p><b>Usage: </b>Cannot be directly used on an entity.
 */

/**
 * @namespace main namespace.
 */
var realXtend = realXtend || {};

/**
 * @namespace EntityController namespace contains functionality to control entities position and rotation. 
 */
realXtend.EntityController = realXtend.EntityController || {};

/**
 * Creates an EntityController object.
 * 
 * @class Controller of Entity
 */
realXtend.EntityController.EntityController = function()
{    
    this.syncProperties = 0;
    
    //default attributes
	this.default_move_vel = 3.0;
	this.default_rot_vel = 90.0;
	this.default_max_vel_factor = 3.0; //3x move velocity
	this.default_max_rot_factor = 3.0; //3x rotate velocity 
	this.default_rot_mask = new Vector3df();
	this.default_rot_mask.x = 1;
	this.default_rot_mask.y = 1;
	this.default_rot_mask.z = 1;
	this.default_lookAtDirection = new Vector3df();
	this.default_lookAtDirection.x = 1;
	this.default_lookAtDirection.y = 0;
	this.default_lookAtDirection.z = 0;
    
	this.goToThreshold = 0.1;
	this.lookAtThreshold = 2;
    
    //Shared properties with inherited objects defined as private.
	var move = new Vector3df();
	this.getMove = function() {return move;};
	this.setMove = function(val) {move = val;};	  
    
    var rotate = new Vector3df();
    this.getRotate = function() {return rotate;};
    this.setRotate = function(val) {rotate = val;};
    
    var goTo = 0;
    /**
     * Gets goTo position.
     * @returns {Vector3df}
     */
    this.getGoTo = function() {return goTo;};
    /**
     * Sets goTo position. Set it to 0 to disable.
     * @param {Vector3df|Number} val
     */
    this.setGoTo = function(val) {goTo = val;};
    
    var goToLooking = false;
    this.getGoToLooking = function() {return goToLooking;};
    this.setGoToLooking = function(val) {goToLooking = val;};

    var lookAt = 0;
    /**
     * Gets lookAt position.
     * @returns {Vector3df}
     */
    this.getLookAt = function() {return lookAt;};
    
    /**
     * Sets lookAt position. Set it to 0 to disable. 
     * @param {Vector3df|Number} val
     */
    this.setLookAt = function(val) {lookAt = val;};
    
    var rotateTo = 0;
    this.getRotateTo = function() {return rotateTo;};
    this.setRotateTo = function(val) {rotateTo = val;};
    
    var auto_stop_rotate = false;
    this.getRotateAutoStop = function() {return auto_stop_rotate;};
    this.setRotateAutoStop = function(val) {auto_stop_rotate = val;};

    //Sync objects
    /** @private */this.syncObjectList = new Array();
    this.AddSyncObject = function(object) {this.syncObjectList.push(object);};
    
    //Initialize
	var isserver = server.IsRunning();
	if (isserver) {
	    this.ServerInitialize();
	} else {
		this.ClientInitialize();
	}
	
};
/**
 * Uninitializes EntityController object.
 * Call it before object removal.
 * @example
 * var controller = new realXtend.EntityController.EntityController();
 * controller.Destroy();
 * delete controller;
 */
realXtend.EntityController.EntityController.prototype.Destroy = function()
{
	if (server.IsRunning())
		this.ServerUninitialize();
	else
		this.ClientUninitialize();
};

//COMMON METHODS
/**
 * Synchronize objects
 * 
 * @param {Number} frametime Elapsed time.
 * @param {Array} [params] List of extra params.
 */
realXtend.EntityController.EntityController.prototype.SyncObjects = function(frametime, params)
{
	//update sync  
	var num_syncs = this.syncObjectList.length;
	for (var i = 0; i < num_syncs; i++)
	{
		this.syncObjectList[i].Update(frametime, params);
	}
};
/**
 * Handles Move action
 * @param {Number} x
 * @param {Number} y
 * @param {Number} z
 */
realXtend.EntityController.EntityController.prototype.HandleMove = function(x, y ,z)
{
	var maxMoveFactor = this.default_max_vel_factor;
		
	if (this.syncProperties)
		maxMoveFactor = this.syncProperties.GetAttribute("maxVelocityFactor");
	
	var tmpVec = this.getMove();
	
	if (x > maxMoveFactor)
		tmpVec.x = parseFloat(maxMoveFactor);
	else if (x < -maxMoveFactor)
		tmpVec.x = parseFloat(-maxMoveFactor);
	else
		tmpVec.x = parseFloat(x);
	
	if (y > maxMoveFactor)
		tmpVec.y = parseFloat(maxMoveFactor);
	else if (y < -maxMoveFactor)
		tmpVec.y = parseFloat(-maxMoveFactor);
	else
		tmpVec.y = parseFloat(y);

	if (z > maxMoveFactor)
		tmpVec.z = parseFloat(maxMoveFactor);
	else if (z < -maxMoveFactor)
		tmpVec.z = parseFloat(-maxMoveFactor);
	else
		tmpVec.z = parseFloat(z);
	
	if (tmpVec.x == 0 && tmpVec.y == 0 && tmpVec.z == 0)
		this.StopMove(false);
	
	
	if (!server.IsRunning())
		//exec move action on server
		me.Exec(2, "Move", tmpVec.x, tmpVec.y, tmpVec.z);	
};

realXtend.EntityController.EntityController.prototype.StopMove = function(arrived)
{
	if (this.getGoTo() != 0)
	{
		/**
		 * [Signal] GoToFinished client/server action
		 * @name realXtend.EntityController.EntityController#GoToFinished
		 * @event
		 * @param {bool} arrived
		 */
		me.Exec(1, "GoToFinished", arrived);
		this.setGoTo(0);
		this.setGoToLooking(false);
	}
};

/**
 * Handles Rotate action
 * @param {Number} x
 * @param {Number} y
 * @param {Number} z
 * @param {Array} params params[0] {Boolean} autostop
 */
realXtend.EntityController.EntityController.prototype.HandleRotate = function(x, y , z, params)
{
	var maxRotFactor = this.default_max_rot_factor;
	if (this.syncProperties)
		maxRotFactor = this.syncProperties.GetAttribute("maxRotationFactor");
	var tmpVec = this.getRotate();	
	if (x > maxRotFactor)
		tmpVec.x = parseFloat(maxRotFactor);
	else if (x < -maxRotFactor)
		tmpVec.x = parseFloat(-maxRotFactor);
	else
		tmpVec.x = parseFloat(x);

	if (y > maxRotFactor)
		tmpVec.y = parseFloat(maxRotFactor);
	else if (y < -maxRotFactor)
		tmpVec.y = parseFloat(-maxRotFactor);
	else
		tmpVec.y = parseFloat(y);

	if (z > maxRotFactor)
		tmpVec.z = parseFloat(maxRotFactor);
	else if (z < -maxRotFactor)
		tmpVec.z = parseFloat(-maxRotFactor);
	else
		tmpVec.z = parseFloat(z);
	
	//params[0]: autostop: boolean
	if (params[0] == "true")
		this.setRotateAutoStop(true);
	
	if (tmpVec.x == 0 && tmpVec.y == 0 && tmpVec.z == 0)
	{
		this.StopRotate(false);
	}
	
	if (!server.IsRunning())
	{	
		//exec rotate action on server				
		me.Exec(2, "Rotate", [tmpVec.x.toString(), tmpVec.y.toString(), tmpVec.z.toString(), this.getRotateAutoStop().toString()]);
	}
};

realXtend.EntityController.EntityController.prototype.StopRotate = function(arrived)
{
	if ((this.getLookAt() != 0) || (this.getRotateTo() != 0))
	{
		if (!this.getGoToLooking())
			me.Exec(1, "RotationFinished", arrived);
		this.setLookAt(0);
		this.setRotateTo(0);
	}
};

/**
 * Handles GoTo action
 * @param {Number} x
 * @param {Number} y
 * @param {Number} z
 * @param {Array} params params[0] {Boolean} looking target point
 * @return
 */
realXtend.EntityController.EntityController.prototype.HandleGoTo = function(x, y, z, params)
{
	var temp = new Vector3df();
	temp.x = parseFloat(x);
	temp.y = parseFloat(y);
	temp.z = parseFloat(z);
	this.setGoTo(temp);
	
	//params[0]: looking target point (boolean)
	var goTolooking = params[0];
	if (goTolooking == "true")
		this.setGoToLooking(true);
	else
		this.setGoToLooking(false);
	
	if (!server.IsRunning())
		//exec rotate action on server
		me.Exec(2, "GoTo", [x, y, z, params[0]]);	
};

/**
 * Handles LookAt action
 * @param {Number} x
 * @param {Number} y
 * @param {Number} z
 */
realXtend.EntityController.EntityController.prototype.HandleLookAt = function(x, y, z)
{
	var temp = new Vector3df();
	temp.x = parseFloat(x);
	temp.y = parseFloat(y);
	temp.z = parseFloat(z);
	this.setLookAt(temp);
	
	if (!server.IsRunning())
		//exec rotate action on server
		me.Exec(2, "LookAt", x, y, z);	
};

/**
 * Handles RotateTo action
 * @param {Number} x
 * @param {Number} y
 * @param {Number} z
 */
realXtend.EntityController.EntityController.prototype.HandleRotateTo = function(x, y, z)
{	
	var temp = new Vector3df();
	temp.x = parseFloat(x);
	temp.y = parseFloat(y);
	temp.z = parseFloat(z);
	temp = this.NormalizeRotation(temp);
	this.setRotateTo(temp);
	
	if (!server.IsRunning())
		//exec rotate action on server
		me.Exec(2, "RotateTo", x, y, z);	
};

//convert rotation values to [-180,180]
realXtend.EntityController.EntityController.prototype.NormalizeRotation = function(rot)
{
	var normalized = false;
	if (rot.x < -180)
	{
		normalized = true;
		rot.x += 360;
	}
	else if (rot.x > 180)
	{
		normalized = true;
		rot.x -= 360;
	}
	if (rot.y < -180)
	{
		normalized = true;
		rot.y += 360;
	}
	else if (rot.y > 180)
	{
		normalized = true;
		rot.y -= 360;
	}
	if (rot.z < -180)
	{
		normalized = true;
		rot.z += 360;
	}
	else if (rot.z > 180)
	{
		normalized = true;
		rot.z -= 360;
	}
	if (normalized == true)
		rot = this.NormalizeRotation(rot);
	return rot;
};

realXtend.EntityController.EntityController.prototype.ResetAutoUpdatedVariables = function()
{
	this.StopMove(false);
	this.StopRotate(false);
};

//CLIENT METHODS
/**
 * Initializes client object.
 */
realXtend.EntityController.EntityController.prototype.ClientInitialize = function()
{
	// Connect actions
	/**
	 * [Action] Move client/server action
	 * @name realXtend.EntityController.EntityController#Move
	 * @event
	 * @param {Number} x
	 * @param {Number} y
	 * @param {Number} z
	 * @see realXtend.EntityController.EntityController#HandleMove
	 */
	me.Action("Move").Triggered.connect(this, this.HandleMove);
	/**
	 * [Action] Rotate client/server action
	 * @name realXtend.EntityController.EntityController#Rotate
	 * @event
	 */
	me.Action("Rotate").Triggered.connect(this, this.HandleRotate);
	/**
	 * [Action] GoTo client/server action
	 * @name realXtend.EntityController.EntityController#GoTo
	 * @event
	 */
	me.Action("GoTo").Triggered.connect(this, this.HandleGoTo);
	/**
	 * [Action] LookAt client/server action
	 * @name realXtend.EntityController.EntityController#LookAt
	 * @event
	 */
	me.Action("LookAt").Triggered.connect(this, this.HandleLookAt);
	/**
	 * [Action] RotateTo client/server action
	 * @name realXtend.EntityController.EntityController#RotateTo
	 * @event
	 */
	me.Action("RotateTo").Triggered.connect(this, this.HandleRotateTo);
	/**
	 * [Action] RotateToRelative client action
	 * @name realXtend.EntityController.EntityController#RotateToRelative
	 * @event
	 */
	me.Action("RotateToRelative").Triggered.connect(this, this.ClientHandleRotateToRelative);
	
	if (me.HasComponent("EC_DynamicComponent", "EntityControllerProperties"))
		this.syncProperties = me.GetComponentRaw("EC_DynamicComponent", "EntityControllerProperties");
	else
		me.ComponentAdded.connect(this, this.ClientComponentAdded);
};

/**
 * Uninitializes client object
 */
realXtend.EntityController.EntityController.prototype.ClientUninitialize = function()
{
	me.RemoveAction("Move");
	me.RemoveAction("Rotate");
	me.RemoveAction("GoTo");
	me.RemoveAction("LookAt");
	me.RemoveAction("RotateTo");
	me.RemoveAction("RotateToRelative");
};

realXtend.EntityController.EntityController.prototype.ClientComponentAdded = function(component, change)
{
	if (component.TypeName == "EC_DynamicComponent" && component.Name == "EntityControllerProperties")
	{
		this.syncProperties = component;
		me.ComponentAdded.disconnect(this, this.ClientComponentAdded);
	}
};

/**
 * Handles RotateToRelative action
 * @param {Number} x
 * @param {Number} y
 * @param {Number} z
 */
realXtend.EntityController.EntityController.prototype.ClientHandleRotateToRelative = function(x, y, z)
{	
	var temp = new Vector3df();
	temp.x = parseFloat(x) + me.placeable.transform.rot.x;
	temp.y = parseFloat(y) + me.placeable.transform.rot.y;
	temp.z = parseFloat(z) + me.placeable.transform.rot.z;
	temp = this.NormalizeRotation(temp);
	this.setRotateTo(temp);
	
	if (!server.IsRunning())
		//exec rotate action on server
		me.Exec(2, "RotateTo", temp.x, temp.y, temp.z);	
};
//SERVER METHODS
/**
 * Initializes server object.
 */
realXtend.EntityController.EntityController.prototype.ServerInitialize = function()
{	
	this.syncProperties = me.GetOrCreateComponentRaw("EC_DynamicComponent", "EntityControllerProperties");
	
	//init sync properties
	/**
	 * [Dynamic] Determines if object is active
	 * @name realXtend.EntityController.EntityController#active
	 * @type bool
	 */
	this.syncProperties.CreateAttribute("bool", "active");
	/**
	 * [Dynamic] Determines object movement velocity
	 * @name realXtend.EntityController.EntityController#movementVelocity
	 * @type real
	 */
	this.syncProperties.CreateAttribute("real", "movementVelocity");
	/**
	 * [Dynamic] Determines object max movement velocity factor
	 * @name realXtend.EntityController.EntityController#maxVelocityFactor
	 * @type real
	 */
	this.syncProperties.CreateAttribute("real", "maxVelocityFactor");
	/**
	 * [Dynamic] Determines object rotation velocity
	 * @name realXtend.EntityController.EntityController#rotationVelocity
	 * @type real
	 */
	this.syncProperties.CreateAttribute("real", "rotationVelocity");
	/**
	 * [Dynamic] Determines object max rotation velocity factor
	 * @name realXtend.EntityController.EntityController#maxRotationFactor
	 * @type real
	 */
	this.syncProperties.CreateAttribute("real", "maxRotationFactor");
	/**
	 * [Dynamic] Determines object x rotation mask 
	 * @name realXtend.EntityController.EntityController#rotationMaskX
	 * @type real
	 */
	this.syncProperties.CreateAttribute("real", "rotationMaskX");
	/**
	 * [Dynamic] Determines object y rotation mask
	 * @name realXtend.EntityController.EntityController#rotationMaskY
	 * @type real
	 */
	this.syncProperties.CreateAttribute("real", "rotationMaskY");
	/**
	 * [Dynamic] Determines object z rotation mask
	 * @name realXtend.EntityController.EntityController#rotationMaskZ
	 * @type real
	 */
	this.syncProperties.CreateAttribute("real", "rotationMaskZ");
	/**
	 * [Dynamic] Determines object x lookAtDirection
	 * @name realXtend.EntityController.EntityController#lookAtDirectionX
	 * @type real
	 */
	this.syncProperties.CreateAttribute("real", "lookAtDirectionX");
	/**
	 * [Dynamic] Determines object y lookAtDirection
	 * @name realXtend.EntityController.EntityController#lookAtDirectionY
	 * @type real
	 */
	this.syncProperties.CreateAttribute("real", "lookAtDirectionY");
	/**
	 * [Dynamic] Determines object z lookAtDirection
	 * @name realXtend.EntityController.EntityController#lookAtDirectionZ
	 * @type real
	 */
	this.syncProperties.CreateAttribute("real", "lookAtDirectionZ");
	
	this.syncProperties.SetAttribute("active", false);
	this.syncProperties.SetAttribute("movementVelocity", this.default_move_vel);
	this.syncProperties.SetAttribute("maxVelocityFactor", this.default_max_vel_factor);
	this.syncProperties.SetAttribute("rotationVelocity", this.default_rot_vel);
	this.syncProperties.SetAttribute("maxRotationFactor", this.default_max_rot_factor);
	this.syncProperties.SetAttribute("rotationMaskX", this.default_rot_mask.x);
	this.syncProperties.SetAttribute("rotationMaskY", this.default_rot_mask.y);
	this.syncProperties.SetAttribute("rotationMaskZ", this.default_rot_mask.z);
	this.syncProperties.SetAttribute("lookAtDirectionX", this.default_lookAtDirection.x);
	this.syncProperties.SetAttribute("lookAtDirectionY", this.default_lookAtDirection.y);
	this.syncProperties.SetAttribute("lookAtDirectionZ", this.default_lookAtDirection.z);
	
	// Connect actions
	me.Action("Move").Triggered.connect(this, this.HandleMove);
	me.Action("Rotate").Triggered.connect(this, this.HandleRotate);	
	//goto
	me.Action("GoTo").Triggered.connect(this, this.HandleGoTo);
	//lookat
	me.Action("LookAt").Triggered.connect(this, this.HandleLookAt);
	//rotateTo
	me.Action("RotateTo").Triggered.connect(this, this.HandleRotateTo);
};

/**
 * Uninitializes server object
 */
realXtend.EntityController.EntityController.prototype.ServerUninitialize = function()
{
	me.RemoveAction("Move");
	me.RemoveAction("Rotate");	
	me.RemoveAction("GoTo");
	me.RemoveAction("LookAt");
	me.RemoveAction("RotateTo");
	
	me.RemoveComponent("EC_DynamicComponent", "EntityControllerProperties");
	
};

/**
 * Triggered when Dynamic properties are changed
 * @param {IAttribute*} attribute
 * @param {AttributeChange::Type} type
 */
realXtend.EntityController.EntityController.prototype.ServerPropertiesChanged = function(attribute, type)
{
	// empty
	// This should be overridden in subclass.
};