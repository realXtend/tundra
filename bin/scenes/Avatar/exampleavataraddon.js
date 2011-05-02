// Avatar addon script. Adds wave gesture & sitting -features

// Check whether runs on server or client, and do different initialization based on that
if (server.IsRunning()) {
    ServerInitialize();
} else {
    ClientInitialize();
}

// Sitting flag. Used only on the server
var sitting = false;
// Own avatar flag. Checked on the client
var own_avatar = false;

function ServerInitialize()
{
    // Connect actions. These come from the client inputmapper
    // Note: use "Addon" prefix in the actions, so that we don't confuse with other actions
    me.Action("AddonWave").Triggered.connect(ServerHandleWave);
    me.Action("AddonSit").Triggered.connect(ServerHandleSit);
}

function ClientInitialize()
{
    // Initialization is only necessary for own avatar. Do nothing with others' avatars
    if (me.GetName() == "Avatar" + client.GetConnectionID()) {
        own_avatar = true;

        // Connect keys to the inputmapper actions
        // If simpleavatar.js has not yet run, create the inputmapper here
        var inputmapper = me.GetOrCreateComponent("EC_InputMapper", 2, false);
        inputmapper.RegisterMapping("Q", "AddonWave()", 1); // 1 = Keypress
        inputmapper.RegisterMapping("E", "AddonSit()", 1); // 1 = Keypress
    }
}

function ServerHandleWave()
{
    // Play the wave animation by using the AnimationController component's action PlayAnimAutoStop
    // (parameters: animation name, fade-in time)
    // Executiontype 7 (client, server & peers) means it gets replicated to all
    // AutoStop means that the animation is faded out automatically when finished, so that it doesn't disturb other animations
    me.Exec(7, "PlayAnimAutoStop", "Wave", 0.25);
}

function ServerHandleSit()
{
    // Toggle sitting state
    sitting = !sitting;

    // Use the attributes in the avatar's dynamic component to disable simpleavatar.js features when we're sitting:
    // - walking
    // - flying
    // - default animations
    var attrs = me.dynamiccomponent;
    attrs.SetAttribute("enableWalk", !sitting);
    attrs.SetAttribute("enableFly", !sitting);
    attrs.SetAttribute("enableAnimation", !sitting);

    // Then either play or stop the sitting animation, depending on the state
    // Like with the Wave gesture animation, use an AnimationController action that is replicated to all
    if (sitting)
        me.Exec(7, "PlayAnim", "SitOnGround", 0.25);
    else
        me.Exec(7, "StopAnim", "SitOnGround", 0.25);
}
