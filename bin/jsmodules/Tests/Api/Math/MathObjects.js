print("Testing Math object construction");

// Verify that Math objects can be constructed properly

print("float3");
var f1 = new float3(float3.zero);
var f2 = new float3(1,1,1);

print("Quat");
var q1 = new Quat(Quat.identity);
var q2 = new Quat(0,0,0,1);
var q3 = new Quat(new float3(0,1,0), Math.PI);

print("Math tests finished.");