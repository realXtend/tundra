// An object camera script. Enables object look using Alt+Click, rotating around object and zooming in and out

// Todo: fix this for flexible coordinate axes, or remove

var rotate_sensitivity = 0.5;
var camera_distance = 7.0;
var scroll_speed = 0.8;
var alt_key_pressed = false;
var last_clicked;
var zooming = false;
var global_transform;

if (!me.GetComponent("EC_Camera"))
{
    var camera = me.GetOrCreateComponent("EC_Camera");
    var inputmapper = me.GetOrCreateComponent("EC_InputMapper");
    var placeable = me.GetOrCreateComponent("EC_Placeable");
    var soundlistener = me.GetOrCreateComponent("EC_SoundListener");
    soundlistener.active = true;    

    camera.AutoSetPlaceable();

    var avatarcameraentity = scene.GetEntityByNameRaw("AvatarCamera");
    if (!avatarcameraentity)
        camera.SetActive();
    
    var transform = placeable.transform;
    transform.rot.x = 90;
    placeable.transform = transform;

    frame.Updated.connect(Update);
    inputmapper.contextPriority = 101;
    inputmapper.takeMouseEventsOverQt = true;
    input.TopLevelInputContext().MouseLeftPressed.connect(mouseLeftPress);
    input.TopLevelInputContext().MouseScroll.connect(mouseScroll);
    input.TopLevelInputContext().KeyPressed.connect(keyPress);
    input.TopLevelInputContext().KeyReleased.connect(keyRelease);

    me.Action("MouseLookX").Triggered.connect(HandleMouseLookX);
    me.Action("MouseLookY").Triggered.connect(HandleMouseLookY);
}


function Update(frametime)
{
    var camera = me.GetComponent("EC_Camera");
    if (camera.IsActive() == false || last_clicked == null)
        return;

    var placeable = me.GetComponent("EC_Placeable");

    if (zooming)
    {
        cameraZoom();
    }

    if (camera.IsActive() == true && last_clicked != null)
    {
        placeable.LookAt(last_clicked.placeable.transform.pos);
    }

}


function HandleMouseLookX(param)
{
    var camera = me.GetComponent("EC_Camera");
    if (camera.IsActive() == false || last_clicked == null)
        return;

    if (!last_clicked)
	return;

    var move = parseInt(param);
    var placeable = me.GetComponent("EC_Placeable");

    var newtransform = placeable.transform;
    newtransform.rot.z -= rotate_sensitivity * move;

    newtransform.pos.y = camera_distance * Math.sin(newtransform.rot.z * Math.PI/180) + last_clicked.placeable.transform.pos.y;
    newtransform.pos.x = camera_distance * Math.cos(newtransform.rot.z * Math.PI/180) + last_clicked.placeable.transform.pos.x;

    placeable.transform = newtransform;
    placeable.LookAt(last_clicked.placeable.transform.pos);
}


function HandleMouseLookY(param)
{
    var camera = me.GetComponent("EC_Camera");
    if (camera.IsActive() == false || last_clicked == null)
        return;

    var move = parseInt(param);
    var placeable = me.GetComponent("EC_Placeable");

    var newtransform = placeable.transform;
    newtransform.rot.x -= rotate_sensitivity * move;

    newtransform.pos.z = camera_distance * Math.cos(newtransform.rot.x * Math.PI/180) + last_clicked.placeable.transform.pos.z;

    if ((newtransform.pos.z > camera_distance && move < 0) || (newtransform.pos.z < -camera_distance + 1 && move > 0)) 
        return;

    placeable.transform = newtransform;
    placeable.LookAt(last_clicked.placeable.transform.pos);
}

function mouseLeftPress(event)
{
    if (alt_key_pressed == true)
    {
        var raycastResult = renderer.Raycast(event.x, event.y);
        if (raycastResult.entity !== null)
        {
            var entityclicked = scene.GetEntityRaw(raycastResult.entity.id);
            var objectcameraentity = scene.GetEntityByNameRaw("ObjectCamera");
            var avatarcameraentity = scene.GetEntityByNameRaw("AvatarCamera");
            if ((objectcameraentity == null) || (avatarcameraentity == null))
                return;
            var objectcamera = objectcameraentity.ogrecamera;
            var avatarcamera = avatarcameraentity.ogrecamera;

            if (objectcamera.IsActive() && last_clicked == entityclicked)
                return;

            last_clicked = entityclicked;
            var cameratransform = objectcameraentity.placeable.transform;

            global_transform = entityclicked.placeable.transform;
            global_transform.pos.y = camera_distance * Math.sin(cameratransform.rot.z * Math.PI/180) + last_clicked.placeable.transform.pos.y;
            global_transform.pos.x = camera_distance * Math.cos(cameratransform.rot.z * Math.PI/180) + last_clicked.placeable.transform.pos.x;
            global_transform.pos.z = camera_distance * Math.cos(cameratransform.rot.x * Math.PI/180) + last_clicked.placeable.transform.pos.z;

            zooming = true;

            if (avatarcamera.IsActive())
                objectcamera.SetActive();
        }
    }
}


function mouseScroll(event)
{
    var camera = me.GetComponent("EC_Camera");
    if (camera.IsActive() == false)
        return;

    var placeable = me.GetComponent("EC_Placeable");

    var delta;

    if (event.relativeZ < 0)
        delta = -25;
    else if (event.relativeZ > 0)
        delta = 25;

    var zoomed = false;

    var pos = new Vector3df()
    pos = placeable.transform.pos;

    var point = new Vector3df();
    point = last_clicked.placeable.transform.pos;

    var dir = new Vector3df();
    dir.x = point.x - pos.x;
    dir.y = point.y - pos.y;
    dir.z = point.z - pos.z;

    var distance = new Vector3df();
    distance = dir;

    var acceleration = 0.01;
    dir.x *= delta * acceleration;
    dir.y *= delta * acceleration;
    dir.z *= delta * acceleration;

    var min = 5;
    var max = 100;
    var distance_length = Math.sqrt(distance.x*distance.x + distance.y*distance.y + distance.z*distance.z);
    var dir_length = Math.sqrt(dir.x*dir.x + dir.y*dir.y + dir.z*dir.z);

    if (delta>0 && (distance_length+dir_length > min))
    {
        zoomed = true;
        camera_distance -= scroll_speed;
    }
    if (delta<0 && (distance_length+dir_length < max))
    {
        zoomed = true;
        camera_distance += scroll_speed;
    }
    if (zoomed)
    {
        var newtransform = placeable.transform;
        newtransform.pos.y = camera_distance * Math.sin(newtransform.rot.z * Math.PI/180) + last_clicked.placeable.transform.pos.y;
        newtransform.pos.x = camera_distance * Math.cos(newtransform.rot.z * Math.PI/180) + last_clicked.placeable.transform.pos.x;
        newtransform.pos.z = camera_distance * Math.cos(newtransform.rot.x * Math.PI/180) + last_clicked.placeable.transform.pos.z;
        placeable.transform = newtransform;
    }
}

function keyPress(event)
{
    if (event.HasAltModifier() == true)
    {
        alt_key_pressed = true;
        return;
    }

    var avatarcameraentity = scene.GetEntityByNameRaw("AvatarCamera");
    var freelookcameraentity = scene.GetEntityByNameRaw("FreeLookCamera");
    if (avatarcameraentity == null) // || freelookcameraentity == null) 
        return;

    var avatarcamera = avatarcameraentity.ogrecamera;
    var freelookcamera = freelookcameraentity.ogrecamera;

    if (freelookcamera.IsActive())
        return;

    //XXX \todo BUG: steals activity from any other camera, this can not be done!
    //this cam is now disabled alltogether, fix this if wanna enable it
    if (!avatarcamera.IsActive())
    {
        if (event.sequence.toString() == "W" 
         || event.sequence.toString() == "S" 
         || event.sequence.toString() == "A" 
         || event.sequence.toString() == "D"
         || event.sequence.toString() == "Left"
         || event.sequence.toString() == "Up"
         || event.sequence.toString() == "Right"
         || event.sequence.toString() == "Down")
            avatarcamera.SetActive();
    }
}

function keyRelease(event)
{
    alt_key_pressed = false;
}

function cameraZoom()
{
    var entityplaceable = last_clicked.GetComponent("EC_Placeable");
    var cameraentity = scene.GetEntityByNameRaw("ObjectCamera");
    if (cameraentity == null)
        return;
    var cameraplaceable = cameraentity.GetComponent("EC_Placeable");
    var cameratransform = cameraplaceable.transform;

    var dir = new Vector3df();
    dir.x = global_transform.pos.x - (camera_distance * Math.cos(cameratransform.rot.z * Math.PI/180) + cameratransform.pos.x);
    dir.y = global_transform.pos.y - (camera_distance * Math.sin(cameratransform.rot.z * Math.PI/180) + cameratransform.pos.y);
    dir.z = global_transform.pos.z - (camera_distance * Math.cos(cameratransform.rot.x * Math.PI/180) + cameratransform.pos.z);

    var u = Math.sqrt(Math.pow(dir.x, 2) + Math.pow(dir.y, 2) + Math.pow(dir.z, 2));
    var dir_unit = new Vector3df();
    dir_unit.x = dir.x / u;
    dir_unit.y = dir.y / u;
    dir_unit.z = dir.z / u;

    dir_unit.x *= 0.1;
    dir_unit.y *= 0.1;
    dir_unit.z *= 0.1;

    cameratransform.pos.x += dir_unit.x;
    cameratransform.pos.y += dir_unit.y;
    cameratransform.pos.z += dir_unit.z;

    if (u > camera_distance)
    {
        cameraplaceable.transform = cameratransform;
        cameraplaceable.LookAt(cameratransform.pos);
    }
    else
    {
        zooming = false;
    }
}
