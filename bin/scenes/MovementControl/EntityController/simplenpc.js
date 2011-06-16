/**
 * @name EntityController/simplenpc.js
 * @fileOverview A simple npc avatar with physics & third person camera
 * <p><b>Usage: </b>Add as an EC_Script to an entity to make it an npc.
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
 * @class A simple npc avatar with physics & third person camera
 */
realXtend.EntityController.SimpleNpc = function()
{
	this.avatar_mass = 10;
	this.rotationVec = new Vector3df();
	this.rotationVec.z = 1;
	
	// Needed bools for logic
	this.isserver = server.IsRunning();
	
	//Create Entity Controller
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
realXtend.EntityController.SimpleNpc.prototype.ServerInitialize = function () {	
    if (!me.HasComponent("EC_Avatar")){
		var avatar = me.GetOrCreateComponentRaw("EC_Avatar");
	    var r = avatar.appearanceRef;
	    r.ref = "local://default_avatar.xml";
	    avatar.appearanceRef = r;
	}
    var voice = me.GetOrCreateComponentRaw("EC_TtsVoice");
       
    var rigidbody = me.rigidbody; 
    // Set the avatar appearance. This creates the mesh & animationcontroller, once the avatar asset has loaded
    // Note: for now, you need the default_avatar.xml in your bin/data/assets folder

    // Set voice to avatar
    voice.voice = "Spanish male";

    // Set physics properties
    var sizeVec = new Vector3df();
    sizeVec.z = 2.4;
    sizeVec.x = 0.5;
    sizeVec.y = 0.5;
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

realXtend.EntityController.SimpleNpc.prototype.ClientInitialize = function() {
    // Make hovering name tag for clients
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
};

new realXtend.EntityController.SimpleNpc();