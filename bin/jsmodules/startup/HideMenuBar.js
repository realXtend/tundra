// Note: use --nocentralwidget option with this to prevent device loss & camera stutter when hiding/showing menubar

var inputContext;

var hideThreshold = 20;

if (!framework.IsHeadless())
{
    engine.ImportExtension("qt.core");
    engine.ImportExtension("qt.gui");

    inputContext = input.RegisterInputContextRaw("HideMenuBar", 9999);
    inputContext.MouseMove.connect(HandleMouseMove);
    inputContext.SetTakeMouseEventsOverQt(true);

    ui.MainWindow().menuBar().visible = false;
}

function HandleMouseMove(mouseEvent)
{
    if (mouseEvent.y < hideThreshold)
        ui.MainWindow().menuBar().visible = true;
    else
        ui.MainWindow().menuBar().visible = false;
}