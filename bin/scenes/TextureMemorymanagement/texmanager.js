function print(s) {
    console.LogInfo(s);
}

print("hop");

ow = scene.ogre;
print(ow.GetVisibleEntities());

ent = ow.GetVisibleEntities()[0];
print(ent);
print(ent.mesh);
print(ent.mesh.meshMaterial[0]);

var matrefs = ent.mesh.meshMaterial;
var matref = matrefs[0];
print(matref);

ent.mesh.meshMaterial = ["local://AxisRed.material"];

//var matass = asset.GetAsset(ref);
//print(matass.References());

var textures = [
    "http://www.realxtend.org/world/lvm/MAP_deer_G-03.png",
    "http://www.realxtend.org/world/lvm/MAP_deer_G-03_SPEC.png",
    "http://www.realxtend.org/world/lvm/MAP_deer_G-03_NRM.png"
];

function unload(assurl) {
    var ass = asset.GetAsset(assurl);
    ass.Unload();
    print("unloaded " + assurl);
}

unload(matref);
for(i in textures) {
    unload(textures[i]);
}

print("done!");
