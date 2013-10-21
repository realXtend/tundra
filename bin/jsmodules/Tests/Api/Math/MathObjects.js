print("");
print("Testing Math object construction.");
print("");
// Verify that Math objects can be constructed properly

print("*** float3 ***");
var f1 = new float3(float3.zero);
print("new float3(float3.zero): " + f1);
var f2 = new float3(1,1,1);
print("new float3(1,1,1): " + f2);
print("");

print("*** Quat ***");
var q1 = Quat.identity;
print("Quat.identity: " + q1);
var q2 = new Quat(0,0,0,1);
print("new Quat(0,0,0,1): " + q2);
var q3 = new Quat(new float3(0,1,0), Math.PI);
print("new Quat(new float3(0,1,0), Math.PI): " + q3);
print("");

print("*** Color ***");
var c1 = new Color(0.1, 0.2, 0.3);
print("new Color(0.1, 0.2, 0.3): " + c1);
var c2 = new Color(0.1, 0.2, 0.3, 0.4);
print("new Color(0.1, 0.2, 0.3, 0.4): " + c2);
var c3 = Color.Magenta;
print("Color.Magenta: " + c3);
print("");

print("Math tests finished.");