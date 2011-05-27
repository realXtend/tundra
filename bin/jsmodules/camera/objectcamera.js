// An object camera script. Enables object look using Alt+Click, rotating around object and zooming in and out
engine.ImportExtension("qt.core");
engine.ImportExtension("qt.gui");

var camera_distance = 7.0;
var last_clicked_pos = new Vector3df();
var zooming = false;
var global_transform;

var mouse_left_pressed = false;
var objectcameraentity;
var objectcamera_mode = false;
var last_camera = null;

var return_button = null;
var return_button_proxy = null;

function init_ui()
{
    if (!return_button)
    {
        return_button = new QPushButton("Return To Previous Camera");
        return_button.resize(190, 35);
        
        return_button_proxy = new UiProxyWidget(return_button); 
        ui.AddProxyWidgetToScene(return_button_proxy);
        return_button_proxy.windowFlags = 0;
        return_button_proxy.effect = 0;        

        var style = "QPushButton          { background-color: qlineargradient(spread:pad, x1:0, y1:0, x2:0, y2:1, stop:0 rgba(235, 235, 235, 150), stop:1 rgba(82, 82, 82, 150)); \
                                            font: 11pt \"Calibri\"; border: 1px solid grey; border-radius: 5px; color: rgb(250, 250, 250); } \
                     QPushButton::hover   { background-color: qlineargradient(spread:pad, x1:0, y1:0, x2:1, y2:1, stop:0 rgba(135, 135, 135, 150), stop:1 rgba(82, 82, 82, 150)); } \
                     QPushButton::pressed { color: rgb(220, 220, 220); background-color: qlineargradient(spread:pad, x1:0, y1:0, x2:0, y2:1, stop:0 rgba(228, 228, 228, 150), stop:1 rgba(82, 82, 82, 150)); }";
        return_button.styleSheet = style;
        return_button.setAttribute(Qt.WA_OpaquePaintEvent);

        scene_rect_changed(ui.GraphicsScene().sceneRect);
        ui.GraphicsScene().sceneRectChanged.connect(scene_rect_changed);

        return_button.clicked.connect(toggle_objectcamera);
    }
    return_button_proxy.visible = true;
    return_button.clearFocus();
}

if (!me.HasComponent("EC_OgreCamera"))
{
    var camera = me.GetOrCreateComponentRaw("EC_OgreCamera");
    //XXX var inputmapper = me.GetOrCreateComponentRaw("EC_InputMapper");
    var placeable = me.GetOrCreateComponentRaw("EC_Placeable");
    var soundlistener = me.GetOrCreateComponentRaw("EC_SoundListener");
    soundlistener.active = true;    

    camera.AutoSetPlaceable();

    //XXX var avatarcameraentity = scene.GetEntityByNameRaw("AvatarCamera");
    //XXX if (!avatarcameraentity)
    //XXX    camera.SetActive();

    var transform = placeable.transform;
    transform.rot.x = 90;
    placeable.transform = transform;

    //XXX inputmapper.contextPriority = 101;
    //XXX inputmapper.takeMouseEventsOverQt = true;

    input.TopLevelInputContext().MouseLeftPressed.connect(mouseLeftPress);
    input.TopLevelInputContext().MouseLeftReleased.connect(mouseLeftRelease);

    input.TopLevelInputContext().MouseScroll.connect(mouseScroll);
    input.TopLevelInputContext().MouseMove.connect(mouseMove);
}

function scene_rect_changed(rect)
{
    return_button_proxy.pos = new QPointF(rect.width()/2 - return_button.size.width()/2, 100);
}

function toggle_objectcamera()
{
    if (last_camera != null)
    {
        last_camera.SetActive();
    } else
    {
        var freelookcameraentity = scene.GetEntityByNameRaw("FreeLookCamera");
        if (freelookcameraentity)
        {
            var freelookcamera = freelookcameraentity.ogrecamera;
            if (freelookcamera)
                freelookcamera.SetActive();
        }
    }
    if (return_button_proxy != null)
    {
        return_button_proxy.visible = false;
    }
}

function mouseLeftPress(event)
{
    if (event.HasAltModifier())
    {
        mouse_left_pressed = true;
        if (!objectcamera_mode)
        {
            var raycastResult = renderer.Raycast(event.x, event.y);
            if (raycastResult.entity !== null)
            {
                if (raycastEntity.entity.HasComponent("EC_Selected"))
                    return;

                init_ui();
                last_clicked_pos = raycastResult.pos;
                objectcameraentity = scene.GetEntityByNameRaw("ObjectCamera");

                if (objectcameraentity == null)
                    return;
                var objectcamera = objectcameraentity.ogrecamera;

                if (objectcamera.IsActive())
                {
                    cameratransform = objectcameraentity.placeable.transform;
                } else
                {
                    var cameralist = scene.GetEntitiesWithComponentRaw("EC_OgreCamera");
                    for (var i = 0; i < cameralist.length; i++)
                    {
                        if (cameralist[i].ogrecamera.IsActive())
                        {
                            last_camera = cameralist[i].ogrecamera;
                            objectcameraentity.placeable.transform = cameralist[i].placeable.transform;
                        }
                    }
                    objectcamera.SetActive();
                }

                objectcamera_mode = true;
            }
        }
    }
}


function mouseScroll(event)
{
    if (objectcameraentity)
    {
        var zoomed = false;
        var min = 5;
        var max = 100;

        var transform = objectcameraentity.placeable.transform;       
        var pos = new Vector3df();
        pos.x = transform.pos.x;
        pos.y = transform.pos.y;
        pos.z = transform.pos.z;

        var pivot = last_clicked_pos;

        var dir = new Vector3df();
        dir.x = pivot.x - pos.x;
        dir.y = pivot.y - pos.y;
        dir.z = pivot.z - pos.z;

        var distance = new Vector3df();
        distance.x = dir.x;
        distance.y = dir.y;
        distance.z = dir.z;

        dir = normalize(dir);
        var acceleration = 0.01;
        var delta = event.relativeZ;

        dir.x = dir.x * delta*acceleration;
        dir.y = dir.y * delta*acceleration;
        dir.z = dir.z * delta*acceleration;

        if (delta>0 && (getLength(distance)+getLength(dir) > min))
        {
            zoomed = true;
        }
        if (delta<0 && (getLength(distance)+getLength(dir) <max))
        {
            zoomed = true;
        }
        if (zoomed)
        {
            transform.pos.x = transform.pos.x + dir.x;
            transform.pos.y = transform.pos.y + dir.y;
            transform.pos.z = transform.pos.z + dir.z;

            objectcameraentity.placeable.transform = transform; 

            objectcameraentity.placeable.LookAt(new QVector3D(pivot.x, pivot.y, pivot.z));
        }
    }
}

function cameraZoom()
{
    /*
     * TODO: cameraZoom function needs to be implemented
     *
    var entityplaceable = last_clicked.GetComponentRaw("EC_Placeable");
    var cameraentity = scene.GetEntityByNameRaw("ObjectCamera");
    if (cameraentity == null)
        return;
    var cameraplaceable = cameraentity.GetComponentRaw("EC_Placeable");
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
        cameraplaceable.LookAt(new QVector3D(cameratransform.pos.x, cameratransform.pos.y, cameratransform.pos.z));
    }
    else
    {
        zooming = false;
    }
    */
}

function mouseMove(event)
{
    if(objectcamera_mode)
    {
        var width = renderer.GetWindowWidth();
        var height = renderer.GetWindowHeight();

        var x = 4*Math.PI*event.relativeX/width;
        var y = 4*Math.PI*event.relativeY/height;

        var transform = objectcameraentity.placeable.transform;       
        var pos = new Vector3df();
        pos.x = transform.pos.x;
        pos.y = transform.pos.y;
        pos.z = transform.pos.z;

        var pivot = last_clicked_pos;

        var dir = new Vector3df();
        dir.x = pos.x - pivot.x;
        dir.y = pos.y - pivot.y;
        dir.z = pos.z - pivot.z;

        var quat = QQuaternion.fromAxisAndAngle(objectcameraentity.placeable.LocalYAxis, (-x*180)/Math.PI);
        quat = multiply_quats(quat, QQuaternion.fromAxisAndAngle(objectcameraentity.placeable.LocalXAxis, (-y*180)/Math.PI));
        var dirq = quat.rotatedVector(new QVector3D(dir.x, dir.y, dir.z));
        dir.x = dirq.x();
        dir.y = dirq.y();
        dir.z = dirq.z();

        var new_pos = new Vector3df();
        new_pos.x = pivot.x + dir.x;
        new_pos.y = pivot.y + dir.y;
        new_pos.z = pivot.z + dir.z;

        transform.pos.x = new_pos.x;
        transform.pos.y = new_pos.y;
        transform.pos.z = new_pos.z; 

        objectcameraentity.placeable.transform = transform;
        objectcameraentity.placeable.LookAt(new QVector3D(pivot.x, pivot.y, pivot.z));
    }
}

function mouseLeftRelease(event)
{
    if (objectcamera_mode)
    {
        mouse_left_pressed = false;
        objectcamera_mode = false;
    }
}

function multiply_quats(q, r)
{
    var w = q.scalar()*r.scalar() - q.x()*r.x() - q.y()*r.y() - q.z()*r.z();
    var x = q.scalar()*r.x() + q.x()*r.scalar() + q.y()*r.z() - q.z()*r.y();
    var y = q.scalar()*r.y() + q.y()*r.scalar() + q.z()*r.x() - q.x()*r.z();
    var z = q.scalar()*r.z() + q.z()*r.scalar() + q.x()*r.y() - q.y()*r.x();
    var t = new QQuaternion(w, x, y, z);
    return t;
}

function normalize(vec)
{
    var result = new Vector3df();
    var length = vec.x*vec.x + vec.y*vec.y + vec.z*vec.z;
    if (length == 0)
        return new Vector3df(0, 0, 0);

    length = 1/Math.sqrt(length);
    result.x = vec.x * length;
    result.y = vec.y * length;
    result.z = vec.z * length;

    return result;
}

function getLength(vec)
{
    var result = Math.sqrt(vec.x*vec.x + vec.y*vec.y + vec.z*vec.z);    
    return result;
}
