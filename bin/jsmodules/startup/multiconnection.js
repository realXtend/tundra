if (!server.IsRunning())
{
    client.switchScene.connect(changeScene);

    function changeScene(name) 
    {
        scene = framework.Scene().GetScene(name);
        var cameraentity = scene.GetEntityByName("AvatarCamera");
        if (cameraentity == null)
            cameraentity = scene.GetEntityByName("FreeLookCamera");
        var camera = cameraentity.camera;
        camera.SetActive(camera);

    }
}
