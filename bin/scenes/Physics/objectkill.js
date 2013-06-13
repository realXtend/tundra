// This script automatically deletes the objects in the scene as soon as they go too far below the ground.

frame.Updated.connect(Update);
function Update(dt)
{
    var entities = scene.Entities();
    for(i in entities)
    {
        var pos = entities[i].placeable;
        if (pos != null && pos.transform.pos.y < -200)
            scene.RemoveEntity(entities[i].id);
    }
}
