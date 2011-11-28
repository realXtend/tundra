//Script is attached into Entity
//This script will need RigidBody and Sound component

print("Loading Collision Sound Script for entity");
//audio.PlaySound(asset.GetAsset("local://Collision.ogg"));

//Test that needed components exist
var collisionSound = me.GetComponent("EC_Sound", "Collision");
var rigidBody = me.GetComponent("EC_Sound", "Collision");
if(collisionSound && rigidBody) {
    print("Found needed Components at start, initing.");
    init();
}
else {
    print("Missing needed Components at start - waiting for: RigidBody, and Sound(Collision)");
    me.ComponentAdded.connect(checkComponent);
}

//called when all the necessary components are here. core could perhaps support having ECs as code deps.
function init() {
    me.rigidbody.PhysicsCollision.connect(collision);
    me.Action("MousePress").Triggered.connect(entityClicked);
}

//Checking if needed components are added after Script component to Entity
function checkComponent(component, type) {      
    if (component.typeName == "EC_Sound") {
        if(component.name == "Collision")
            collisionSound = component;
    }
    else if(component.typeName == "RigidBody")
        rigidBody = true;
                
    if(collisionSound && rigidBody) {
        print("found needed Components");
        me.ComponentAdded.disconnect(checkComponent);
        init();
    }               
}

function collision (otherEntity, pos, normal, distance, impulse, newCollision) {
    //print("Collision: " + impulse);
    if (impulse > 3.5) {
        // To play sound using the Audio API directly, do the following.
        //      audio.PlaySound(asset.GetAsset("local://Collision.ogg"));
        //audio.PlaySound(asset.GetAsset("local://Collision.ogg"));

        // To play a sound clip stored to EC_Sound, do the following.
        me.sound.soundRef = "local://Collision.ogg";
        me.Exec(1, "PlaySound");
    }
}

function entityClicked() {
    print("click");
    var sndref = "local://Click.ogg";
    //sndref = "local://Collision.ogg"
    //audio.PlaySound(asset.GetAsset(sndref));

    // To play a sound clip stored to EC_Sound, do the following.
    me.sound.soundRef = sndref;
    print(me.sound.soundRef.ref);
    me.Exec(1, "PlaySound");
}
