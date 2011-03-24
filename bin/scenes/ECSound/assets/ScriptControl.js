//print("<-- ECSound test ScriptControl.js --:");

//Look for Entities with name Collision box and set RigidBody's mass to 2.0
var entityList = scene.GetEntityIdsWithComponent("EC_Script");

//print(entityList);

for( i in entityList)
{

	entity = scene.GetEntityRaw(entityList[i]);
	if(entity.Name == "CollisionBox")
	{
		entity.rigidbody.mass = 2.0;
		entity.rigidbody.friction = 1.0;
	}
	

}


//Get and Set Cameraposition

var cameraId = scene.GetEntityIdsWithComponent("EC_OgreCamera");
var camera = scene.GetEntityRaw(cameraId);
var transform = camera.placeable.transform;

transform.pos.x = 130.0;
transform.pos.y = 140.0;
transform.pos.z = 80.0;

transform.rot.x = 72.0;
transform.rot.y = 0.0;
transform.rot.z = -220.0;

camera.placeable.transform = transform;
