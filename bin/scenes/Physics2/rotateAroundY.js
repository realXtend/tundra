
print ("rotateAroundY.js running")

var rotationY = 0.0
var R = 5.0

// first get the light distance from the Y axis (it will be used to rotate the light)
var light = scene.GetEntityByName("MainLight");
var trans = light.placeable.transform;
R = Math.sqrt(trans.pos.x*trans.pos.x + trans.pos.z*trans.pos.z);
frame.DelayedExecute(0.5).Triggered.connect(this, this.rotateAroundY);

// And then initialize inputmapper to grab left mouse
// The boulder gets mass when clicked
var inputmapper = this.me.GetOrCreateComponent("InputMapper", 2, false);
inputmapper.contextPriority = 100;
inputmapper.takeMouseEventsOverQt = true;
inputmapper.modifiersEnabled = false;
inputmapper.executionType = 1; // Execute actions locally
// Connect left mouse button
var inputContext = inputmapper.GetInputContext();
inputContext.MouseLeftPressed.connect(this, this.HandleMouseLeftPressed);

// raycast tied to left mouse button
function HandleMouseLeftPressed(event)
{
    var raycastResult = scene.ogre.Raycast(event.x, event.y, 0xffffffff);
    if(raycastResult.entity != null)
    {
        if (raycastResult.entity.name == "Boulder")
        {
            raycastResult.entity.rigidbody.mass = 100.0
        }
    }
}

// Circular light animation
function rotateAroundY()
{
    //print("Rotating around y: " + rotationY);

    var light = scene.GetEntityByName("MainLight");
    var trans = light.placeable.transform;
    trans.pos.x = R*Math.sin(rotationY/360.0*2*3.1415)
    trans.pos.z = R*Math.cos(rotationY/360.0*2*3.1415)
    light.placeable.transform = trans;

    rotationY += 2.0;
    if (rotationY > 360.0) rotationY -= 360.0;

    frame.DelayedExecute(1.0/30.0).Triggered.connect(this, this.rotateAroundY);
}

