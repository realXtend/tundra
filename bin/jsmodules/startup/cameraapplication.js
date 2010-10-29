// A startup script that hooks to scene added & scene cleared signals, and creates a freelook camera to the scene.

function OnSceneAdded(scenename)
{
    // Get pointer to scene through framework
    scene = framework.Scene(scenename);
    print("Scene " + scene.Name + " has been created");


    // Connect scene clear signal
    scene.SceneCleared.connect(OnSceneCleared);
}

function OnSceneCleared(scene)
{
    print("Scene " + scene.Name + " has been cleared");
}

framework.SceneAdded.connect(OnSceneAdded);
