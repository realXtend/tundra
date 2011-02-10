/*
  SlideShow

  TODO
   * Get filesuffix better
   * Open asset from url!

*/

engine.ImportExtension("qt.core");
engine.ImportExtension("qt.gui");

// connect to Drag events
ui.GraphicsView().DragEnterEvent.connect(handleEnter);
ui.GraphicsView().DragMoveEvent.connect(handleMove);
ui.GraphicsView().DropEvent.connect(handleDrop);
print ('foo')

var entity;
var canvassource;
var slides;
var slide_index = 0;

function checkSuffix(url) {
    // FIXME
    if (("" + url).split('.txt').length == 2) {
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
	    print(filename)
	    var file = new QFile(filename);
	    file.open(QIODevice.ReadOnly);
	    var streamer = new QTextStream(file);
	    slides = [];
	    while (true) {
		var line = streamer.readLine();
		print(line)
		if (!line) {
		    break;
		}
		slides.push(line)
	    }
    	    print(slides);
	    createCanvas(filename, slides, event);
	}
    }
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


function createCanvas(filename, slides, event) {
    entity = scene.CreateEntityRaw(scene.NextFreeId(), ['EC_Placeable', 'EC_Mesh', 'EC_3DCanvasSource', 'EC_Name', 'EC_DynamicComponent', 'EC_Script']);

    // set name
    entity.name.name = "Slideshow: " + filename;
    entity.name.description = "Simple slideshow app from " + filename;

    // set mesh
    var mesh_ref = entity.mesh.meshRef;
    mesh_ref.ref = 'local://screen.mesh';
    entity.mesh.meshRef = mesh_ref;

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
    //makeSlideWidget(slides);

    // add buttons

    rotvec = new Vector3df();

    rotvec.x = 180;
    rotvec.y = 0;

    makeButton('prev', entity.placeable, -1, rotvec);

    rotvec.x = 187;
    rotvec.y = 180;
       
    makeButton('next', entity.placeable, 1, rotvec);

    // Now we are done
    scene.EmitEntityCreatedRaw(entity);

}

function makeButton(name, placeable, dir, rotation) {

    var button = scene.CreateEntityRaw(scene.NextFreeId(), ['EC_Mesh', 'EC_Placeable', 'EC_Name']);
    button.name.name = 'Button ' + name +' (' + entity.name.name + ')';

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

    scene.EmitEntityCreatedRaw(button);


}

function makeSlideWidget(slides) {
    var gfxscene = new QGraphicsScene();
    var view = new QGraphicsView(gfxscene);

    view.resize(110, 400);

    for (s = 0; s < slides.length; s++) {
	var label = new MyLabel(gfxscene, s);
	var pic = new QPixmap("C:\\Users\\playsign\\sand_d.jpg");
	label.setPixmap(pic.scaledToWidth(100));
	label.move(0, 110 * s);
	gfxscene.addWidget(label)


    }    
    uiservice.AddWidgetToScene(view);
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
