framework.Scene().SceneAdded.connect(OnSceneAdded);

var scene = null;
var avatarEntityId = 0;

function OnSceneAdded(scenename)
{
    // Get pointer to scene through framework
    scene = framework.Scene().GetScene(scenename);
    scene.EntityCreated.connect(OnEntityCreated);
}

function OnEntityCreated(entity, change)
{
    if (!server.IsRunning() && entity.name == "Avatar" + client.connectionId)
    {
        avatarEntityId = entity.id;
        frame.Updated.connect(ProduceRandomMovement);
    }
}

function ProduceRandomMovement()
{
    var entity = scene.GetEntity(avatarEntityId);
    if (entity == null)
    {
        frame.Updated.disconnect(ProduceRandomMovement);
        return;
    }

    var decision = Math.floor(Math.random() * 64);
    if (decision == 1)
        entity.Exec(EntityAction.Server, "Move", "forward");
    if (decision == 2)
        entity.Exec(EntityAction.Server, "Move", "back");
    if (decision == 3)
        entity.Exec(EntityAction.Server, "Move", "right");
    if (decision == 4)
        entity.Exec(EntityAction.Server, "Move", "left");
    if (decision == 5)
        entity.Exec(EntityAction.Server, "Stop", "forward");
    if (decision == 6)
        entity.Exec(EntityAction.Server, "Stop", "back");
    if (decision == 7)
        entity.Exec(EntityAction.Server, "Stop", "right");
    if (decision == 8)
        entity.Exec(EntityAction.Server, "Stop", "left");
}
