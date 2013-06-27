//var ACTIVE_MATERIAL = "local://green.material";
//var UNACTIVE_MATERIAL = "local://red.material";

//var isServer = server.IsRunning();

print("[ScenePartManager] Loading script...");

function print(s) {
    console.LogInfo(s);
}

var parts = scene.EntitiesWithComponent("EC_DynamicComponent", "ScenePart");
//print(parts);

var partfile;
for (var i=0; i<parts.length; i++) {
    //print(i + ":" + parts[i]);

    var placeholder = parts[i];
    partfile = placeholder.dynamiccomponent.GetAttribute("sceneref");
    loadPart(placeholder, partfile);
}

function loadPart(placeholder, partfile) {
    print(placeholder + ":" + partfile);
    scene.LoadSceneXML("scenes/CompositeScene/" + partfile, false, false, 2); //, changetype);
}

//var vol = me.GetOrCreateComponent("EC_VolumeTrigger");
