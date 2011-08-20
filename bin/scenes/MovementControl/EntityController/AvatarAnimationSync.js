
/**
 * @namespace main namespace.
 */
var realXtend = realXtend || {};

/**
 * @namespace EntityController namespace contains functionality to control entities position and rotation. 
 */
realXtend.EntityController = realXtend.EntityController || {};

realXtend.EntityController.AvatarAnimationSync = function()
{
	//Animation detection
	this.standAnimName = "Stand";
	this.walkAnimName = "Walk";
	this.runAnimName = "Run";
	this.flyAnimName = "Fly";
	this.hoverAnimName = "Hover";
	this.sitAnimName = "SitOnGround";
	this.waveAnimName = "Wave";
	this.animsDetected = false;
	
	this.walk_anim_speed = 0.5;
	this.run_anim_speed = 0.25;
	
	this.velocityTrigger = 0.1;

	this.isserver = server.IsRunning();
	if (!this.isserver) {
		me.Action("Gesture").Triggered.connect(this, this.ClientHandleGesture);
	}
	this.CommonFindAnimations();
};

realXtend.EntityController.AvatarAnimationSync.prototype.Update = function(frametime, params) {
	var velocity = params[0];
	var flying = params[1];
	var running = params[2];
	var onGround = params[3];
	if (this.isserver) {
		this.ServerSetAnimationState(velocity, flying, running, onGround);
		this.CommonUpdateAnimation(frametime, velocity);
	}
	else {
		this.CommonUpdateAnimation(frametime, velocity);
	}
};


realXtend.EntityController.AvatarAnimationSync.prototype.CommonFindAnimations = function () {
    var animcontrol = me.animationcontroller;
    if (animcontrol == null)
    	return;
    var availableAnimations = animcontrol.GetAvailableAnimations();
    if (availableAnimations.length > 0) {
        // Detect animation names
        var searchAnims = [this.standAnimName, this.walkAnimName, this.flyAnimName, this.hoverAnimName, this.sitAnimName, this.waveAnimName];
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
        this.standAnimName = searchAnims[0];
        this.walkAnimName = searchAnims[1];
        this.flyAnimName = searchAnims[2];
        this.hoverAnimName = searchAnims[3];
        this.sitAnimName = searchAnims[4];

        this.animsDetected = true;
    }
};

realXtend.EntityController.AvatarAnimationSync.prototype.CommonUpdateAnimation = function(frametime, velocity) {
    if (!this.animsDetected) {
    	this.CommonFindAnimations();
        return;
    }

    var animcontroller = me.animationcontroller;
    if (animcontroller == null){
        return;
    }

    var animName = animcontroller.animationState;

    // Enable animation, skip with headless server
    if (animName != "" && !framework.IsHeadless()) {
        // Do custom speeds for certain anims
        if (animName == this.hoverAnimName) {
            animcontroller.SetAnimationSpeed(animName, 0.25);
        }
        if (animName == this.sitAnimName) { // Does not affect the anim speed on jack at least?!
            animcontroller.SetAnimationSpeed(animName, 0.5);
        }
        if (animName == this.waveAnimName) {
            animcontroller.SetAnimationSpeed(animName, 0.75);
        }
        // Enable animation
        if (!animcontroller.IsAnimationActive(animName)) {
            // Gestures with non exclusive
            if (animName == this.waveAnimName) {
                animcontroller.EnableAnimation(animName, false, 0.25, 0.25, false);
            // Normal anims exclude others
            } else {
                animcontroller.EnableExclusiveAnimation(animName, true, 0.25, 0.25, false);
            }
        }
    }

 // If walk animation is playing, adjust its speed according to the avatar rigidbody velocity
    if (animName != ""  && animcontroller.IsAnimationActive(this.walkAnimName)) {
        // Note: on client the rigidbody does not exist, so the velocity is only a replicated attribute
        var walkspeed = Math.sqrt(velocity.x * velocity.x + velocity.y * velocity.y) * this.walk_anim_speed;        
        if (velocity.x < -this.velocityTrigger)
        	walkspeed = -walkspeed;
        animcontroller.SetAnimationSpeed(animName, walkspeed);
    }
    else if (animName != ""  && animcontroller.IsAnimationActive(this.runAnimName)) {
        // Note: on client the rigidbody does not exist, so the velocity is only a replicated attribute
        var runspeed = Math.sqrt(velocity.x * velocity.x + velocity.y * velocity.y) * this.run_anim_speed;
        if (velocity.x < -this.velocityTrigger)
        	runspeed = -runspeed;
        animcontroller.SetAnimationSpeed(animName, runspeed);
    }
};


realXtend.EntityController.AvatarAnimationSync.prototype.ClientHandleGesture = function(gestureName) {
    var animName = "";
    if (gestureName == "wave") {
        animName = this.waveAnimName;
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
};


realXtend.EntityController.AvatarAnimationSync.prototype.ServerSetAnimationState = function(velocity, flying, running, onGround) {

    var animName = this.standAnimName;
    // Not flying: Stand, Walk, Run, Crouch, Jump or Fall
    if (!flying)
    {
    	if (velocity.z > this.velocityTrigger) { //Jump
    		animName = this.hoverAnimName;
    	}
    	else if (velocity.z < - this.velocityTrigger){ //Crouch or Fall
    		if (onGround) {
    			animName = this.sitAnimName;
    		}
    		else {
    			animName = this.hoverAnimName;
    		}
    	}
    	else { //Stand or Walk
    		if ((Math.abs(velocity.x) > this.velocityTrigger) || (Math.abs(velocity.y) > this.velocityTrigger)) {
        		if (running)
        			animName = this.runAnimName;
        		else
        			animName = this.walkAnimName;
        	}
    	}
    }
    else { // Flying: Fly if moving in x-axis, otherwise hover
    	animName = this.flyAnimName;
        if (Math.abs(velocity.x) < this.velocityTrigger)
        	animName = this.hoverAnimName;
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
};
