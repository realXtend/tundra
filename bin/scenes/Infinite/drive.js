//!ref: road_a.material
//!ref: road_b.material

print("<-");

var latest_y = 0;
var materials = ["axis_black", "BlueTransparent"];
//seems that can't assign mat assets from js, so have to use by-default loaded ones. 
//var materials = ["road_a", "road_b"];

//function addmesh(meshname, pos, ..)

function addpiece(y) {
    var roadpiece = scene.CreateEntityRaw(scene.NextFreeId(),
                                          ["EC_Placeable", "EC_Mesh"]);

    //set a mesh ref. we really need to streamline this.
    //one option is js written wrapper i guess, but perhaps some setter in same place does it too
    var ref = roadpiece.mesh.meshRef;
    ref.ref = "rect_plane.mesh";
    roadpiece.mesh.meshRef = ref;

    var mat = materials[Math.floor(Math.random() * materials.length)]; //random.choice
    print(mat);
    roadpiece.mesh.SetMaterial(0, mat); // + ".material");

    var t = roadpiece.placeable.transform;
    t.pos.y = y;
    t.scale.z = 30;
    roadpiece.placeable.transform = t;
    print(roadpiece.placeable.transform.pos.y);

    scene.EmitEntityCreatedRaw(roadpiece);

    //copy paste from above, didn't make addmesh() yet 'cause seemed that would need many params
    var tree = scene.CreateEntityRaw(scene.NextFreeId(),
                                     ["EC_Placeable", "EC_Mesh"]);
    var ref = tree.mesh.meshRef;
    ref.ref = "sassafras.mesh";
    tree.mesh.meshRef = ref;

    var t = tree.placeable.transform;
    t.pos.y = y + 30;
    var x = 0.6 //3.15;
    if (Math.random() > 0.5) {
        x = -x;
    }
    t.pos.x = x;
    tree.placeable.transform = t;

    scene.EmitEntityCreatedRaw(roadpiece);
}

var car = null; //freecam is apparently not there yet when loading this, so gotta fetch later
function on_carmove(attr, type) {
    if (car.placeable.transform.pos.y > latest_y) {
        latest_y += 30;
        addpiece(latest_y);
    }
}

function init(d) {
    if (!car) {
        var cam = scene.GetEntityByNameRaw("FreeLookCamera");
        if (cam) {
            car = cam;
            var t = car.placeable.transform;
            t.rot.x = 90;
            t.rot.y = 0;
            t.rot.z = 0;
            t.pos.x = 0.19;
            t.pos.y = 4.53;
            t.pos.z = 0.08;
            car.placeable.transform = t;
            print("cam/car set to" + car.placeable.transform.rot.x);

            car.placeable.OnAttributeChanged.connect(on_carmove);
        }
        else {
          print(cam);
        }
    }
}

addpiece(0); //so that know where to start driving :)
frame.Updated.connect(init);

print("->");