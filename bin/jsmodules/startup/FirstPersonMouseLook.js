var inputContext = input.RegisterInputContextRaw("fpsMouseLook", 150);
inputContext.SetTakeMouseEventsOverQt(true);
inputContext.MouseEventReceived.connect(MouseEvent);

// Returns user's avatar entity if connected.
function FindUserAvatar()
{
    var scene = framework.Scene().MainCameraScene();
    if (scene && client.IsConnected())
        return scene.GetEntityByName("Avatar" + client.GetConnectionID());
    else
        return null;
}

// Hides and shows mourse cursor when RMB is pressed and released
function MouseEvent(event)
{
    // Never show mouse cursor, if avatar camera is active and in first-person mode.
    var avatarCameraActiveInFps = false;
    var userAvatar = FindUserAvatar();
    var scene = framework.Scene().MainCameraScene();
    if (userAvatar && scene)
    {
        var avatarCamera = scene.GetEntityByName("AvatarCamera");
        if (avatarCamera && avatarCamera.camera)
            avatarCameraActiveInFps = avatarCamera.camera.IsActive() && userAvatar.dynamiccomponent.GetAttribute("cameraDistance") < 0;
    }

    // RMB pressed
    if (event.GetEventType() == 3 && event.GetMouseButton() == 2 && input.IsMouseCursorVisible())
        input.SetMouseCursorVisible(false);
    // RMB released
    else if (event.GetEventType() == 4 && event.GetMouseButton() == 2 && !input.IsMouseCursorVisible())
        if (!avatarCameraActiveInFps)
            input.SetMouseCursorVisible(true);
}
