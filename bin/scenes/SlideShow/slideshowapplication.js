/*
  SlideShow

  TODO
   * Get filesuffix better
   * Accept only one file at the time 
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

    canvassource.source = dyn.GetAttribute(slide_index);
    dyn.SetAttribute("Current", slide_index);
}

MyPixMap = {};

MyPixMap.prototype = new QWidget();




function makeSlideWidget() {
    var gfxscene = new QGraphicsScene();
    var view = new QGraphicsView(gfxscene);
    var slidethumbs = [];

    for (s = 0; s < 5; s++) {
	slidethumbs.push((new QPixmap("C:\\Users\\playsign\\sand_d.jpg")).scaledToWidth(100));
	slidethumbs.push((new QPixmap("C:\\Users\\playsign\\terrapin_texture.png")).scaledToWidth(100));
    }

    uiservice.AddWidgetToScene(view);

    for (s = 0; s < slidethumbs.length; s++) {
	var item = gfxscene.addPixmap(slidethumbs[s]);
	item.mousePressEvent = function(event) { print("goo"); };
	item.setPos(0,110 * s);
    }
    
    view.resize(110, 400);
    view.show();

}

makeSlideWidget();