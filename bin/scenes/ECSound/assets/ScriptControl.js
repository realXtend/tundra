//allows editing
//physics.SetRunPhysics(false);

//print("<-- ECSound test ScriptControl.js --:");

//Look for Entities with name Collision box and set RigidBody's mass to 2.0
var entityList = scene.GetEntityIdsWithComponent("EC_Script");

//print(entityList);

for( i in entityList)
{

	entity = scene.GetEntityRaw(entityList[i]);
	if(entity.name == "CollisionBox")
	{
		entity.rigidbody.mass = 2.0;
		entity.rigidbody.friction = 1.0;
	}
	

}
