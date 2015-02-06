/** For conditions of distribution and use, see copyright notice in LICENSE

    EntityMoveTest.js - Demo scene for profiling object movement replication. */

// Number of boxes = numRows x numColumns
var numRows = 10;
var numColums = 10;
var boxes = []; // All the created boxes are stored here.
var originalTransforms = [];
var startPositions = [];
var startRotations = [];
var startScales = [];
var destinationPositions = [];
var destinationRotations = [];
var destinationScales = [];
var moveTimer = 0;
var dirChangeFreq = 4; // seconds
// Current set of operations.
var move = false;
var rotate = false;
var scale = false;
var cInputContextName = "EntityMoveTest";

function ParseBool(str)
{
    str = str.trim().toLowerCase();
    return (str == "true" || str == "yes" || str == "1" || str == "y" || str == "on");
}

function OnScriptDestroyed()
{
    if (framework.IsExiting())
        return;

    if (server.IsRunning())
    {
        DeleteBoxes();
        if (!framework.IsHeadless())
            input.UnregisterInputContextRaw(cInputContextName);

        console.UnregisterCommand("setNumRows");
        console.UnregisterCommand("setNumCols");
        console.UnregisterCommand("setMoving");
        console.UnregisterCommand("setRotating");
        console.UnregisterCommand("setScaling");
        console.UnregisterCommand("increaseSpeed");
        console.UnregisterCommand("decreaseSpeed");
    }
}

// Entry point for the script.
if (server.IsAboutToStart())
{
    engine.ImportExtension("qt.core");

    CreateBoxes();
    frame.Updated.connect(Update);

    if (!framework.IsHeadless())
    {
        var inputContext = input.RegisterInputContextRaw(cInputContextName, 90);
        inputContext.KeyPressed.connect(HandleKeyPressed);
    }

    console.RegisterCommand("setNumRows", "EntityMoveTest: Sets number of rows.").Invoked.connect(function(params)
    {
         numRows = parseInt(params[0]);
         DeleteBoxes();
         CreateBoxes();
    });
    console.RegisterCommand("setNumCols", "EntityMoveTest: Sets number of columns.").Invoked.connect(function(params)
    {
        numCols = parseInt(params[0]);
        console.LogInfo("numCols " + numCols);
        DeleteBoxes();
        CreateBoxes();
    });
    console.RegisterCommand("setMoving", "EntityMoveTest: Enabled/disables movement of objects.").Invoked.connect(function(params)
    {
        move = ParseBool(params[0]);
    });
    console.RegisterCommand("setRotating", "EntityMoveTest: Enabled/disables rotation of objects.").Invoked.connect(function(params)
    {
        rotate = ParseBool(params[0]);
    });
    console.RegisterCommand("setScaling", "EntityMoveTest: Enabled/disables scaling of objects.").Invoked.connect(function(params)
    {
        scale  = ParseBool(params[0]);
    });
    console.RegisterCommand("increaseSpeed", "EntityMoveTest: Increases speed.").Invoked.connect(function(/*params*/)
    {
        IncreaseSpeed();
    });
    console.RegisterCommand("decreaseSpeed", "EntityMoveTest: Decreases speed.").Invoked.connect(function(/*params*/)
    {
        DecreaseSpeed();
    });
}
else
{
    syncmanager.observer = scene.EntityByName("FreeLookCamera");
    console.LogInfo(syncmanager.observer);
}

function IncreaseSpeed()
{
    dirChangeFreq += 0.25;
}

function Decreasepeed()
{
    dirChangeFreq -= 0.25;
    if (dirChangeFreq < 0.25)
        dirChangeFreq = 0.25;
}

function HandleKeyPressed(e)
{
    if (e.HasCtrlModifier())
    {
        switch(e.keyCode)
        {
        case Qt.Key_1:
            move = !move;
            break;
        case Qt.Key_2:
            rotate = !rotate;
            break;
        case Qt.Key_3:
            scale = !scale;
            break;
        case Qt.Key_R:
            Reset();
            break;
        case Qt.Key_Plus:
            IncreaseSpeed();
            break;
        case Qt.Key_Minus:
            Decreasepeed();
            break;
        }
    }
}

function DeleteBoxes()
{
    for(var i = 0; i < boxes.length; ++i)
        try
        {
            var id = boxes[i].id;
            boxes[i] = null;
            scene.RemoveEntity(id);
        }
        catch(e)
        {
            // We end up here when quitting the app.
            console.LogWarning(e);
        }

    boxes = [];
}

function CreateBoxes()
{
    for(var i = 0; i < numRows; ++i)
        for(var j = 0; j < numColums; ++j)
        {
            var box = scene.CreateEntity(0, ["Name", "Mesh", "Placeable"]);
            box.name = "Box" + i.toString() + j.toString();

            box.mesh.meshRef = "Box.mesh";
            box.mesh.meshMaterial = [ "Box.material" ];

            box.placeable.SetScale(new float3(5, 5, 5));
            box.placeable.SetPosition(new float3(7*i, 0, 7*j));
            boxes.push(box);

            // Save start and destination values.
            originalTransforms.push(box.placeable.transform);

            var pos = box.placeable.transform.pos;
            var dest = new float3(pos);
            dest.y = (Math.random() > 0.5) ? -2 : 2;
            startPositions.push(pos);
            destinationPositions.push(dest);

            startRotations.push(box.placeable.transform.rot);
            destinationRotations.push(new float3(0, 360, 0));

            startScales.push(box.placeable.transform.pos);
            destinationScales.push(new float3(1,1,1).Mul(Math.random() * 5));
        }

    console.LogInfo("EntityMoveTest.CreateBoxes: " + boxes.length + " boxes created.");
}

function Reset()
{
    dirChangeFreq = 4;
    for(var i = 0; i < boxes.length; ++i)
        boxes[i].placeable.transform = originalTransforms[i];
    move = rotate = scale = false;
}

function Update(frameTime)
{
    moveTimer += frameTime;
    if (moveTimer > dirChangeFreq)
        moveTimer = 0;
    if (move)
        MoveBoxes();
    if (rotate)
        RotateBoxes();
    if (scale)
        ScaleBoxes();
}

function MoveBoxes()
{
    profiler.BeginBlock("EntityMoveTest.MoveBoxes");

    for(var i = 0; i < boxes.length; ++i)
    {
        var box = boxes[i];
        var currentPos = box.placeable.transform.pos;
        var dest = destinationPositions[i];

        if (moveTimer === 0)
        {
            startPositions[i] = currentPos;
            dest.y = Math.random() * 5;
            dest.y = -dest.y;
        }

        currentPos = float3.Lerp(startPositions[i], dest, moveTimer/dirChangeFreq);
        box.placeable.SetPosition(currentPos);
    }

    profiler.EndBlock();
}

function Lerp(a, b, t)
{
    return a*(1-t) + (b*t);
}

function RotateBoxes()
{
    profiler.BeginBlock("EntityMoveTest.RotateBoxes");

    for(var i = 0; i < boxes.length; ++i)
    {
        var box = boxes[i];
        var currentRot = box.placeable.transform.rot;
        var destRot = destinationRotations[i];

        if (moveTimer === 0)
        {
            startRotations[i] = currentRot;
            destRot.y = (destRot.y === 0 ? 360 : 0);
        }

        currentRot.y = Lerp(startRotations[i].y, destRot.y, moveTimer/dirChangeFreq);
        var t = box.placeable.transform;
        t.rot.y = currentRot.y;
        box.placeable.transform = t;
    }

    profiler.EndBlock();
}

function ScaleBoxes()
{
    profiler.BeginBlock("EntityMoveTest.ScaleBoxes");

    for(var i = 0; i < boxes.length; ++i)
    {
        var box = boxes[i];
        var currentScale = box.placeable.transform.scale;
        var dest = destinationScales[i];

        if (moveTimer === 0)
        {
            startScales[i] = currentScale;
            destinationScales[i] = new float3(1,1,1).Mul(Math.random() * 5);
        }

        currentScale = float3.Lerp(startScales[i], dest, moveTimer/dirChangeFreq);
        box.placeable.SetScale(currentScale);
    }

    profiler.EndBlock();
}
