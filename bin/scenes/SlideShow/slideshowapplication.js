/*
  SlideShow

  TODO
   * Get filesuffix better

*/

engine.ImportExtension("qt.core");
engine.ImportExtension("qt.gui");

//handly for dealing with qbytearrays
QByteArray.prototype.toString = function() {
    ts = new QTextStream( this, QIODevice.ReadOnly ); 
    return ts.readAll();
}


// connect to Drag events
ui.GraphicsView().DragEnterEvent.connect(handleEnter);
ui.GraphicsView().DragMoveEvent.connect(handleMove);
ui.GraphicsView().DropEvent.connect(handleDrop);

// Url for the server
var serverurl = "http://192.168.1.114:8000/";
// Storage name
var storagename = "Productivity Converter storage";

print ('foo');

var entity;
var canvassource;
var prev;
var next;
var slide_index = 0;
var max_slides;

function checkSuffix(url) {
    // FIXME find a better way to get the suffix
    url = ("" + url).replace('.pptx', '.ppt')
    if (url.split('.ppt').length == 2) {
    print("I accept!");
    return true;
    }
    print("No sir, I don't like it");
    return false;
}

function accept(event) {
    
    var mimedata = event.mimeData();

    if (mimedata.hasUrls()) {
    var urls = mimedata.urls();
    for (u = 0; u < urls.length; u++ ) {
        if (checkSuffix(urls[u])) {
        event.acceptProposedAction();        
        }
    }
    }
}

function handleEnter(event) {
    print("Enter the event!");
    accept(event);
}

function handleMove(event) {
    print("I like to move it move it!");
    accept(event);
}

function handleDrop(event) {
    print ("DROP!");
    accept(event);
    
    var mimedata = event.mimeData();
    var urls = mimedata.urls();
    for (u = 0; u < urls.length; u++ ) {
    if (checkSuffix(urls[u])) {
        //FIXME!!!!!!1!
        print(urls[u]+"");
        var filename = ("" + urls[u]).split('//')[1];
        print(filename);
        upload(serverurl, storagename, filename);
        break;
    }
    }
    createCanvas(event);
}

function upload(uploadStorageUrl, uploadStorageName, filename) {
    if (AddAssetStorage(uploadStorageUrl, uploadStorageName)) {
    var parts = filename.split('/');
    var newName = parts[parts.length - 1];
    UploadAsset(filename, uploadStorageName, newName, "binary");
    } else {
    print("HORRERNOUS ERROR! CANNOT HANDLE ERRORNOUS HORROR!!!1!");
    }
}

function AddAssetStorage(url, name) {
    var assetstorageptr = asset.AddAssetStorage(url, name);
    var assetstorage = assetstorageptr.get();
    if (assetstorage != null) {
        print("Added asset storage");
        print(">>  Name :", assetstorage.Name());
        print(">>  URL  :", assetstorage.BaseURL(), "\n");
        return true;
    } else {
        print("Failed to add asset storage:", name, "-", url, "\n");
        return false;
    }
}

function UploadAsset(fileName, storageName, uploadName) {
    print("Uploading:", fileName, "with destination name", uploadName);
    var uploadtransferptr = asset.UploadAssetFromFile(fileName, storageName, uploadName);
    var uploadtransfer = uploadtransferptr.get();
    if (uploadtransfer != null) {
        uploadtransfer.Completed.connect(UploadCompleted);
        uploadtransfer.Failed.connect(UploadFailed);
    } else {
        print(" >> Failed to upload, AssetAPI returned a null AssetUploadTransferPtr");
    }
}


function RequestAsset(ref, type)
{
    ForgetAsset(ref);

    print("Requesting:", ref);
    var transferptr = asset.RequestAsset(ref, type);
    var transfer = transferptr.get();
    transfer.Downloaded.connect(DownloadReady);
}

function DownloadReady(/* IAssetTransfer* */ transfer)
{
    var data = transfer.GetRawData();
    print("Download ready");
    print("  >> Source    :", transfer.GetSourceUrl());
    print("  >> Type      :", transfer.GetAssetType());
    print("  >> Data len  :", data.size());
    print("  >> index     :", parseInt(data.toString()));
    noSlides = parseInt(data.toString());

    var slides = [];
    
    var baseurl = transfer.GetSourceUrl().replace('/index.txt', '');
    print(baseurl)
    var parts = baseurl.split('/')
    print(parts)
    var slidename = parts[parts.length - 1]
    print(slidename)
    
    for (i = 0; i < noSlides; i++) {
         slides.push(baseurl + '/' + slidename + '-' + i + '.png');
    }

    // set source to first slide
    canvassource = entity.GetComponentRaw('EC_3DCanvasSource')
    canvassource.show2d = false;
    canvassource.source = slides[0];
    canvassource.submesh = 1;

    // Make dynamic component of the slide stuff

    var dyn = entity.dynamiccomponent;

    dyn.Name = "Slidelist";

    for (s = 0; s < slides.length; s++) {
    var slidename = s;
    var attr = dyn.CreateAttribute("string", slidename);
    dyn.SetAttribute(slidename, slides[s] + "");
    }

    dyn.CreateAttribute("int", "Current");
    dyn.SetAttribute("Current", 0);

    dyn.CreateAttribute("int", "MaxIndex");
    dyn.SetAttribute("MaxIndex", noSlides - 1);


    // ec_script
    var script = entity.script;
    script.type = "js";
    script.runOnLoad = true;
    var r = script.scriptRef;
    r.ref = "local://slideshow.js";
    script.scriptRef = r;

    //FIXME move this and makeslide widget to slideshow.js when
    // appropriate
    // Create UI
    // makeSlideWidget(slides);
    
    // Now we are done
    scene.EmitEntityCreatedRaw(entity);
}

function ForgetAsset(assetRef) {
    // Make AssetAPI forget this asset if already loaded in
    // to the system and remove the disk cache entry.
    asset.ForgetAsset(assetRef, true);
}

function getSlides(ref) {
    var parts = ref.split('/');
    var filename = parts[parts.length - 1];
    var path = filename.replace('.pptx', '');
    path = path.replace('.ppt', '');
    RequestAsset(serverurl + path + '/index.txt', "Binary");

    // set name
    entity.name = "Slideshow: " + filename;
    entity.name.description = "Simple slideshow app from " + filename;
    prev.name = 'Button prev (' + entity.name + ' ' + entity.id + ')';
    scene.EmitEntityCreatedRaw(prev);
    next.name.name = 'Button next (' + entity.name.name + ' ' + entity.id + ')';
    scene.EmitEntityCreatedRaw(next);
}



function UploadCompleted(/* IAssetUploadTransfer* */ transfer) {
    print("Upload completed");
    print("  >> New asset ref    :", transfer.AssetRef());
    print("  >> Destination name :", transfer.GetDesticationName(), "\n");

    getSlides(transfer.AssetRef())
}

function UploadFailed(/* IAssetUploadTransfer* */ transfer) {
    print("Upload failed");
    print("  >> File name   :", transfer.GetSourceFilename());
    print("  >> Destination :", transfer.GetDesticationName(), "\n");
}

    
function vprint(v) {
    print(v.x + ", " + v.y + ", " + v.z);
}

function crossp(v1, v2) {
    var result = new Vector3df();

    result.x = v1.y * v2.z - v1.z * v2.y;
    result.y = v1.z * v2.x - v1.x * v2.z;
    result.z = v1.x * v2.y - v1.y * v2.x;

    return result;
}

function vadd(v1, v2) {
    var result = new Vector3df();

    result.x = v1.x + v2.x;
    result.y = v1.y + v2.y;
    result.z = v1.z + v2.z;

    return result;
}

function smul(v1, s) {
    var result = new Vector3df();
    
    result.x = v1.x * s;
    result.y = v1.y * s;
    result.z = v1.z * s;

    return result;
}

function conjg(quat, v) {
    var qvec = new Vector3df();
    qvec.x = quat.x;
    qvec.y = quat.y;
    qvec.z = quat.z;

    var uv = crossp(qvec, v);
    var uuv = crossp(qvec, uv);
    uv = smul(uv, 2.0 * quat.w);
    uuv = smul(uuv, 2.0);

    return vadd(vadd(v, uv), uuv);
}


function createCanvas(event) {
    entity = scene.CreateEntityRaw(scene.NextFreeId(), ['EC_Placeable', 'EC_Mesh', 'EC_3DCanvasSource', 'EC_Name', 'EC_DynamicComponent', 'EC_Script']);

    // set mesh
    var mesh_ref = entity.mesh.meshRef;
    mesh_ref.ref = 'local://screen.mesh';
    entity.mesh.meshRef = mesh_ref;

    var worldpos;

    var res = renderer.Raycast(event.pos().x(), event.pos().y());

    // Calculationg the correct position for dropped slideshow
    if (!res.entity) {
        // no hit
        var ids = scene.GetEntityIdsWithComponent('EC_OgreCamera');
        for (i = 0; i < ids.length; i++) {
        var camentity = scene.GetEntityRaw(ids[i]);
            var placeable = camentity.GetComponentRaw('EC_Placeable');
            if (placeable) {
            var q = placeable.Orientation;

        // create unitvector for negative Z-axis
        var unz = new Vector3df();
        unz.z = -1;

        // calculate conjugate
        var v = conjg(q, unz);
        worldpos = vadd(placeable.Position, smul(v, 20));
            break;
            }
        }
    } else {
        worldpos = res.pos();
    }

    // set postition and rotation    
    var transform = entity.placeable.transform;

    transform.pos.x = worldpos.x;
    transform.pos.y = worldpos.y;
    transform.pos.z = worldpos.z;
    transform.rot.x = placeable.transform.rot.x - 90;
    transform.rot.y = placeable.transform.rot.y;
    transform.rot.z = placeable.transform.rot.z - 180;
    
    entity.placeable.transform = transform;

    rotvec = new Vector3df();

    rotvec.x = 187;
    rotvec.y = 180;

    prev = makeButton('prev', entity.placeable, 1, rotvec);

    rotvec.x = 180;
    rotvec.y = 0;

    next = makeButton('next', entity.placeable, -1, rotvec);
}

function makeButton(name, placeable, dir, rotation) {

    var button = scene.CreateEntityRaw(scene.NextFreeId(), ['EC_Mesh', 'EC_Placeable', 'EC_Name']);

    var button_mesh_ref = button.mesh.meshRef;
    button_mesh_ref.ref = 'local://kolmionappi.mesh';
    button.mesh.meshRef = button_mesh_ref;

    // create unit vector for X-axis (negative or positive)
    var unx = new Vector3df();
    unx.x = dir;
    
    // calculate conjugate
    var v = conjg(placeable.Orientation, unx);
    worldpos = vadd(placeable.Position, smul(v, 6));

    var transform = button.placeable.transform;
    transform.pos = worldpos;

    transform.rot = vadd(placeable.transform.rot, rotation);

    button.placeable.transform = transform;

    return button;
}

function makeSlideWidget(slides) {
    var gfxscene = new QGraphicsScene();
    var view = new QGraphicsView(gfxscene);

    view.resize(110, 400);

    for (s = 0; s < slides.length; s++) {
    var label = new MyLabel(gfxscene, s);
    var pic = new QPixmap(slides[s]);
    label.setPixmap(pic.scaledToWidth(100));
    label.move(0, 110 * s);
    gfxscene.addWidget(label)
    }

    ui.AddWidgetToScene(view);
    view.show();
}

function MyLabel(parent, slide) {
    this.slide = slide;
    QLabel.call(this, "", parent, 0);
}

MyLabel.prototype = new QLabel();

MyLabel.prototype.mousePressEvent = function (event) {
    event.accept();
    print('AAAAARGH! ' + this.slide);
    var dyn = entity.dynamiccomponent;
    dyn.SetAttribute("Current", this.slide + "");
}
