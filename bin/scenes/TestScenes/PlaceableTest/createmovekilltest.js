// use random values for creation and moving
useRandom = true;

var isserver = server.IsRunning();
var outputEnabled = true;
var entityName = "box";

if(useRandom == true){
    //initial location
    var x1 = Math.floor(10*Math.random());
    var y1 = Math.floor(10*Math.random());
    var z1 = Math.floor(10*Math.random());
    //move to
    var x2 = Math.floor(10*Math.random());
    var y2 = Math.floor(10*Math.random());
    var z2 = Math.floor(10*Math.random());
}
else{
    //use integers, doubles seem to cause differences in client
    //initial location
    var x1 = 0;
    var y1 = 6;
    var z1 = 0;
    //move to
    var x2 = 0;
    var y2 = 10;
    var z2 = 0;
}

var randomnumber=Math.floor(Math.random()*100);
if(isserver == false){
    print("client");
    //client.Login("localhost",2345,"Guest"+randomnumber.toString()," ","udp");
    var scene = framework.Scene();
    var myScene = scene.MainCameraScene();
    main();
}
else{
    print("server");
    var myScene = scene;
    main();
}

function quit(){
    framework.Exit();
}

function createEntity(entityName, x, y, z){
    var pos_x = x;
    var pos_y = y;
    var pos_z = z;
    var entityId = myScene.NextFreeId();
    if(outputEnabled){
        print("ACTION: Creating a box(id: " + entityId +") at (x,y,z): " + pos_x + ", " + pos_y + ", " + pos_z);
    }
    var entity = myScene.CreateEntity(entityId, ["EC_Placeable", "EC_Mesh"]);
    entity.SetTemporary(true);
    entity.SetName(entityName);
    entity.mesh.SetMeshRef("local://box.mesh");

    // Set starting position for the entity
    var placeable = entity.placeable;
    var transform = placeable.transform;
    transform.pos.x = pos_x;
    transform.pos.y = pos_y;
    transform.pos.z = pos_z;
    placeable.transform = transform;
    
    //scene.EmitEntityCreated(entity);
}

function checkEntityLocation(entityName, x, y, z){
    var entity = myScene.GetEntityByName(entityName);
    if(entity != null){
        var xbool = false;
        var ybool = false;
        var zbool = false;
        
        if(entity.placeable.transform.pos.x == x){
            xbool = true;
        }
        if(entity.placeable.transform.pos.y == y){
            ybool = true;
        }
        if(entity.placeable.transform.pos.z == z){
            zbool = true;
        }
        if(outputEnabled){
            print("Checking if old and new coordinates match");
            print(" x "+ x +":" + entity.placeable.transform.pos.x + " " + xbool);
            print(" y "+ y +":" + entity.placeable.transform.pos.y + " " + ybool);
            print(" z "+ z +":" + entity.placeable.transform.pos.z + " " + zbool);
        }
        if (xbool == true && ybool == true && zbool == true){
            return true;
        }
        else{
            return false;
        }
    }
    else{
        print("no entity found");
    }
}

function moveEntity(ent, x, y ,z){
    var entity = myScene.GetEntityByName(ent);
    if(entity != null){
        if(outputEnabled){
            print("ACTION: Moving box(id: " + entity.id + ") to (x,y,z): " + x + ", " + y + ", " + z);
        }
        var placeable = entity.placeable;
        var transform = placeable.transform;
        transform.pos.x = x;
        transform.pos.y = y;
        transform.pos.z = z;
        placeable.transform = transform;
    }
    else{
        print("No entity found");
    } 
}

function removeEntity(ent){
    var entity = myScene.GetEntityByName(ent);
    if (entity != null){
        if(outputEnabled){
            print("ACTION: Removing entity");
        }
        var entId = entity.id;
        myScene.RemoveEntity(entId);
    }
    else{
        print("No entity found");
    }
}

function isAlive(ent, expected){
    var status = false;
    var entity = myScene.GetEntityByName(ent);
    if (entity == null){
        status = false;
    }
    if (entity != null){
        status = true;
    }
    if(status == expected){
        boolmatch = true;
    }
    if(status != expected){
        boolmatch = false;
    }
    if(outputEnabled){
        print("Entity is alive: " + status);
    }
    return boolmatch;
}

function evaluateResults(test1, test2, test3, test4){
    testResult = false;
    if(!test1){
        print("FAIL: Entity is not alive");
    }
    if(!test2){
        print("FAIL: Coordinate mismatch");
    }
    if(!test3){
        print("FAIL: Coordinate mismatch");
    }
    if(!test4){
        print("FAIL: Entity is still alive");
    }
    else if(test1 && test2 && test3 && test4){
        testResult = true;
        print("All test phases completed succesfully");
    }
    return testResult;
}

function main(){
    if(outputEnabled){
        print("##########################");
    }
    createEntity(entityName, x1, y1, z1);
    test1 = isAlive(entityName, true);
    test2 = checkEntityLocation(entityName, x1, y1, z1);
    moveEntity(entityName, x2, y2, z2);
    test3 = checkEntityLocation(entityName, x2, y2, z2);
    removeEntity(entityName);
    test4 = isAlive(entityName, false);
    if(outputEnabled){
        print("##########################");
    }
    if(outputEnabled){
        print("Result: " + evaluateResults(test1, test2, test3, test4));
    }
    quit();
}

//######################### SLOWTEST
function create(){
    createEntity(entityName, x1, y1, z1);
}
function test1(){
    isAlive(entityName, true);
}
function test2(){
    checkEntityLocation(entityName, x1, y1, z1);
}
function move(){
    moveEntity(entityName, x2, y2, z2);
}
function test3(){
    checkEntityLocation(entityName, x2, y2, z2);
}
function remove(){
    removeEntity(entityName);
}
function test4(){
    isAlive(entityName, false);
}

function main1(){
    frame.DelayedExecute(1).Triggered.connect(this,create);
    frame.DelayedExecute(3).Triggered.connect(this,test1);
    frame.DelayedExecute(6).Triggered.connect(this,test2);
    frame.DelayedExecute(9).Triggered.connect(this,move);
    frame.DelayedExecute(12).Triggered.connect(this,test3);
    frame.DelayedExecute(15).Triggered.connect(this,remove);
    frame.DelayedExecute(18).Triggered.connect(this,test4);
}
//#########################
