/**
 * @name PathFollow/PathFollower.js
 * @fileOverview PathFollower.js contains implementation of PathFollower class.
 * <dl>
 * <dt><b>Usage: </b>Add as an EC_Script to an entity to make it follow a path or include it from another script.</dt>
 * <dt><b>Requirements: </b></dt>
 * <dd>&bull; Entity should have a {@link realXtend.EntityController.EntityController} or {@link realXtend.EntityController.PlaceableController} implementation.</dd>
 * <dt><b>Actions</b>:</dt>
 * <dd>&bull; pauseFollow() Pauses following.</dd>
 * <dd>&bull; resumeFollow() Resumes following.</dd>
 * <dd>&bull; followPath() Starts following.</dd>
 * <dt><b>Dynamic attributes</b>:</dt>
 * <dd>&bull; Path {int}</dd>
 * <dd>&bull; Direction {int}</dd>
 * </dl>
 */

var PathFollowerCallMain = false;
if (typeof(realXtend) == "undefined")
	PathFollowerCallMain = true;

/**
 * @namespace main namespace.
 */
var realXtend = realXtend || {};

/**
 * @namespace PathFollow namespace contains functionality to follow paths. 
 */
realXtend.PathFollow = realXtend.PathFollow || {};

/**
 * Main PathFollower.js
 * @static
 */
realXtend.PathFollow.PathFollowerMain = function()
{
	var follower = new realXtend.PathFollow.PathFollower();
	if (server.IsRunning())
		follower.ServerFollowPath();
};

engine.IncludeFile("local://Path.js");

/**
 * Creates an PathFollower object.
 * 
 * @class Controller of the Path that follows the Entity
 */
realXtend.PathFollow.PathFollower = function()
{
	if (server.IsRunning())
		this.ServerInitialize();
	else 
		this.ClientInitialize();
};

realXtend.PathFollow.PathFollower.prototype.ClientInitialize = function()
{
	me.Action("pauseFollow").Triggered.connect(this, this.ClientPauseFollow);
	me.Action("resumeFollow").Triggered.connect(this, this.ClientResumeFollow);
	me.Action("followPath").Triggered.connect(this, this.ClientFollowPath);
};

realXtend.PathFollow.PathFollower.prototype.ClientPauseFollow = function()
{
	me.Exec(2, "pauseFollow");
};

realXtend.PathFollow.PathFollower.prototype.ClientResumeFollow = function()
{
	me.Exec(2, "resumeFollow");
};

realXtend.PathFollow.PathFollower.prototype.ClientFollowPath = function()
{
	me.Exec(2, "followPath");
};



realXtend.PathFollow.PathFollower.prototype.ServerInitialize = function()
{
	this.actualpath=0;
	this.direction=0;
	this.nextWayPoint = 0;
	this.nextWayPoint_entity=0;
	this.paused=false;
	this.primera=true;
	this.PathProperties = me.GetOrCreateComponentRaw("EC_DynamicComponent", "PathProperties");
	
	if(!this.PathProperties.ContainsAttribute("Path"))
	{
		/**
		 * [Dynamic] Determines path to follow
		 * @name realXtend.PathFollow.PathFollower#Path
		 * @type int
		 */
		this.PathProperties.CreateAttribute("int", "Path");
		
		this.PathProperties.SetAttribute("Path", 0);
	}	
	
	if(!this.PathProperties.ContainsAttribute("Direction"))
	{
		/**
		 * [Dynamic] Determines direction the path is followed
		 * @name realXtend.PathFollow.PathFollower#Direction
		 * @type int
		 */
		this.PathProperties.CreateAttribute("int", "Direction");
				
		this.PathProperties.SetAttribute("Direction", 1);
	}		
	/**
	 * [Action] pauseFollow action pauses following.
	 * @name realXtend.PathFollow.PathFollower#pauseFollow
	 * @event
	 */
	me.Action("pauseFollow").Triggered.connect(this, this.ServerPauseFollow);
	
	/**
	 * [Action] resumeFollow action resumes following.
	 * @name realXtend.PathFollow.PathFollower#resumeFollow
	 * @event
	 */
	me.Action("resumeFollow").Triggered.connect(this, this.ServerResumeFollow);
	
	/**
	 * [Action] followPath starts following a path.
	 * @name realXtend.PathFollow.PathFollower#followPath
	 * @event
	 */
	me.Action("followPath").Triggered.connect(this, this.ServerFollowPath);
	this.Riggid = me.GetComponentRaw("EC_RigidBody");
	
	if(this.Riggid){
		if(this.Riggid.physicsType==1){
			this.Riggid.physicsType=2;
		}
	}
	else{
		this.Riggid = me.GetOrCreateComponentRaw("EC_RigidBody");
		this.Riggid.phantom=true;
		var sizeVec = new Vector3df();
		sizeVec.x = 1;
		sizeVec.y = 1;
		sizeVec.z = 1;
		this.Riggid.size=sizeVec;	
		this.Riggid.physicsType=2;
	}
};


/**
 * Start the Path Following if the parameters (Path,Direction) exists.
 */
realXtend.PathFollow.PathFollower.prototype.ServerFollowPath = function() {
	this.nextWayPoint = 0;
	var pathnumber=this.PathProperties.GetAttribute("Path");
	this.actualpath=scene.GetEntityByNameRaw("Path["+pathnumber+"]");
	this.direction=this.PathProperties.GetAttribute("Direction");
	if(this.actualpath && (this.direction == 1 || this.direction == -1)){
			//print("Cambio al Camino: Path["+pathnumber+"]");
			if(this.primera){
				this.primera=false;
				this.Point();
			}
			else if(!this.primera){
				try {
					this.nextWayPoint_entity.volumetrigger.EntityEnter.disconnect(this, this.Arrive);
				} catch (e) {
					print("[PathFollower] Error when disconnecting from volume trigger:" + e.message);
					}
				me.Exec(2, "Move", 0,0,0);
				this.Point();
			}
		}
	else{
		print("Parametros incorrectos.");
	}
};

/**
 * @private
 */
realXtend.PathFollow.PathFollower.prototype.Point = function(){
	//print("getNextWaypoint("+this.actualpath.id+","+this.nextWayPoint+","+this.direction+");");
	var result = realXtend.PathFollow.getNextWaypoint(this.actualpath.id,this.nextWayPoint,this.direction);
	if (result[0]==0){
		//print("Path finalizado.");
		me.Exec(2, "Move", 0,0,0);
		this.primera=true;
	}
	else if (result[0]!=0){
		if (this.nextWayPoint==result[0]){
			this.Point();
		}
		else if (this.nextWayPoint!=result[0]){
			this.nextWayPoint = result[0];
			this.direction = result[1];
			this.nextWayPoint_entity=scene.GetEntityRaw(this.nextWayPoint);
			
			if (typeof(this.nextWayPoint_entity.volumetrigger) == "undefined" || 
					this.nextWayPoint_entity.volumetrigger == null)
				return;
			
			if (this.nextWayPoint_entity.volumetrigger.IsPivotInside(me)) {
				print("[PathFollower] entity already placed on next waypoint.");
				this.Point();
			}
			else
			{
				me.Exec(2, "goto", [this.nextWayPoint_entity.placeable.transform.pos.x,this.nextWayPoint_entity.placeable.transform.pos.y,this.nextWayPoint_entity.placeable.transform.pos.z,"true"]);
				//print("goto("+this.nextWayPoint_entity.placeable.transform.pos.x+","+this.nextWayPoint_entity.placeable.transform.pos.y+","+this.nextWayPoint_entity.placeable.transform.pos.z+");");
				//this.nextWayPoint_entity.rigidbody.drawDebug = true;
				this.nextWayPoint_entity.volumetrigger.EntityEnter.connect(this, this.Arrive);
			}
		}
	}
};

/**
 * @private
 * @param {IEntity*} Entity who arrives to one point.
 */
realXtend.PathFollow.PathFollower.prototype.Arrive = function(Entity){
	if (me.id == Entity.id){
		//this.nextWayPoint_entity.rigidbody.drawDebug = false;
		this.nextWayPoint_entity.volumetrigger.EntityEnter.disconnect(this, this.Arrive);
		this.Point();
	}
};

/**
 * Pauses the Path if the Entity is moving.
 */
realXtend.PathFollow.PathFollower.prototype.ServerPauseFollow = function() {
	if(this.paused==true){
		//print("Already paused");
	}
	else{
		this.paused=true;
		//print("Path following paused");
		//this.nextWayPoint_entity.rigidbody.drawDebug = false;
		this.nextWayPoint_entity.volumetrigger.EntityEnter.disconnect(this, this.Arrive);
		me.Exec(2, "Move", 0,0,0);
	}
};

/**
 * Resumes the Path if following is paused.
 */
realXtend.PathFollow.PathFollower.prototype.ServerResumeFollow = function() {
	if(this.paused==true){
		//print("Path following resumed");
		this.paused=false;
		//print("Tienes que volver al punto: "+this.nextWayPoint);
		me.Exec(2, "goto", [this.nextWayPoint_entity.placeable.transform.pos.x,this.nextWayPoint_entity.placeable.transform.pos.y,this.nextWayPoint_entity.placeable.transform.pos.z,"true"]);
		//print("goto("+this.nextWayPoint_entity.placeable.transform.pos.x+","+this.nextWayPoint_entity.placeable.transform.pos.y+","+this.nextWayPoint_entity.placeable.transform.pos.z+");");
		//this.nextWayPoint_entity.rigidbody.drawDebug = true;
		this.nextWayPoint_entity.volumetrigger.EntityEnter.connect(this, this.Arrive);
	}
	else{
		//print("Already following the Path");
	}
};

if (PathFollowerCallMain)
	realXtend.PathFollow.PathFollowerMain();
