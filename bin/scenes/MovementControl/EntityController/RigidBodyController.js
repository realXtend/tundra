/**
 * @name EntityController/RigidBodyController.js
 * @fileOverview RigidBodyController.js contains implementation of RigidBodyController class, which 
 * specializes EntityController class for a dynamic physics object.
 * <p><b>Usage: </b>Currently must be used from within another script.
 */

engine.IncludeFile("local://EntityController.js");

/**
 * @namespace main namespace.
 */
var realXtend = realXtend || {};

/**
 * @namespace EntityController namespace contains functionality to control entities position and rotation. 
 */
realXtend.EntityController = realXtend.EntityController || {};

/**
 * Creates a RigidBodyController object.
 * 
 * @class Controller of RigidBody
 * @augments realXtend.EntityController.EntityController
 */
realXtend.EntityController.RigidBodyController = function ()
{
	var isserver = server.IsRunning();
	if (isserver) 
	{
		this.default_fly_vel_factor = 1.5;
		this.default_run_vel_factor = 2.0;
		this.timeOnGround = 0;
		this.minTimeOnGround = 1;
		this.jump_velocity = 7.0;
		this.fly_jump_meters = 3.0;
		this.angularDamping = 0.5;
	    this.ServerInitialize();
	}
	else
		this.ClientInitialize();
	
};

realXtend.EntityController.RigidBodyController.prototype = new realXtend.EntityController.EntityController;
realXtend.EntityController.RigidBodyController.prototype.constructor = realXtend.EntityController.RigidBodyController;

/**
 * @see realXtend.EntityController.EntityController#Destroy
 */
realXtend.EntityController.RigidBodyController.prototype.Destroy = function()
{
	if (server.IsRunning())
		this.ServerUninitialize();
	else
		this.ClientUninitialize();
};

//COMMON METHODS
/**
 * Common update method
 * @param {Number} frametime
 */
realXtend.EntityController.RigidBodyController.prototype.CommonUpdate = function(frametime)
{
	if (me.rigidbody)
	{
		var velocity = me.rigidbody.linearVelocity;
		velocity = me.placeable.GetAbsoluteVector(velocity);
		var flying = false;
		var onGround = true;
		var running = false;
		
		if (this.syncProperties)
		{
			flying = this.syncProperties.GetAttribute("flying");
			running = this.syncProperties.GetAttribute("running");
			onGround = this.syncProperties.GetAttribute("onGround");
		}
		
		if (onGround) {
			if (this.getMove().z < 0)
				velocity.z = -1;
			else
				velocity.z = 0;
		}
	
		this.SyncObjects(frametime, [velocity, flying, running, onGround]);
	}
	
	if (this.getRotateAutoStop() == true)
	{
		var tmpVec = new Vector3df();
		tmpVec.x = 0;
		tmpVec.y = 0;
		tmpVec.z = 0;
		this.setRotate(tmpVec);
		this.setRotateAutoStop(false);
	}

};

//CLIENT METHODS
/**
 * @see realXtend.EntityController.EntityController#ClientInitialize
 */
realXtend.EntityController.RigidBodyController.prototype.ClientInitialize = function()
{	
	// Connect actions
	/**
	 * [Action] ToggleFly action toggles fly mode.
	 * @name realXtend.EntityController.RigidBodyController#ToggleFly
	 * @event
	 */
	me.Action("ToggleFly").Triggered.connect(this, this.ClientHandleToggleFly);
	/**
	 * [Action] ToggleRun action toggles run mode.
	 * @name realXtend.EntityController.RigidBodyController#ToggleRun
	 * @event
	 */
	me.Action("ToggleRun").Triggered.connect(this, this.ClientHandleToggleRun);
	
	
    // Hook to tick update to update visual effects (both own and others' avatars)
    frame.Updated.connect(this, this.ClientUpdate);
};

/**
 * @see realXtend.EntityController.EntityController#ClientUninitialize
 */
realXtend.EntityController.RigidBodyController.prototype.ClientUninitialize = function()
{
	frame.Updated.disconnect(this, this.ClientUpdate);
	
	me.RemoveAction("ToggleFly");
	me.RemoveAction("ToggleRun");
	
	realXtend.EntityController.EntityController.prototype.ClientUninitialize.call(this);
};

/**
 * Client update method called at every frame
 * @param {Number} frametime
 */
realXtend.EntityController.RigidBodyController.prototype.ClientUpdate = function(frametime)
{
	this.CommonUpdate(frametime);	
};

/**
 * Handles ToggleFly action
 */
realXtend.EntityController.RigidBodyController.prototype.ClientHandleToggleFly = function()
{
	if (this.syncProperties)
	{
		if (this.syncProperties.GetAttribute("flying")==false)
			this.syncProperties.SetAttribute("flying", true);
		else
			this.syncProperties.SetAttribute("flying", false);
	}
};

/**
 * Handles ToggleRun action
 */
realXtend.EntityController.RigidBodyController.prototype.ClientHandleToggleRun = function()
{
	if (this.syncProperties)
	{
		if (this.syncProperties.GetAttribute("running")==false)
			this.syncProperties.SetAttribute("running", true);
		else
			this.syncProperties.SetAttribute("running", false);
	}		
};


//SERVER METHODS
/**
 * @see realXtend.EntityController.EntityController#ServerInitialize
 */
realXtend.EntityController.RigidBodyController.prototype.ServerInitialize = function()
{
	me.GetOrCreateComponentRaw("EC_RigidBody");
	
	//init sync properties
	/**
	 * [Dynamic] Determines if entity is flying
	 * @name realXtend.EntityController.RigidBodyController#flying
	 * @type bool
	 */
	this.syncProperties.CreateAttribute("bool", "flying");
	
	/**
	 * [Dynamic] Determines if entity flying movement velocity factor
	 * @name realXtend.EntityController.RigidBodyController#flyingMovementVelocityFactor
	 * @type real
	 */
	this.syncProperties.CreateAttribute("real", "flyingMovementVelocityFactor");
	
	/**
	 * [Dynamic] Determines if entity is running
	 * @name realXtend.EntityController.RigidBodyController#running
	 * @type bool
	 */
	this.syncProperties.CreateAttribute("bool", "running");
	
	/**
	 * [Dynamic] Determines entity running movement velocity factor
	 * @name realXtend.EntityController.RigidBodyController#runningMovementVelocityFactor
	 * @type real
	 */
	this.syncProperties.CreateAttribute("real", "runningMovementVelocityFactor");
	
	/**
	 * [Dynamic] Determines if entity is on ground
	 * @name realXtend.EntityController.RigidBodyController#onGround
	 * @type bool
	 */
	this.syncProperties.CreateAttribute("bool", "onGround");
	
	this.syncProperties.SetAttribute("flying", false);
	this.syncProperties.SetAttribute("flyingMovementVelocityFactor", this.default_fly_vel_factor);
	this.syncProperties.SetAttribute("running", false);
	this.syncProperties.SetAttribute("runningMovementVelocityFactor", this.default_run_vel_factor);
	this.syncProperties.SetAttribute("onGround", false);	

	this.syncProperties.AttributeChanged.connect(this, this.ServerPropertiesChanged);
	
	// Hook to physics update
	me.rigidbody.GetPhysicsWorld().Updated.connect(this, this.ServerUpdate);
};

/**
 * @see realXtend.EntityController.EntityController#ServerUninitialize
 */
realXtend.EntityController.RigidBodyController.prototype.ServerUninitialize = function()
{
	me.rigidbody.GetPhysicsWorld().Updated.disconnect(this, this.ServerUpdate);
	
	this.syncProperties.AttributeChanged.disconnect(this, this.ServerPropertiesChanged);
	
	this.syncProperties.RemoveAttribute("flying");
	this.syncProperties.RemoveAttribute("flyingMovementVelocityFactor");
	this.syncProperties.RemoveAttribute("running");
	this.syncProperties.RemoveAttribute("runningMovementVelocityFactor");
	this.syncProperties.RemoveAttribute("onGround");
	
	realXtend.EntityController.EntityController.prototype.ServerUninitialize.call(this);
};

/**
 * @see realXtend.EntityController.EntityController#ServerPropertiesChanged
 */
realXtend.EntityController.RigidBodyController.prototype.ServerPropertiesChanged = function(attribute, type)
{
	if (attribute.name == "flying")
	{
		this.ServerHandleFly();
	}
};

/**
 * Server update method called at every physics simulation tick
 * @param {Number} frametime
 */
realXtend.EntityController.RigidBodyController.prototype.ServerUpdate = function(frametime)
{   
	var onGround = this.syncProperties.GetAttribute("onGround");
	var flying = this.syncProperties.GetAttribute("flying");
	var goToLooking = 0;
	
	if (!onGround && !flying)
	{
		//calculate if rigid body is onGround	
		if (this.IsOnGround(frametime))
		{
			this.syncProperties.SetAttribute("onGround",true);
			onGround = true;
		}
	}
	
	//if is flying and is onground. goTo x meters up
	if (flying && onGround)
	{
		var tmpVec = new Vector3df();
		tmpVec.x = me.placeable.transform.pos.x;
		tmpVec.y = me.placeable.transform.pos.y;
		tmpVec.z = me.placeable.transform.pos.z + this.fly_jump_meters;
		this.setGoTo(tmpVec);
		this.syncProperties.SetAttribute("onGround", false);
	}
	
	//move rigidbody
	var moveVec = new Vector3df();
	if ((this.getMove().x != 0) || (this.getMove().y != 0) || (this.getMove().z != 0))
	{
		this.ResetAutoUpdatedVariables();
		moveVec = this.getMove();
		moveVec = me.placeable.GetRelativeVector(moveVec);
	}
	else if (this.getGoTo() != 0)
	{
		//calc lookat vector and set this.lookAt
		var placeable_pos = me.placeable.transform.pos;
		var dest = new Vector3df();
		dest.x = this.getGoTo().x;
		dest.y = this.getGoTo().y;
		dest.z = this.getGoTo().z;
		
		//if rigid body is not flying and is on ground move rigid body in xy axis
		if (!flying && onGround)
		{
			placeable_pos.z = 0;
			dest.z = 0;
		}
		
		if ((Math.abs(placeable_pos.x - dest.x) < this.goToThreshold) && (Math.abs(placeable_pos.y - dest.y) < this.goToThreshold) && (Math.abs(placeable_pos.z - dest.z) < this.goToThreshold))
		{
			this.StopMove(true);
		}
		else
		{
			//set move vector 			
			moveVec.x = dest.x - placeable_pos.x;
			moveVec.y = dest.y - placeable_pos.y;
			moveVec.z = dest.z - placeable_pos.z;
			
			moveVec = moveVec.normalize();
			//print("goTo moveVec = x:" + moveVec.x + " y:" + moveVec.y + " z:" + moveVec.z);
			
			//set lookat if goTolooking is true
			if (this.getGoToLooking() == true)
				goToLooking = this.getGoTo();
		}
	}
	//if there is movement
	if ((moveVec.x != 0) || (moveVec.y != 0) || (moveVec.z != 0) || me.rigidbody.IsActive())
	{		
		this.Move(moveVec.x, moveVec.y, moveVec.z, frametime);
	}
	
	//rotate
	var rotateVec = new Vector3df();
	if ((this.getRotate().x != 0) || (this.getRotate().y != 0) || (this.getRotate().z != 0))
	{
		this.ResetAutoUpdatedVariables();
		rotateVec = this.getRotate();
	}
	else if ((this.getLookAt() != 0) || (this.getRotateTo() != 0) || (goToLooking != 0))
	{
		var lookAtRot = 0;
		var lookAt = 0;
		if (goToLooking != 0) //first check if goTo looking is enabled
			lookAt = goToLooking;
		else if (this.getLookAt() != 0)
			lookAt = this.getLookAt();

		if (lookAt) //set lookAtRot for lookat variable
		{			
			var rotationMaskX = this.syncProperties.GetAttribute("rotationMaskX");
			var rotationMaskY = this.syncProperties.GetAttribute("rotationMaskY");
					
			var diff = new Vector3df();
			var current_pos = me.placeable.transform.pos;
			diff.x = lookAt.x - current_pos.x;
			diff.y = lookAt.y - current_pos.y;
			diff.z = lookAt.z - current_pos.z;		
			if ((rotationMaskX==0) && (rotationMaskY==0))
				diff.z = 0;
			
			var mov_dir = new Vector3df();
			mov_dir.x = 1;
			
			mov_dir = me.placeable.GetRelativeVector(mov_dir);
			
			lookAtRot = me.placeable.GetRotationFromTo(mov_dir,diff);			
		}
		else if (this.getRotateTo() != 0)  //set lookAtRot for rotateTo variable
		{
			lookAtRot = new Vector3df();			
			lookAtRot.x	=  this.getRotateTo().x - me.placeable.transform.rot.x;
			lookAtRot.y =  this.getRotateTo().y - me.placeable.transform.rot.y;
			lookAtRot.z =  this.getRotateTo().z - me.placeable.transform.rot.z;					
		}
		
		if (lookAtRot != 0)
		{
			lookAtRot = this.NormalizeRotation(lookAtRot);
			//print(lookAtRot.x + " "+ lookAtRot.y +" "+lookAtRot.z);
			//rotate with default velocity (set 1 or -1 rotateVec)
			if (lookAtRot.z > this.lookAtThreshold)
				rotateVec.z = 1;
			else if (lookAtRot.z < -this.lookAtThreshold)
				rotateVec.z = -1;
			if (lookAtRot.y > this.lookAtThreshold)
				rotateVec.y = 1;
			else if (lookAtRot.y < -this.lookAtThreshold)
				rotateVec.y = -1;			
			if (lookAtRot.x > this.lookAtThreshold)
				rotateVec.x = 1;
			else if (lookAtRot.x < -this.lookAtThreshold)
				rotateVec.x = -1;

			
			if ((rotateVec.x == 0) && (rotateVec.y == 0) && (rotateVec.z == 0))			
				this.StopRotate(true);		
		}
	}			
	
	//rotate rigidbody
	this.Rotate(rotateVec.x, rotateVec.y, rotateVec.z);	
	
	this.CommonUpdate(frametime);
};

/**
 * Checks if rigid body is on ground or not
 * @param {Number} frametime
 * @returns {Boolean}
 */
realXtend.EntityController.RigidBodyController.prototype.IsOnGround = function(frametime) 
{
	if (Math.abs(me.rigidbody.linearVelocity.z) < 0.005)
		this.timeOnGround += frametime;
	if (this.timeOnGround != 0 && Math.abs(me.rigidbody.linearVelocity.z) > 0.005)
		this.timeOnGround = 0;
	if (this.timeOnGround > this.minTimeOnGround)
	{	
		this.timeOnGround = 0;
		return true;
	}
	return false;
};

/**
 * Performs rotation
 * @param {Number} x
 * @param {Number} y
 * @param {Number} z
 */
realXtend.EntityController.RigidBodyController.prototype.Rotate = function(x, y, z) 
{
	//rotate rigid body
	if ((x != 0) || (y != 0) || (z != 0) || me.rigidbody.IsActive())
	{
		var velocity = this.syncProperties.GetAttribute("rotationVelocity");
		if (!velocity)
			return;
		
		var rotationMaskX = this.syncProperties.GetAttribute("rotationMaskX");
		var rotationMaskY = this.syncProperties.GetAttribute("rotationMaskY");
		var rotationMaskZ = this.syncProperties.GetAttribute("rotationMaskZ");
		
		if (isNaN(rotationMaskX) || isNaN(rotationMaskY) || isNaN(rotationMaskZ))
			return;
		
		
        var rotateVec = new Vector3df();
        rotateVec.x = x *rotationMaskX;
        rotateVec.y = y *rotationMaskY;
        rotateVec.z = z *rotationMaskZ;        
        
        var v = new Vector3df();
        v.x = velocity * rotateVec.x/180 * Math.PI;
        v.y = velocity * rotateVec.y/180 * Math.PI;
        v.z = velocity * rotateVec.z/180 * Math.PI;
        
		//transform velocity vector to placeable coordinates
		//TorqueImpulse: a * t = ((vf - vi)/ t) * t = (vf - vi)
		var v_current = me.rigidbody.angularVelocity;
		//print("v_g = z:" + v.z);
		//print("v_c = z:" + v_current.z);
		rotateVec.x = (v.x - v_current.x/180*Math.PI);
		rotateVec.y = (v.y - v_current.y/180*Math.PI);
		rotateVec.z = (v.z - v_current.z/180*Math.PI);
		
		//Apply damping. 
		if (x == 0)
			rotateVec.x *= this.angularDamping;
		if (y == 0)
			rotateVec.y *= this.angularDamping;
		if (z == 0)
			rotateVec.z *= this.angularDamping;
		//print("Rotating rigid body: x=", rotateVec.x, " y=", rotateVec.y, " z=", rotateVec.z);
		me.rigidbody.ApplyTorqueImpulse(rotateVec);
    }
};

/**
 * Performs movement
 * @param {Number} x
 * @param {Number} y
 * @param {Number} z
 * @param {Number} frametime
 */
realXtend.EntityController.RigidBodyController.prototype.Move = function(x, y, z, frametime)
{
	var moveVec = new Vector3df();
	moveVec.x = x;
	moveVec.y = y;
	moveVec.z = z;
	
	var onGround = this.syncProperties.GetAttribute("onGround");
	var flying = this.syncProperties.GetAttribute("flying");
	var running = this.syncProperties.GetAttribute("running");
	
	var jump = false;
	var velocity = this.syncProperties.GetAttribute("movementVelocity");		
	if (flying)
		velocity *= this.syncProperties.GetAttribute("flyingMovementVelocityFactor");
	else if (running)
		velocity *= this.syncProperties.GetAttribute("runningMovementVelocityFactor");
	
    // Apply motion force
   
	var impulseVec = new Vector3df();
	// velocity
	var v = new Vector3df();
	v.x = velocity*moveVec.x;
	v.y = velocity*moveVec.y;
	v.z = velocity*moveVec.z;
	
	if (this.getMove().z > 0 && onGround) {
		// jump velocity
		v.z = this.jump_velocity;
		this.getMove().z = 0;
		this.syncProperties.SetAttribute("onGround", false);
		jump = true;
	}
	//transform velocity vector to placeable coordinates
	//var v_g = me.placeable.GetRelativeVector(v);
	//Impulse: F * t 
	//Force: m * a 
	//Acceleration = ((vf - vi)/ t)
	//Impulse = (vf - vi) * m
	var v_current = me.rigidbody.linearVelocity;			
	impulseVec.x = (v.x - v_current.x) * me.rigidbody.mass;		
	impulseVec.y = (v.y - v_current.y) * me.rigidbody.mass;
	if (flying || jump)
		impulseVec.z = (v.z - v_current.z) * me.rigidbody.mass;
		
	
	if (Math.abs(impulseVec.x) > 0.5 || Math.abs(impulseVec.y) > 0.5 || Math.abs(impulseVec.z) > 0.5)
	{
		me.rigidbody.ApplyImpulse(impulseVec);
		//print("v = x:" + v_current.x + " y:" + v_current.y + " z:" + v_g.z)
		//print("impulse = x:" + impulseVec.x + " y:" + impulseVec.y + " z:" + impulseVec.z);
	}	
};

/**
 * Handles {@link realXtend.EntityController.RigidBodyController#flying} changes
 */
realXtend.EntityController.RigidBodyController.prototype.ServerHandleFly = function() {
    var rigidbody = me.rigidbody;

    var flying = this.syncProperties.GetAttribute("flying");
    if (flying) {
		var gravity = new Vector3df(0,0,0);
		rigidbody.ApplyGravity(gravity);		
    } else {
		rigidbody.ApplyGravity(rigidbody.GetPhysicsWorld().GetGravity());
        // Reset the x rot if left
        var av_placeable = me.placeable;
        var av_transform = av_placeable.transform;
        if (av_transform.rot.x != 0) {
            av_transform.rot.x = 0;
            av_placeable.transform = av_transform;
        }
        // Push rigid body a bit to the fly direction
        // so the motion does not just stop to a wall
        var moveVec = new Vector3df();
        moveVec.x = this.getMove().x * 120;
        moveVec.y = this.getMove().y * 120;
        moveVec.z = this.getMove().z * 120;
        var pushVec = av_placeable.GetRelativeVector(moveVec);
        me.rigidbody.ApplyImpulse(pushVec);		
    }
};






