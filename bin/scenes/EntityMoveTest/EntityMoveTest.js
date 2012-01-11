/** For conditions of distribution and use, see copyright notice in LICENSE

    Test.js - Demo scene for profiling object movement replication. */

// Number of boxes = numRows x numColumns
var numRows = 100;
var numColums = 10;
var boxes = []; // All the created boxes are stored here.
var startPositions = [];
var destinationPositions = [];
var moveTimer = 0;
var dirChangeFreq = 2; // seconds

function OnScriptDestroyed()
{
    DeleteBoxes();
}

// Entry point for the script.
if (server.IsAboutToStart())
{
    CreateBoxes();
    frame.Updated.connect(MoveBoxes);
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
            box.placeable.SetPosition(new float3(6*i, 0, 6*j));
            boxes.push(box);

            // Save initial destination points.
            var pos = box.placeable.transform.pos;
            var dest = new float3(pos);
            dest.y = (Math.random() > 0.5) ? -2 : 2;
            startPositions.push(pos);
            destinationPositions.push(dest);
        }

    console.LogInfo("CreateBoxes: " + boxes.length + " boxes created.");
}

function MoveBoxes(frameTime)
{
    profiler.BeginBlock("MoveBoxes");

    moveTimer += frameTime;
    if (moveTimer > dirChangeFreq)
        moveTimer = 0;

    for(var i = 0; i < boxes.length; ++i)
    {
        var box = boxes[i];
        var currentPos = box.placeable.transform.pos;
        var dest = destinationPositions[i];

        if (moveTimer == 0)
        {
            startPositions[i] = currentPos;
            destinationPositions[i].y = -destinationPositions[i].y;
        }

        currentPos = float3.Lerp(startPositions[i], destinationPositions[i], moveTimer/dirChangeFreq);
        box.placeable.SetPosition(currentPos);
    }

    profiler.EndBlock();
}
