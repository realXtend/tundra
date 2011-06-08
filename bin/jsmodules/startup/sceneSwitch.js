var SceneAPI = framework.Scene();

SceneAPI.DefaultWorldSceneChanged.connect(SceneChanged);

function SceneChanged(scene)
{
	print("Default scene changed!");
	if(scene==null)
	{
		print("Scene was null!");	
		return;
	}

	var	cameraEntity = scene.GetEntityByNameRaw("AvatarCamera");
	
	if(cameraEntity==null)
	{
		print("Camera entity was null! Getting FreeLookCamera.");
		cameraEntity = scene.GetEntityByNameRaw("FreeLookCamera");
	}

	if(cameraEntity==null)
	{
		print("CameraEntity null... returning!");
		return;
	}

	camera = cameraEntity.GetComponentRaw("EC_OgreCamera");
	
	if(camera==null)
	{
		print("EC_OgreCamera null!");
		return;
	}

	print("Setting camera active!");
	camera.SetActive();
}
