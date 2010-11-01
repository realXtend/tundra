// A freelook camera script. Upon run, creates necessary components if they don't exist yet, and hooks to the InputMapper's
// input context to process camera movement (WASD + mouse)

var rotate_sensitivity = 0.3
var move_sensitivity = 30.0
var motion_z = 0
var motion_x = 0

if (!me.HasComponent("EC_OgreCamera"))
{
    // Create components & setup default position/lookat for the camera, mimicing RexLogicModule::CreateOpenSimViewerCamera()
    var camera = me.GetOrCreateComponentRaw("EC_OgreCamera");
    var inputmapper = me.GetOrCreateComponentRaw("EC_InputMapper");
    var placeable = me.GetOrCreateComponentRaw("EC_Placeable");
    var soundlistener = me.GetOrCreateComponentRaw("EC_SoundListener");

    camera.AutoSetPlaceable();
    camera.SetActive();
    
    var transform = placeable.transform;
    transform.rot.x = 90;
    placeable.transform = transform;

    // Hook to update tick
    frame.Updated.connect(Update);
    // Register press & release action mappings to the inputmapper, use higher priority than RexMovementInput to be sure
    inputmapper.contextPriority = 101;
    inputmapper.takeMouseEventsOverQt = true;
    inputmapper.RegisterMapping("W", "Move(forward)", 1);
    inputmapper.RegisterMapping("S", "Move(back)", 1);
    inputmapper.RegisterMapping("A", "Move(left)", 1);
    inputmapper.RegisterMapping("D", "Move(right)", 1);
    inputmapper.RegisterMapping("W", "Stop(forward)", 3);
    inputmapper.RegisterMapping("S", "Stop(back)", 3);
    inputmapper.RegisterMapping("A", "Stop(left)", 3);
    inputmapper.RegisterMapping("D", "Stop(right)", 3);
    // Connect actions
    me.Action("Move").Triggered.connect(HandleMove);
    me.Action("Stop").Triggered.connect(HandleStop);
    me.Action("MouseLookX").Triggered.connect(HandleMouseLookX);
    me.Action("MouseLookY").Triggered.connect(HandleMouseLookY);
}

function Update(frametime)
{
    var placeable = me.GetComponentRaw("EC_Placeable");
    if (motion_z != 0)
    {
        var motionvec = new Vector3df();
        motionvec.z = -motion_z * move_sensitivity * frametime;
        placeable.TranslateRelative(motionvec);
    }
    if (motion_x != 0)
    {
        var motionvec = new Vector3df();
        motionvec.x = motion_x * move_sensitivity * frametime;
        placeable.TranslateRelative(motionvec);
    }
}

function HandleMove(param)
{
    if (param == "forward")
        motion_z = 1;
    if (param == "back")
        motion_z = -1;
    if (param == "right")
        motion_x = 1;
    if (param == "left")
        motion_x = -1;
}

function HandleStop(param)
{
    if ((param == "forward") && (motion_z == 1))
        motion_z = 0;
    if ((param == "back") && (motion_z == -1))
        motion_z = 0;
    if ((param == "right") && (motion_x == 1))
        motion_x = 0;
    if ((param == "left") && (motion_x == -1))
        motion_x = 0;
}

function HandleMouseLookX(param)
{
    var move = parseInt(param);
    var placeable = me.GetComponentRaw("EC_Placeable");
    var newtransform = placeable.transform;
    newtransform.rot.z -= rotate_sensitivity * move;
    placeable.transform = newtransform;
}

function HandleMouseLookY(param)
{
    var move = parseInt(param);
    var placeable = me.GetComponentRaw("EC_Placeable");
    var newtransform = placeable.transform;
    newtransform.rot.x -= rotate_sensitivity * move;
    placeable.transform = newtransform;
}
