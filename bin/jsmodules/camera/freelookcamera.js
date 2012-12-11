// A freelook camera script. Upon run, creates necessary components if they don't exist yet, and hooks to the InputMapper's
// input context to process camera movement (WASD + mouse)

// Global application data
var _g =
{
    connected : false,
    rotate :
    {
        sensitivity : 0.3
    },
    move :
    {
        sensitivity : 30.0,
        amount : new float3(0,0,0)
    },
    motion : new float3(0,0,0),

    // Android specific
    isAndroid : application.platform == "android"
};

function Initialize()
{
    // Connect to camera changed signal. This disconnects frame updates when camera is not active
    // and connects back when camera is active. This is a optimization not to have any update processing
    // when the camera is not active. Note that for clients this saves 2x fps updates as the empty "TundraServer"
    // scene would think it is active even when the scene is not visible!
    renderer.MainCameraChanged.connect(ActiveCameraChanged);

    // Create components & setup default position/lookat for the camera
    var camera = me.GetOrCreateComponent("EC_Camera");
    var inputmapper = me.GetOrCreateComponent("EC_InputMapper");
    var placeable = me.GetOrCreateComponent("EC_Placeable");
    var soundlistener = me.GetOrCreateComponent("EC_SoundListener");

    // Co-operate with the AvatarApplication: if AvatarCamera already exists, do not activate the freelookcamera right now
    var avatarcameraentity = scene.GetEntityByName("AvatarCamera");
    if (!avatarcameraentity)
    {
        camera.SetActive(); // This will connect frame updates for this script.
        soundlistener.active = true;
    }

    // Register press & release action mappings to the inputmapper, use higher priority than RexMovementInput to be sure
    inputmapper.contextPriority = 101;
    inputmapper.takeMouseEventsOverQt = true;
    inputmapper.modifiersEnabled = false;
    inputmapper.RegisterMapping("W", "Move(forward)", 1);
    inputmapper.RegisterMapping("S", "Move(back)", 1);
    inputmapper.RegisterMapping("A", "Move(left)", 1);
    inputmapper.RegisterMapping("D", "Move(right)", 1);
    inputmapper.RegisterMapping("Space", "Move(up)", 1);
    inputmapper.RegisterMapping("C", "Move(down)", 1);
    inputmapper.RegisterMapping("W", "Stop(forward)", 3);
    inputmapper.RegisterMapping("S", "Stop(back)", 3);
    inputmapper.RegisterMapping("A", "Stop(left)", 3);
    inputmapper.RegisterMapping("D", "Stop(right)", 3);
    inputmapper.RegisterMapping("Space", "Stop(up)", 3);
    inputmapper.RegisterMapping("C", "Stop(down)", 3);
    inputmapper.RegisterMapping("Up", "Move(forward)", 1);
    inputmapper.RegisterMapping("Down", "Move(back)", 1);
    inputmapper.RegisterMapping("Left", "Move(left)", 1);
    inputmapper.RegisterMapping("Right", "Move(right)", 1);
    inputmapper.RegisterMapping("Up", "Stop(forward)", 3);
    inputmapper.RegisterMapping("Down", "Stop(back)", 3);
    inputmapper.RegisterMapping("Left", "Stop(left)", 3);
    inputmapper.RegisterMapping("Right", "Stop(right)", 3);
    inputmapper.enabled = true;

    // Connect actions
    me.Action("Move").Triggered.connect(HandleMove);
    me.Action("Stop").Triggered.connect(HandleStop);
    me.Action("MouseLookX").Triggered.connect(HandleMouseLookX);
    me.Action("MouseLookY").Triggered.connect(HandleMouseLookY);

    // Connect gestures
    var inputContext = inputmapper.GetInputContext();
    if (!_g.isAndroid)
    {
        if (inputContext.GestureStarted && inputContext.GestureUpdated)
        {
            inputContext.GestureStarted.connect(GestureStarted);
            inputContext.GestureUpdated.connect(GestureUpdated);
        }
    }
}

function IsCameraActive()
{
    return (me.camera == null ? false : me.camera.IsActive());
}

function Update(frametime)
{
    profiler.BeginBlock("FreeLookCamera_Update");
    if (!IsCameraActive())
    {
        profiler.EndBlock();
        return;
    }

    // Android movement logic: move forward if more than 1 finger on screen
    if (_g.isAndroid)
        _g.move.amount.z = input.NumTouchPoints() > 1 ? -1 : 0; 

    if (_g.move.amount.x == 0 && _g.move.amount.y == 0 && _g.move.amount.z == 0)
    {
        profiler.EndBlock();
        return;
    }

    _g.motion.x = _g.move.amount.x * _g.move.sensitivity * frametime;
    _g.motion.y = _g.move.amount.y * _g.move.sensitivity * frametime;
    _g.motion.z = _g.move.amount.z * _g.move.sensitivity * frametime;

    _g.motion = me.placeable.Orientation().Mul(_g.motion);
    me.placeable.SetPosition(me.placeable.Position().Add(_g.motion));

    profiler.EndBlock();
}

function HandleMove(param)
{
    if (!IsCameraActive())
        return;

    if (param == "forward")
        _g.move.amount.z = -1;
    else if (param == "back")
        _g.move.amount.z = 1;
    else if (param == "right")
        _g.move.amount.x = 1;
    else if (param == "left")
        _g.move.amount.x = -1;
    else if (param == "up")
        _g.move.amount.y = 1;
    else if (param == "down")
        _g.move.amount.y = -1;
}

function HandleStop(param)
{
    if (!IsCameraActive())
        return;

    if ((param == "forward") && (_g.move.amount.z == -1))
        _g.move.amount.z = 0;
    else if ((param == "back") && (_g.move.amount.z == 1))
        _g.move.amount.z = 0;
    else if ((param == "right") && (_g.move.amount.x == 1))
        _g.move.amount.x = 0;
    else if ((param == "left") && (_g.move.amount.x == -1))
        _g.move.amount.x = 0;
    else if ((param == "up") && (_g.move.amount.y == 1))
        _g.move.amount.y = 0;
    else if ((param == "down") && (_g.move.amount.y == -1))
        _g.move.amount.y = 0;
}

function HandleMouseLookX(param)
{
    if (!IsCameraActive())
        return;

    var transform = me.placeable.transform;
    transform.rot.y -= _g.rotate.sensitivity * parseInt(param);
    me.placeable.transform = transform;
}

function HandleMouseLookY(param)
{
    if (!IsCameraActive())
        return;

    var transform = me.placeable.transform;
    transform.rot.x -= _g.rotate.sensitivity * parseInt(param);
    if (transform.rot.x > 90.0)
        transform.rot.x = 90.0;
    if (transform.rot.x < -90.0)
        transform.rot.x = -90.0;
    me.placeable.transform = transform;
}

function GestureStarted(gestureEvent)
{
        if (!IsCameraActive())
            return;
    
        if (gestureEvent.GestureType() == Qt.TapAndHoldGesture)
        {
            if (motion_z == 0)
                HandleMove("forward");
            else
                HandleStop("forward");
            gestureEvent.Accept();
        }
        else if (gestureEvent.GestureType() == Qt.PanGesture)
        {
            var offset = gestureEvent.Gesture().offset.toPoint();
            HandleMouseLookX(offset.x());
            HandleMouseLookY(offset.y());
            gestureEvent.Accept();
        }
}

function GestureUpdated(gestureEvent)
{
        if (!IsCameraActive())
            return;

        if (gestureEvent.GestureType() == Qt.PanGesture)
        {
            var delta = gestureEvent.Gesture().delta.toPoint();
            HandleMouseLookX(delta.x());
            HandleMouseLookY(delta.y());
            gestureEvent.Accept();
        }
}

function OnScriptDestroyed() 
{
    renderer.MainCameraChanged.disconnect(ActiveCameraChanged);
}

function DisconnectApplication()
{
    if (_g.connected)
    {
        // Disconnect frame updates and enabled inputmapper + soundlistener
        frame.Updated.disconnect(Update);
        if (me.inputmapper != null)
            me.inputmapper.enabled = false;
        if (me.soundlistener != null)
            me.soundlistener.active = false;
        _g.connected = false;

        _g.move.amount.x = 0;
        _g.move.amount.y = 0;
        _g.move.amount.z = 0;
    }
}

function ConnectApplication()
{
    if (!_g.connected)
    {
        // Connect frame updates and enabled inputmapper + soundlistener
        frame.Updated.connect(Update);
        if (me.inputmapper != null)
            me.inputmapper.enabled = true;
        if (me.soundlistener != null)
            me.soundlistener.active = true;
        _g.connected = true;

        _g.move.amount.x = 0;
        _g.move.amount.y = 0;
        _g.move.amount.z = 0;
    }
}

function ActiveCameraChanged(cameraEnt)
{
    if (cameraEnt == null)
        return;

    if (cameraEnt == me)
        ConnectApplication();
    else
        DisconnectApplication();
}

if (!framework.IsHeadless())
    Initialize();
