//allows editing
//physics.SetRunPhysics(false);

// Look for Entities with Script component and name "CollisionBox" and set RigidBody's mass to 2.0
var entities = scene.EntitiesWithComponent("EC_Script");
for(i in entities)
{
    entity = entities[i];
    if (entity.name == "CollisionBox")
    {
        entity.rigidbody.mass = 2.0;
        entity.rigidbody.friction = 1.0;
    }
}
