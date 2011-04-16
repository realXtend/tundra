var inputContext = input.RegisterInputContextRaw("fpsMouseLook", 150);
inputContext.SetTakeMouseEventsOverQt(true);
inputContext.MouseEventReceived.connect(MouseEvent);

function MouseEvent(event)
{
  if (event.IsRightButtonDown() && input.IsMouseCursorVisible())
    input.SetMouseCursorVisible(false);
  else if (!event.IsRightButtonDown() && !input.IsMouseCursorVisible())
    input.SetMouseCursorVisible(true);
}

