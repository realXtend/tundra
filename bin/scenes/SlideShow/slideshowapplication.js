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
	    var filename = ("" + urls[u]).split('///')[1];
	    var file = new QFile(filename);
	    file.open(QIODevice.ReadOnly);
	    var streamer = new QTextStream(file);
	    slides = [];
	    while (true) {
		var line = streamer.readLine();
		if (!line) {
		    break;
		}
		slides.push(line)
	    }
    	    print(slides);
	    createCanvas(filename, slides);
	}
    }
}

function createCanvas(filename, slides) {
    entity = scene.CreateEntityRaw(scene.NextFreeId(), ['EC_Placeable', 'EC_Mesh', 'EC_3DCanvasSource', 'EC_Name', 'EC_DynamicComponent']);

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

    // start canvas
    var canvas = entity.GetComponentRaw('EC_3DCanvas');
    frame.DelayedExecute(0.1).Triggered.connect(this, function () {
	canvas.Start();
    });

    // Make dynamic component of the slide stuff

    var dyn = entity.dynamiccomponent;

    dyn.Name ="Slidelist";

    for (s = 0; s < slides.length; s++) {
	var slidename = s;
	var attr = dyn.CreateAttribute("string", slidename);
	dyn.SetAttribute(slidename, slides[s] + "");
	
    }
    dyn.CreateAttribute("int", "Current");
    dyn.SetAttribute("Current", 0);

    dyn.OnAttributeChanged.connect(onSlideChanged);
    
    //FIXME not needed in final product :) Just here to make it show
    //right away...
    
    var transform = entity.placeable.transform
    var pos = new Vector3df();
    pos.y = 20;
    transform.pos = pos;
    var rot = new Vector3df();
    rot.y = 180;
    rot.x = 180;
    transform.rot = rot;
    entity.placeable.transform = transform;

    // Now we are done
    scene.EmitEntityCreatedRaw(entity);
    
    entity.Action("MousePress").Triggered.connect(nextSlide);


}

function nextSlide() {
    print("Next slide!");
    changeSlide(1);
}

function prevSlide() {
    print("Previous slide!");
    changeSlide(-1);
}


function changeSlide(dir) {
    print("Changing slide!");
    var dyn = entity.GetComponentRaw("EC_DynamicComponent", "Slidelist");
    var slide_index = dyn.GetAttribute("Current") + dir;

    if (slide_index >= slides.length) {
	slide_index = 0;
    }
    
    if (slide_index < 0) {
	slide_index = slides.length - 1;
    }

    dyn.SetAttribute("Current", slide_index);
}

function onSlideChanged(attribute,type) {
    var dyn = entity.GetComponentRaw("EC_DynamicComponent", "Slidelist");
    var index = dyn.GetAttribute('Current');
    canvassource.source = dyn.GetAttribute(index);
}

function MyLabel(parent) {
    QLabel.call(this, "", parent, 0);
}

MyLabel.prototype = new QLabel();

MyLabel.prototype.mousePressEvent = function (event) {
    event.accept();
    print('AAAAARGH');
}

function makeSlideWidget(slides) {
    var gfxscene = new QGraphicsScene();
    var view = new QGraphicsView(gfxscene);

    view.resize(110, 400);

    var slidethumbs = [];
    

    for (s = 0; s < slides.length; s++) {
	var label = new MyLabel(gfxscene);
	var pic = new QPixmap("C:\\Users\\playsign\\sand_d.jpg");
	label.setPixmap(pic.scaledToWidth(100));
	gfxscene.addWidget(label)
	label.move(0, 110 * s);

    }    
    uiservice.AddWidgetToScene(view);
    view.show();

}

makeSlideWidget([1,2,3,4]);