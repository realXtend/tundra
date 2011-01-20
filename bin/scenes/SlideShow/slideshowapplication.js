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
	    createCanvas(slides);
	}
    }
}

function createCanvas(slides) {
    entity = scene.CreateEntityRaw(scene.NextFreeId(), ['EC_Placeable', 'EC_Mesh', 'EC_3DCanvasSource', 'EC_Name']);

    // set name
    entity.name.name = "Slideshow";
    entity.name.description = "Simple slideshow app";

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

    // Now we are done
    scene.EmitEntityCreatedRaw(entity);
    
    entity.Action("MousePress").Triggered.connect(nextSlide);


}


function nextSlide() {
    print("Changing slide!");
    slide_index += 1;
    if (slide_index >= slides.length) {
	slide_index = 0;
    }

    canvassource.source = slides[slide_index];
}

