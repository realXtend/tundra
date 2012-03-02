// Avatar addon script. Adds wave gesture & sitting -features

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
    // Connect actions. These come from the client inputmapper
    // Note: use "Addon" prefix in the actions, so that we don't confuse with other actions
    this.me.Action("AddonWave").Triggered.connect(this, this.ServerHandleWave);
    this.me.Action("AddonSit").Triggered.connect(this, this.ServerHandleSit);
}

ExampleAvatarAddon.prototype.ClientInitialize = function()
{
    // Initialization is only necessary for own avatar. Do nothing with others' avatars
    if (this.me.name == "Avatar" + client.connectionId) {
        ownAvatar = true;

        // Connect keys to the inputmapper actions
        // If simpleavatar.js has not yet run, create the inputmapper here
        var inputmapper = this.me.GetOrCreateComponent("EC_InputMapper", 2, false);
        inputmapper.modifiersEnabled = true; // Don't ignore modifiers, but take them into account (e.g. don't trigger on Shift+E or Ctrl+E, but require a plain E).
        inputmapper.suppressKeyEvents = true; // Suppress the handled 'Q' and 'E' from going further.
        inputmapper.RegisterMapping("Q", "AddonWave()", 1); // 1 = Keypress
        inputmapper.RegisterMapping("E", "AddonSit()", 1); // 1 = Keypress
    }
}

ExampleAvatarAddon.prototype.ServerHandleWave = function()
{
    // Play the wave animation by using the AnimationController component's action PlayAnimAutoStop
    // (parameters: animation name, fade-in time)
    // Executiontype 7 (client, server & peers) means it gets replicated to all
    // AutoStop means that the animation is faded out automatically when finished, so that it doesn't disturb other animations
    this.me.Exec(7, "PlayAnimAutoStop", "Wave", 0.25);
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
    if (this.sitting)
        this.me.Exec(7, "PlayAnim", "SitOnGround", 0.25);
    else
        this.me.Exec(7, "StopAnim", "SitOnGround", 0.25);
}
