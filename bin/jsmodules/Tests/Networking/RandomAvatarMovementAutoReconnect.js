// For conditions of distribution and use, see copyright notice in LICENSE

var scene = null;
var avatarEntityId = 0;

var address ="localhost";
var port = 2345;
var protocol = "udp";
var username = "RandomAvatarMovementReconnect";
var password = "";

function RandomTimeToLive()
{
    return Math.random() * 15;
}

function RandomReconnectionTime()
{
    return Math.random() * 5;
}

var connectArg = framework.CommandLineParameters("--connect");
if (connectArg.length > 0)
{
    var args = connectArg[0].split(";");
    if (args.length > 0)
        address = args[0];
    if (args.length > 1)
        port = args[1];
    if (args.length > 2)
        protocol = args[2];
    if (args.length > 3)
        username = args[3];
    if (args.length > 4)
        password = args[4];
    console.LogInfo("User '" + username + "' connecting to " + address + ":" + port + " using " + protocol);
}

var scene = null;
framework.Scene().SceneCreated.connect(function(newScene)
{
    scene = newScene;
    scene.EntityCreated.connect(OnEntityCreated);
});

function OnEntityCreated(entity, change)
{
    if (!server.IsRunning() && entity.name == "Avatar" + client.connectionId)
    {
        avatarEntityId = entity.id;
        frame.Updated.connect(ProduceRandomMovement);
        var t = RandomTimeToLive();
        print("OnEntityCreated: disconnecting from the server in " + t + " seconds.");
        frame.DelayedExecute(t).Triggered.connect(Disconnect);
    }
}

function ProduceRandomMovement()
{
    if (!client.IsConnected())
    {
        frame.Updated.disconnect(ProduceRandomMovement);
        return;
    }
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

function Disconnect()
{
//    frame.Updated.disconnect(ProduceRandomMovement);
    client.Logout();
    var t = RandomReconnectionTime();
    print("Disconnect: reconnecting in " + t + " seconds.");
    frame.DelayedExecute(t).Triggered.connect(Connect);
}

function Connect()
{
    client.Login(address, port, username, password, protocol);
}
