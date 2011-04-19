//Script is attached into Entity
//This script will need RigidBody and Sound component

print("Loading Collision Sound Script for enity");

//Test that needed components excist
var CollisionSound = me.GetComponent("EC_Sound", "Collision");
var RigidBody = me.GetComponent("EC_RigidBody");

if(CollisionSound && RigidBody)
{	
	print("Found needed Components");

	me.rigidbody.PhysicsCollision.connect(Collision);
	me.Action("MousePress").Triggered.connect(EntityClicked);
}	
else
{
	print("Missing needed Components, Check that you have RigidBody, and Sound(Collision)");
	me.ComponentAdded.connect(CheckComponent);
}

//Checking if needed components are added after Script component to Entity
 function CheckComponent(component, type)
{	
	if (component.typeName == "EC_Sound")
	{
		if(component.name == "Collision")
			CollisionSound = component;
	}
	else if(component.typeName == "RigidBody")
		RigidBody = true;
		
	if(CollisionSound && RigidBody)
	{	
		print("found needed Components");
		me.rigidbody.PhysicsCollision.connect(Collision);
		me.Action("MousePress").Triggered.connect(EntityClicked);
		me.ComponentAdded.disconnect(CheckComponent);
	}		
}
function Collision (otherEntity, pos, normal, distance, impulse, newCollision)
{
    if (impulse > 3.5)
    {
        // To play sound using the Audio API directly, do the following.
//	    audio.PlaySound(asset.GetAsset("local://Collision.ogg"));

	    // To play a sound clip stored to EC_Sound, do the following.
        me.sound.SoundId = "local://Collision.ogg";
	    me.Exec(1, "PlaySound");
	}
}

function EntityClicked()
{
	// To play a sound clip stored to EC_Sound, do the following.
    me.sound.SoundId = "local://Click.ogg";
	me.Exec(1, "PlaySound");
}
