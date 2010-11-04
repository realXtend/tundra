// Avatar application. Will handle switching logic between avatar & freelook camera (clientside), and
// spawning avatars for clients (serverside). Note: this is not a startup script, but is meant to be
// placed in an entity in a scene that wishes to implement avatar functionality.

me.Action("ToggleCamera").Triggered.connect(HandleToggleCamera);

// Todo: do this clientside only
var inputmapper = me.GetOrCreateComponentRaw("EC_InputMapper");
inputmapper.SetTemporary(true);
inputmapper.contextPriority = 102;
inputmapper.RegisterMapping("Ctrl+Tab", "ToggleCamera", 1);

function HandleToggleCamera()
{
    // For camera switching to work, must have both the freelookcamera & avatarcamera in the scene
    var freelookcameraentity = scene.GetEntityByNameRaw("FreeLookCamera");
    var avatarcameraentity = scene.GetEntityByNameRaw("AvatarCamera");
    if ((freelookcameraentity == null) || (avatarcameraentity == null))
        return;
    var freelookcamera = freelookcameraentity.GetComponentRaw("EC_OgreCamera");
    var avatarcamera = avatarcameraentity.GetComponentRaw("EC_OgreCamera");

    if (avatarcamera.IsActive())
    {
        freelookcameraentity.GetComponentRaw("EC_Placeable").transform = avatarcameraentity.GetComponentRaw("EC_Placeable").transform
        freelookcamera.SetActive();
    }
    else
    {
        avatarcamera.SetActive();
    }
}
