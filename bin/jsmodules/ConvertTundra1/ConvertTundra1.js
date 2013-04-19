if (!framework.IsHeadless())
{
    engine.ImportExtension("qt.core");
    engine.ImportExtension("qt.gui");

    var menu = ui.MainWindow().menuBar();
    var convertMenu = menu.addMenu("&Convert");
    if (convertMenu != null)
        convertMenu.addAction("Convert Tundra 1 scene").triggered.connect(ConvertScene);
}

function ConvertScene()
{
    var rotateMatrix = float3x4.FromEulerZYX(0, 0, -Math.PI/2);
    var meshRotateMatrix = float3x4.FromEulerZYX(Math.PI, 0, -Math.PI/2);

    var entities = framework.Scene().MainCameraScene().EntitiesWithComponent("EC_Placeable");
    print("Scene has " + entities.length + " positional entities");
    for (var i = 0; i < entities.length; ++i)
    {
        var entity = entities[i];
        if (entity.name == "FreeLookCamera")
        {
            print("Skipping freelookcamera");
            continue;
        }
        print("Processing entity " + entity.name);

        // Get correct world rotation & position
        var placeable = entity.GetComponent("EC_Placeable");
        var transform = placeable.transform;
        var matrix = transform.ToFloat3x4();
        var resultMatrix = rotateMatrix.Mul(matrix);
        transform.FromFloat3x4(resultMatrix);
        placeable.transform = transform;
        
        // Fix the mesh node transform if exists
        var mesh = entity.GetComponent("EC_Mesh");
        if (mesh != null)
        {
            var nodeTransform = mesh.nodeTransformation;
            var rotQuat = nodeTransform.Orientation().Inverted();
            
            nodeTransform.rot.x = 0;
            nodeTransform.rot.y = 0;
            nodeTransform.rot.z = 0;
            mesh.nodeTransformation = nodeTransform;

            transform.SetOrientation(transform.Orientation().Mul(rotQuat));
            placeable.transform = transform;
        }
    }
}