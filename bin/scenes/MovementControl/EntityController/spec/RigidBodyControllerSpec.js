engine.IncludeFile("local://RigidBodyController.js");

describe("Tundra-RigidBodyController", function() {
	//Used variables among specs
	var placeable = me.GetOrCreateComponentRaw("EC_Placeable");
	var controller;
	var scene_entities = [];
	
	//Restore transform before each spec
	beforeEach(function () {
		var transform = placeable.transform;
		transform.pos.x = 0;
		transform.pos.y = 0;
		transform.pos.z = 0;
		transform.rot.x = 0;
		transform.rot.y = 0;
		transform.rot.z = 0;
		placeable.transform = transform;
	  });
	
	//Setup spec: load scene and create avatar components
	it("Setup", function() {
		//Load required scene
		var test_dir;
		var name = me.name.name;
		if (name.substring(0,15) == "JasmineTestPath")
			test_dir = name.substring(16);
		else
		{
			test_dir = asset.assetdir;
		}
		scene_entities = scene.LoadSceneXML(test_dir + "/spec/RigidBodyControllerTestScene.txml",
				false, false, 0);
		
		//Avatar
		var avatar = me.GetOrCreateComponentRaw("EC_Avatar");
		expect(avatar).toBeDefined();
		avatar.appearanceId = "http://subwiki/contenidoMV/assets/default_avatar.xml";
		
		//RigidBody
		var rigidbody = me.GetOrCreateComponentRaw("EC_RigidBody");
		expect(rigidbody).toBeDefined();
		var sizeVec = new Vector3df();
		sizeVec.z = 2.4;
		sizeVec.x = 0.5;
		sizeVec.y = 0.5;
		rigidbody.mass = 10;
		rigidbody.shapeType = 3; // Capsule
		rigidbody.size = sizeVec;
		rigidbody.friction = 0.3;
		var angularVec = new Vector3df();
		angularVec.z = 1;
		rigidbody.angularFactor = angularVec; // Set zero angular factor so that body stays upright
		
		//RigidBodyController
		controller = new realXtend.EntityController.RigidBodyController();
		expect(controller).toBeDefined();
		
		//Wait 2 seconds for the avatar to fall onto floor
		waits(2000);
	});
	
	//Moves forward spec
	it("Moves forward", function() {
		
		this.after(function(){ me.Exec(1, "Move", 0, 0, 0); });
		
		runs(function () {
			me.Exec(1, "Move", 1, 0, 0);
		});
		
		waits(2000);
		
		runs(function () {
			expect(placeable.transform.pos.x).toBeGreaterThan(0);
		});
	});
	
	//Cleanup spec: remove avatar componentes
	it("Cleanup", function() {
		controller.Destroy();
		delete controller;
				
		me.RemoveComponent("EC_RigidBody");
		expect(me.HasComponent("EC_RigidBody")).toBeFalsy();
		
		me.RemoveComponent("EC_Avatar");
		expect(me.HasComponent("EC_Avatar")).toBeFalsy();
		
		me.RemoveComponent("EC_AvatarAppearance");
		expect(me.HasComponent("EC_AvatarAppearance")).toBeFalsy();
		
		me.RemoveComponent("EC_Mesh");
		expect(me.HasComponent("EC_Mesh")).toBeFalsy();
		
		me.RemoveComponent("EC_Placeable");
		expect(me.HasComponent("EC_Placeable")).toBeFalsy();
		
		for (i in scene_entities)
		{
			scene.RemoveEntityRaw(scene_entities[i].Id);
		}
	});

});