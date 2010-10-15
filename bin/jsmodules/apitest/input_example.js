print("Loading input test script.");

function mouseMove(event)
{
    print("Mouse move event x:" + event.x + " y:" + event.y);
}

function mousePress(event)
{
    print("Mouse pressed at x:" + event.x + " y:" + event.y + ".");
}

//Not working.
function scroll(event)
{
    print("Mouse scroll z:" + event.z);
}

function keyPressed(event)
{
    print(event.sequence.toString() + " Key pressed.");
}

//Not working.
function keyDown(event)
{
    print(event.sequence.toString() + " Key down.");
}

function keyUp(event)
{
    print(event.sequence.toString() + " Key Up.");
}

input.TopLevelInputContext().MouseMove.connect(mouseMove);
input.TopLevelInputContext().MouseLeftPressed.connect(mousePress);
input.TopLevelInputContext().MouseMiddlePressed.connect(mousePress);
input.TopLevelInputContext().MouseRightPressed.connect(mousePress);
input.TopLevelInputContext().MouseScroll.connect(scroll); //Not working!

input.TopLevelInputContext().KeyPressed.connect(keyPressed);
input.TopLevelInputContext().KeyDown.connect(keyDown); //Not working!
input.TopLevelInputContext().KeyReleased.connect(keyUp);