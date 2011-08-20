/**
 * @name EntityController/PlaceableController.js
 * @fileOverview PlaceableController.js contains implementation of PlaceableController class.
 */

var PlaceableControllerCallMain = false;
if (typeof(realXtend) == "undefined")
	PlaceableControllerCallMain = true;


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
 * Main PathFollower.js
 * @static
 */
realXtend.EntityController.PlaceableControllerMain = function()
{
	var controller = new realXtend.EntityController.PlaceableController();
};


/**
 * Creates a PlaceableController object.
 * 
 * @class Controller of Placeable
 * @augments realXtend.EntityController.EntityController
 */
realXtend.EntityController.PlaceableController = function ()
{
	this.velocity = new Vector3df();
	//Initialize
	var isserver = server.IsRunning();
	if (isserver) {
	    this.ServerInitialize();
	} 
	else {
		this.ClientInitialize(); 
	}
};

realXtend.EntityController.PlaceableController.prototype = new realXtend.EntityController.EntityController;
realXtend.EntityController.PlaceableController.prototype.constructor = realXtend.EntityController.PlaceableController;

/**
 * @see realXtend.EntityController.EntityController#Destroy
 */
realXtend.EntityController.PlaceableController.prototype.Destroy = function()
{
	if (server.IsRunning())
		this.ServerUninitialize();
	//else
	//	this.ClientUninitialize();
};

//COMMON METHODS
/**
 * Common update method
 * @param {Number} frametime
 */
realXtend.EntityController.PlaceableController.prototype.CommonUpdate = function(frametime)
{
	if (me.placeable)
	{
		this.SyncObjects(frametime, [this.velocity]);
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
 * Initializes client object.
 */
realXtend.EntityController.PlaceableController.prototype.ClientInitialize = function()
{
	if(me.IsLocal())
		this.ServerInitialize();	
	else
		frame.Updated.connect(this, this.CommonUpdate);	
	
	// Connect actions
	/**
	 * [Action] Move client/server action
	 * @name realXtend.EntityController.PlaceableController#Move
	 * @event
	 * @param {Number} x
	 * @param {Number} y
	 * @param {Number} z
	 */
	me.Action("Move").Triggered.connect(this, this.Move);
	/**
	 * [Action] Rotate client/server action
	 * @name realXtend.EntityController.PlaceableController#Rotate
	 * @event
	 */
	me.Action("Rotate").Triggered.connect(this, this.Rotate);

};

/**
 * Uninitializes client object
 */
realXtend.EntityController.PlaceableController.prototype.ClientUninitialize = function()
{
	frame.Updated.disconnect(this, this.ClientUpdate);
	
	me.RemoveAction("Move");
	me.RemoveAction("Rotate");	
	
	realXtend.EntityController.EntityController.prototype.ClientUninitialize.call(this);
};

/**
 * Client update method called at every frame
 * @param {Number} frametime
 */
realXtend.EntityController.PlaceableController.prototype.ClientUpdate = function(frametime)
{
	this.CommonUpdate(frametime);	
};

//SERVER METHODS
/**
 * Initializes server object.
 */
realXtend.EntityController.PlaceableController.prototype.ServerInitialize = function()
{
	me.GetOrCreateComponentRaw("EC_Placeable");
	frame.Updated.connect(this, this.ServerUpdate);
	
};

/**
 * Uninitializes server object
 */
realXtend.EntityController.PlaceableController.prototype.ServerUninitialize = function()
{

};

/**
 * Placeable update method
 * @param {Number} frametime
 */
realXtend.EntityController.PlaceableController.prototype.ServerUpdate = function(frametime)
{   
	//move placeable
	var moveVec = new Vector3df();
	if ((this.getMove().x != 0) || (this.getMove().y != 0) || (this.getMove().z != 0))
	{
		this.ResetAutoUpdatedVariables();
		moveVec = this.getMove();
		moveVec = me.placeable.GetRelativeVector(moveVec);
	}
	else if (this.getGoTo() != 0)
	{
		var placeable_pos = me.placeable.transform.pos;
		var dest = new Vector3df();
		dest.x = this.getGoTo().x;
		dest.y = this.getGoTo().y;
		dest.z = this.getGoTo().z;
			
		if ((Math.abs(placeable_pos.x - dest.x) < this.goToThreshold) && (Math.abs(placeable_pos.y - dest.y) < this.goToThreshold) && (Math.abs(placeable_pos.z - dest.z) < this.goToThreshold))
		{
			this.setGoTo(0);
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
				this.setLookAt(this.getGoTo());
		}
	}
	//if there is movement
	if ((moveVec.x != 0) || (moveVec.y != 0) || (moveVec.z != 0))
		this.Move(moveVec.x, moveVec.y, moveVec.z, frametime);
		
	//rotate placeable
	var rotateVec = new Vector3df();
	if ((this.getRotate().x != 0) || (this.getRotate().y != 0) || (this.getRotate().z != 0))
	{
		this.ResetAutoUpdatedVariables();
		rotateVec = this.getRotate();
	}
	else if ((this.getLookAt() != 0) || (this.getRotateTo() != 0))
	{
		var lookAtRot = 0;
		if (this.getLookAt() != 0) //set lookAtRot for lookat variable
		{
			var lookAt = this.getLookAt();
			var rotationMaskX = this.syncProperties.GetAttribute("rotationMaskX");
			var rotationMaskY = this.syncProperties.GetAttribute("rotationMaskY");
					
			var diff = new Vector3df();
			var current_pos = me.placeable.transform.pos;
			diff.x = lookAt.x - current_pos.x;
			diff.y = lookAt.y - current_pos.y;
			diff.z = lookAt.z - current_pos.z;		
			if ((rotationMaskX==0) && (rotationMaskY==0))
				diff.z = 0;
			
			var lookAtDirectionX = this.syncProperties.GetAttribute("lookAtDirectionX");
			var lookAtDirectionY = this.syncProperties.GetAttribute("lookAtDirectionY");
			var lookAtDirectionZ = this.syncProperties.GetAttribute("lookAtDirectionZ");
			var mov_dir = new Vector3df();

			mov_dir.x = lookAtDirectionX;
			mov_dir.y = lookAtDirectionY;
			mov_dir.z = lookAtDirectionZ;
			
			mov_dir = me.placeable.GetRelativeVector(mov_dir);
			
			lookAtRot = me.placeable.GetRotationFromTo(mov_dir,diff);			
		}
		else if (this.getRotateTo() != 0)  //set lookAtRot for rotateTo variable
		{
			lookAtRot = new Vector3df();			
			lookAtRot.x	=  this.getRotateTo().x - me.placeable.transform.rot.x;
			lookAtRot.y =  this.getRotateTo().y - me.placeable.transform.rot.y;
			lookAtRot.z =  this.getRotateTo().z - me.placeable.transform.rot.z;
			lookAtRot = this.NormalizeRotation(lookAtRot);			
		}		
		
		if (lookAtRot != 0)
		{
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
			{	
				this.setLookAt(0);	
				this.setRotateTo(0);
			}		
		}
			
	}
	//if there is rotation
	if ((rotateVec.x != 0) || (rotateVec.y != 0) || (rotateVec.z != 0))
		this.Rotate(rotateVec.x, rotateVec.y, rotateVec.z, frametime);   
		
	this.CommonUpdate(frametime);
};

/**
 * Rotate action
 * @param {Number} x
 * @param {Number} y
 * @param {Number} z
 */
realXtend.EntityController.PlaceableController.prototype.Rotate = function(x, y, z) 
{
	if ((x != 0) || (y != 0) || (z != 0))
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
		
		var placeable = me.placeable;
        var transform = placeable.transform;

		transform.rot.x=transform.rot.x+v.x;
		transform.rot.y=transform.rot.y+v.y;
		transform.rot.z=transform.rot.z+v.z;
		
	    placeable.transform = transform;	
    }
};

/**
 * Move action
 * @param {Number} x
 * @param {Number} y
 * @param {Number} z
 */
realXtend.EntityController.PlaceableController.prototype.Move = function(x, y, z, frametime)
{
	var moveVec = new Vector3df();
	moveVec.x = x;
	moveVec.y = y;
	moveVec.z = z;
	
	var velocity = this.syncProperties.GetAttribute("movementVelocity");
	if (!velocity)
		return;		

	// translation
	var translation = new Vector3df();

	translation.x = velocity*moveVec.x*frametime;
	translation.y = velocity*moveVec.y*frametime;
	translation.z = velocity*moveVec.z*frametime;
	
	me.placeable.Translate(translation);
	
	this.velocity.x=translation.x/frametime;
	this.velocity.y=translation.y/frametime;
	this.velocity.z=translation.z/frametime;
};

if (PlaceableControllerCallMain)
	realXtend.EntityController.PlaceableControllerMain();