
var widget = null;

function log(msg)
{
    console.LogInfo("[Instancing Script]: " + msg);
}

function init()
{
    if (framework.IsHeadless())
        return;

    engine.ImportExtension("qt.core");
    engine.ImportExtension("qt.gui");

    // Don't limit FPS
    application.targetFpsLimit = 0;

    // Setup camera, only on first run (keep pos for reloads)
    var cameraEnt = scene.EntityByName("FreeLookCamera");
    if (cameraEnt.camera.farPlane != 2000000)
    {
        var t = cameraEnt.placeable.transform;
        t.pos = new float3(-330,650,-330);
        t.rot = new float3(-35,-135,0);
        cameraEnt.placeable.transform = t;
        cameraEnt.camera.farPlane = 2000000;
    }

    widget = new QWidget();
    widget.objectName = "instancingWidget";
    widget.setLayout(new QHBoxLayout());
    widget.styleSheet = "QWidget#instancingWidget { border: 1px solid grey; } QWidget { font-family: 'Arial'; font-size: 12pt; }"

    var spanBox = new QSpinBox();
    spanBox.objectName = "spanBox";
    spanBox.singleStep = 5;
    spanBox.maximum = 10000;
    spanBox.value = 50;

    var instancingBox = new QCheckBox("Use Istancing");
    instancingBox.objectName = "useInstancingBox";
    instancingBox.checked = true;

    var drawDebugbox = new QCheckBox("Draw Debug");
    drawDebugbox.objectName = "drawDebugBox";
    drawDebugbox.toggled.connect(onDrawDebugToggled);

    var buttonStatic = new QPushButton("Enable Static Instancing");
    buttonStatic.checkable = true;
    buttonStatic.objectName = "buttonStatic";
    buttonStatic.toggled.connect(onToggleStatic);

    var buttonRefresh = new QPushButton("Refresh");
    buttonRefresh.objectName = "buttonRefresh";
    buttonRefresh.clicked.connect(generate);

    widget.layout().addWidget(new QLabel("Row/column count"), 0, 0);
    widget.layout().addWidget(spanBox, 0, 0);
    widget.layout().addWidget(instancingBox, 0, 0);
    widget.layout().addWidget(drawDebugbox, 0, 0);
    widget.layout().addWidget(buttonStatic, 0, 0);
    widget.layout().addWidget(buttonRefresh, 0, 0);

    var proxy = ui.AddWidgetToScene(widget, Qt.Widget);
    proxy.windowFlags = Qt.Widget;
    proxy.x = 10; proxy.y = 10;
    proxy.visible = true;

    drawDebugbox.checked = true;

    generate();
}

function onDrawDebugToggled()
{
    scene.ogre.drawDebugInstancing = findChild(widget, "drawDebugBox").checked;
    log("Setting draw instancing debug to " + scene.ogre.drawDebugInstancing);
}

function onToggleStatic()
{
    var button = findChild(widget, "buttonStatic");
    var static = button.checked;
    if (scene.ogre.SetInstancesStatic("instancing-cube.mesh", static))
    {
        log("Setting instancing to static for 'instancing-cube.mesh': " + static);
        button.text = (static ? "Disable" : "Enable") + " Static Instancing";
    }
}

function generate()
{
    var span            = findChild(widget, "spanBox").value;
    var useInstancing   = findChild(widget, "useInstancingBox").checked;
    var x = z           = 0;

    // Clear any meshe entities in the scene
    var meshEnts = scene.EntitiesWithComponent("EC_Mesh");
    for (var i = meshEnts.length-1; i >= 0; --i)
        scene.RemoveEntity(meshEnts[i].id);

    log("Creating " + (useInstancing ? "instanced" : "non-instanced") + " meshes, please wait...");

    for (var i = 0; i < span; ++i)
    {
        for (var k = 0; k < span; ++k)
        {
            var entName = (i+1) + "_" + (k+1);
            var ent = scene.EntityByName(entName);
            if (ent != null)
                scene.RemoveEntity(ent.id);

            ent = scene.CreateLocalEntity(["EC_Name", "EC_Placeable", "EC_Mesh"], 2, false, true);
            ent.name = entName;

            ent.mesh.meshRef = "instancing-cube.mesh";
            ent.mesh.meshMaterial = [ "instancing-textured.material" ];
            ent.mesh.useInstancing = useInstancing;

            ent.placeable.SetScale(0.1, 0.1, 0.1);
            ent.placeable.SetPosition(new float3(x, 0, z));
            x += 20;
        }
        z += 20;
        x = 0;
    }

    log(">> Created " + (span*span) + " entities");
}

function OnScriptDestroyed()
{
    if (widget != null)
    {
        log("Uninitializing UI.");
        ui.RemoveWidgetFromScene(widget);
        widget = null;
    }
}

init();
