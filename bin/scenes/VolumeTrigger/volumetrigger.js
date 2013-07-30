var ACTIVE_MATERIAL = "local://green.material";
var UNACTIVE_MATERIAL = "local://red.material";

var isServer = server.IsRunning();

print("[Volume Trigger test] Loading script...");

function print(s) {
    console.LogInfo(s);
}

function entityEntered(ent) {
    print("Entity has entered the cube! " + ent.name);
   
    var mats = me.mesh.meshMaterial;
    mats[0] = ACTIVE_MATERIAL;
    me.mesh.meshMaterial = mats;	
    
}

function entityLeft(ent){ 
    print("Entity has left the cube! " + ent.name );
 
    var mats = me.mesh.meshMaterial;
    mats[0] = UNACTIVE_MATERIAL;
    me.mesh.meshMaterial = mats;
       
}

var vol = me.GetOrCreateComponent("VolumeTrigger");
        
print(vol.entityEnter.connect(entityEntered));
print(vol.entityLeave.connect(entityLeft));
        
var rigid = me.GetOrCreateComponent("RigidBody");
rigid.phantom = true;
      
mats = me.mesh.meshMaterial;
mats[0] = UNACTIVE_MATERIAL;    
me.mesh.meshMaterial = mats;
