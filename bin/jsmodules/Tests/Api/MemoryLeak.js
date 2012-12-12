var ent;
var myscene;

function init() {
    myscene = framework.DefaultScene();
    ent = myscene.CreateEntity(myscene.NextFreeId(), ['EC_Placeable']);
}

//doesn't actually leak -- couldn't reproduce a leak here yet.
//but free cam leaks! misobserved that here first, before removed it from startup.
function no_leak(dt) {
    if (!myscene) init();

    ent = myscene.GetEntity(ent.id);
    var x;
    for(var i=0; i < 1000; i++) {
	var t = ent.placeable.transform;
	x = t.pos.x;
    }
    print(x);
}
/* (this was actually due to freecam)
  PID USER      PR  NI  VIRT  RES  SHR S %CPU %MEM    TIME+  COMMAND            
 2803 antont    20   0  939m 161m  82m R   92  4.1   1:26.02 server             

  PID USER      PR  NI  VIRT  RES  SHR S %CPU %MEM    TIME+  COMMAND            
 2803 antont    20   0  945m 168m  82m R   92  4.2   8:53.39 server            
*/

function createvec(dt) {
    var v;
    for(var i=0; i < 1000; i++) {
	v = new float3();
	v.x = 1.2;
    }
    print(v.x);
}

function noop(dt) {
}
/* stays at:
 PID USER      PR  NI  VIRT  RES  SHR S %CPU %MEM    TIME+  COMMAND            
2892 antont    20   0  901m 143m  81m R   52  3.6   1:59.48 server             
*/

//run = noop;
//run = no_leak;
run = createvec;

frame.Updated.connect(run);


