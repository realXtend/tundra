// This script automatically deletes the objects in the scene as soon as they go too far below the ground.

frame.Updated.connect(Update);
function Update(dt)
{
    var entities = scene.GetAllEntities();

    for(i = 0; i < entities.length; i++)
    {
        var pos = entities[i].GetComponent("EC_Placeable");
        if (pos != null && pos.transform.pos.y < -200)
            scene.RemoveEntity(entities[i].id);
    }
}
