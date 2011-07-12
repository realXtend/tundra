var inputContext = input.RegisterInputContextRaw("fpsMouseLook", 150);
inputContext.SetTakeMouseEventsOverQt(true);
inputContext.MouseEventReceived.connect(MouseEvent);

function MouseEvent(event)
{
    // Press RMB
    if (event.GetEventType() == 3 && event.GetMouseButton() == 2)
    {
        if (input.IsMouseCursorVisible())
            input.SetMouseCursorVisible(false);
    }
    // Release RMB
    if (event.GetEventType() == 4 && event.GetMouseButton() == 2)
    {
        if (!input.IsMouseCursorVisible())
            input.SetMouseCursorVisible(true);
    }
}

