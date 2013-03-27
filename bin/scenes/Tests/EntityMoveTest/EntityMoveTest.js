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

function OnScriptDestroyed()
{
    DeleteBoxes();
    if (!framework.IsHeadless())
        input.UnregisterInputContextRaw("EntityMoveTest");
}

// Entry point for the script.
if (server.IsAboutToStart())
{
    engine.ImportExtension("qt.core");

    CreateBoxes();
    frame.Updated.connect(Update);

    if (framework.IsHeadless())
    {
        // For now, enable only movement on headless servers.
        move = true;
    }
    else
    {
        var inputContext = input.RegisterInputContextRaw("EntityMoveTest", 90);
        inputContext.KeyPressed.connect(HandleKeyPressed);
    }
}

function HandleKeyPressed(e)
{
    if (e.HasCtrlModifier())
    {
        if (e.keyCode == Qt.Key_1)
            move = !move;
        if (e.keyCode == Qt.Key_2)
            rotate = !rotate;
        if (e.keyCode == Qt.Key_3)
            scale = !scale;
        if (e.keyCode == Qt.Key_R)
            Reset();
        if (e.keyCode == Qt.Key_Plus)
        {
            dirChangeFreq -= 0.25;
            if (dirChangeFreq < 0.25)
                dirChangeFreq = 0.25;
        }
        if (e.keyCode == Qt.Key_Minus)
            dirChangeFreq += 0.25;
    }
}

function DeleteBoxes()
{
    for(var i = 0; i < boxes.length; ++i)
        try
        {
            scene.RemoveEntity(boxes[i].id);
        }
        catch(e)
        {
            // We end up here when quitting the app.
            //console.LogWarning(e);
        }
}

function CreateBoxes()
{
    for(var i = 0; i < numRows; ++i)
        for(var j = 0; j < numColums; ++j)
        {
            var box = scene.CreateEntity(scene.NextFreeId(), ["EC_Name", "EC_Mesh", "EC_Placeable"]);
            box.name = "Box" + i.toString() + j.toString();

            var meshRef = box.mesh.meshRef;
            meshRef.ref = "Box.mesh";
            box.mesh.meshRef = meshRef;

            var matRefs = new AssetReferenceList();
            matRefs = [ "Box.material" ];
            box.mesh.meshMaterial = matRefs;

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

        if (moveTimer == 0)
        {
            startPositions[i] = currentPos;
            destinationPositions[i].y = Math.random() * 5
            destinationPositions[i].y = -destinationPositions[i].y;
        }

        currentPos = float3.Lerp(startPositions[i], destinationPositions[i], moveTimer/dirChangeFreq);
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

        if (moveTimer == 0)
        {
            startRotations[i] = currentRot;
            destinationRotations[i].y = (destinationRotations[i].y == 0 ? 360 : 0);
        }

        currentRot.y = Lerp(startRotations[i].y, destinationRotations[i].y, moveTimer/dirChangeFreq);
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

        if (moveTimer == 0)
        {
            startScales[i] = currentScale;
            destinationScales[i] = new float3(1,1,1).Mul(Math.random() * 5);
        }

        currentScale = float3.Lerp(startScales[i], destinationScales[i], moveTimer/dirChangeFreq);
        box.placeable.SetScale(currentScale);
    }

    profiler.EndBlock();
}
