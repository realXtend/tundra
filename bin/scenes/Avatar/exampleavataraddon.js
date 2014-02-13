// For conditions of distribution and use, see copyright notice in LICENSE
// ExampleAvatarAddon.js - Avatar addon script. Adds wave gesture & sitting -features

// A simple walking avatar with physics & 1st/3rd person camera
function ExampleAvatarAddon(entity, comp)
{
    // Store the entity reference
    this.me = entity;

    // Sitting flag. Used only on the server
    this.sitting = false;
    // Own avatar flag. Checked on the client
    this.ownAvatar = false;

    // Check whether runs on server or client, and do different initialization based on that
    if (server.IsRunning()) {
        this.ServerInitialize();
    } else {
        this.ClientInitialize();
    }
}

ExampleAvatarAddon.prototype.ServerInitialize = function()
{
    // Connect to actions that come from the client.
    // Note: use "Addon" prefix in the actions, so that we don't confuse with other actions
    this.me.Action("AddonWave").Triggered.connect(this, this.ServerHandleWave);
    this.me.Action("AddonSit").Triggered.connect(this, this.ServerHandleSit);
}

ExampleAvatarAddon.prototype.ClientInitialize = function()
{
    // Initialization is only necessary for own avatar. Do nothing with others' avatars
    if (this.me.name == "Avatar" + client.connectionId)
    {
        ownAvatar = true;
        engine.ImportExtension("qt.core");
        // Handle key presses using InputContext.
        var ic = input.RegisterInputContextRaw("ExampleAvatarAddon", 90);
        ic.KeyPressed.connect(this, this.ClientHandleKeyPress);
    }
}

SimpleAvatar.prototype.OnScriptObjectDestroyed = function()
{
    // Remember to unregister our input context.
    input.UnregisterInputContextRaw("ExampleAvatarAddon");
}

ExampleAvatarAddon.prototype.ClientHandleKeyPress = function(e)
{
    if (e.modifiers == 0) // Require plain key without modifiers.
    {
        if (e.keyCode == Qt.Key_E)
        {
            this.me.Exec(EntityAction.Server, "AddonSit");
            e.Suppress(); // Suppress the handled press going further.
        }
        else if (e.keyCode == Qt.Key_Q)
        {
            this.me.Exec(EntityAction.Server, "AddonWave");
            e.Suppress();
        }
    }
}

ExampleAvatarAddon.prototype.ServerHandleWave = function()
{
    // Play the wave animation by using the AnimationController component's action PlayAnimAutoStop
    // (parameters: animation name, fade-in time)
    // AutoStop means that the animation is faded out automatically when finished, so that it doesn't disturb other animations
    this.me.Exec(EntityAction.Local | EntityAction.Server | EntityAction.Peers, "PlayAnimAutoStop", "Wave", 0.25);
}

ExampleAvatarAddon.prototype.ServerHandleSit = function()
{
    // Toggle sitting state
    this.sitting = !this.sitting;

    // Use the attributes in the avatar's dynamic component to disable simpleavatar.js features when we're sitting:
    // - walking
    // - flying
    // - default animations
    var attrs = this.me.dynamiccomponent;
    attrs.SetAttribute("enableWalk", !this.sitting);
    attrs.SetAttribute("enableFly", !this.sitting);
    attrs.SetAttribute("enableAnimation", !this.sitting);

    // Then either play or stop the sitting animation, depending on the state
    // Like with the Wave gesture animation, use an AnimationController action that is replicated to all
    this.me.Exec(EntityAction.Local | EntityAction.Server | EntityAction.Peers,
        this.sitting ? "PlayAnim" : "StopAnim", "SitOnGround", 0.25);
}
