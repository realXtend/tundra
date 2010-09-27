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

input.MouseMove.connect(mouseMove);
input.MouseLeftPressed.connect(mousePress);
input.MouseMiddlePressed.connect(mousePress);
input.MouseRightPressed.connect(mousePress);
input.MouseScroll.connect(scroll); //Not working!

input.KeyPressed.connect(keyPressed);
input.KeyDown.connect(keyDown); //Not working!
input.KeyReleased.connect(keyUp);