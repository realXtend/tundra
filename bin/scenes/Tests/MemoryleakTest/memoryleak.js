//free cam leaks! trying to reproduce here in ec_script 'cause couldn't with --run
function no_leak(dt) {
     var x;
    for(var i=0; i < 1000; i++) {
	var t = me; //ent.placeable.transform;
	x = t; //t.pos.x;
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
/*
  PID USER      PR  NI  VIRT  RES  SHR S %CPU %MEM    TIME+  COMMAND            
 1972 antont    20   0  933m 160m  82m R   77  4.1   0:24.89 server  

  PID USER      PR  NI  VIRT  RES  SHR S %CPU %MEM    TIME+  COMMAND            
 1972 antont    20   0  938m 166m  82m R   78  4.2   2:47.37 server             */

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

/* camera app
  PID USER      PR  NI  VIRT  RES  SHR S %CPU %MEM    TIME+  COMMAND            
 2276 antont    20   0  928m 155m  82m S   40  3.9   0:09.01 server             

  PID USER      PR  NI  VIRT  RES  SHR S %CPU %MEM    TIME+  COMMAND            
 2276 antont    20   0  932m 159m  82m R   99  4.0   0:37.11 server             

 PID USER      PR  NI  VIRT  RES  SHR S %CPU %MEM    TIME+  COMMAND            
 2276 antont    20   0  935m 162m  83m R   55  4.1  14:08.05 server
*/