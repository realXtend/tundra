//a gui tool / menu entry to start a new project, to create a new scene in a new directory.
//sets the new dir as default storage

var mainwin = null;
var orgtitle = null;
var curfile = null;
var saveact = null;

function initgui() {
    engine.ImportExtension("qt.core");
    engine.ImportExtension("qt.gui");

    mainwin = ui.MainWindow();
    orgtitle = mainwin.windowTitle;

    var newact = mainwin.AddMenuAction("&File", "New Scene");
    newact.triggered.connect(showNewSceneDialog);

    saveact = mainwin.AddMenuAction("&File", "Save");
    saveact.triggered.connect(saveCurrent);
    saveact.enabled = false;
}

function showNewSceneDialog() {
    curfile = QFileDialog.getSaveFileName(mainwin, "Scene file", "", "Tundra Scene (*.txml *.tbin)");
    saveact.enabled = true;
    mainwin.windowTitle = curfile + " - " + orgtitle;

    var newstorage = asset.AddAssetStorage("file://" + curfile, "New Scene"); //LocalAssetProvider strips the trailing filename away, uses the dir only
    //print("New storage added: " + newstorage + "-- from filepath: " + curfile); 
    asset.SetDefaultAssetStorage(newstorage);
}

function saveCurrent() {
    if (curfile) {
	//copy-paste from MenuBar.js which has Import & Export scene. should perhaps move this there too -- made here first 'cause thought that they were added by scenestruct or something

	var currentScene = framework.Scene().GetDefaultSceneRaw();

	var ext = new QFileInfo(curfile).suffix();
        if (ext != "txml" && ext != "tbin") {
            QMessageBox.information(ui.MainWindow(), "Invalid extension", "Invalid Tundra scene file extension '" + ext + "'");
            return;
        }
           
        if (ext == "txml")
            currentScene.SaveSceneXML(curfile);
        else if (ext == "tbin")
            currentScene.SaveSceneBinary(curfile);
    }
}

if (server.IsAboutToStart() && !framework.IsHeadless()) {
    initgui();
}
