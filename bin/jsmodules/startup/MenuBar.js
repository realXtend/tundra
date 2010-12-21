engine.ImportExtension("qt.core");
engine.ImportExtension("qt.gui");

var menu = ui.MainWindow().menuBar();
menu.clear();

var fileMenu = menu.addMenu("&File");                     
//fileMenu.addAction("New scene").triggered.connect(NewScene);
fileMenu.addAction("Quit").triggered.connect(Quit);

var viewMenu = menu.addMenu("&View");
if (framework.GetModuleQObj("SceneStructure"))
{
  viewMenu.addAction("Assets").triggered.connect(OpenAssetsWindow);
  viewMenu.addAction("Scene").triggered.connect(OpenSceneWindow);
}

//var eceditorAction = viewMenu.addAction("EC Editor");

if (framework.GetModuleQObj("DebugStats"))
  viewMenu.addAction("Profiler").triggered.connect(OpenProfilerWindow);

if (framework.GetModuleQObj("Environment"))
  viewMenu.addAction("Terrain Editor").triggered.connect(OpenTerrainEditor);

if (framework.GetModuleQObj("PythonScript"))
  viewMenu.addAction("Python Console").triggered.connect(OpenPythonConsole);

function NewScene()
{
  scene.RemoveAllEntities();
}

function Quit()
{
  framework.Exit();
}

function OpenSceneWindow()
{
  framework.GetModuleQObj("SceneStructure").ShowSceneStructureWindow();
}

function OpenAssetsWindow()
{
  framework.GetModuleQObj("SceneStructure").ShowAssetsWindow();
}

function OpenProfilerWindow()
{
  console.ExecuteCommand("prof");
}

function OpenTerrainEditor()
{
  console.ExecuteCommand("TerrainTextureEditor");
}

function OpenPythonConsole()
{
  console.ExecuteCommand("pythonconsole");
}
