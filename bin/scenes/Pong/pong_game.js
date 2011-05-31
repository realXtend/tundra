print("yo! here we go..");

var ball = scene.GetEntityByName("ball");
print(ball.id);

var bat_a = scene.GetEntityByName("bat_a");
var bat_b = scene.GetEntityByName("bat_b"); //$('bat_b') - how about?

//nice to have in code to reinit correctly after live updates when devving
var t = ball.placeable.transform;
zerovec = new Vector3df();
t.pos = zerovec;
t.rot = zerovec;
ball.placeable.transform = t;

//r.linearVelocity = new Vector3df(1.0, 0, 0); //XXX NOTE: this fails *silently*
//initial vel for the ball
var v = new Vector3df();
v.x = 10.0;
v.y = 2;
ball.rigidbody.SetLinearVelocity(v);

var speed = 16.0;

function autopilot() {
  var ta = bat_a.placeable.transform;
  var tb = bat_b.placeable.transform;
  var ball_y = ball.placeable.transform.pos.y;
  ta.pos.y = ball_y;
  tb.pos.y = ball_y;
  bat_a.placeable.transform = ta;
  bat_b.placeable.transform = tb;
}

/*function handlecoll(other, pos, nor, dist, imp, newcoll) {
//PhysicsCollision(Scene::Entity* otherEntity, const Vector3df& position, const Vector3df& normal, float distance, float impulse, bool newCollision);
  print("---coll:---");
  print(pos.x + " : " + pos.y);
YAY not needed anymore,
but can perhaps put custom dir setting related to bat speeds etc here, 
if bullet doesn't do what we want */

function update(dt) {
  /* with rigidbody restitution set to 1.0 this shouldn't theoretically be needed,
     but with even a little mass (and restiturion 2.0) the ball gets really slow after this
  .. so here we just maintain contant speed (and could of course control it in any way) */
  var rigidbody = ball.rigidbody;

  var velvec = rigidbody.GetLinearVelocity();
  var curdir = velvec.normalize();
  velvec = curdir.mul(speed);
  rigidbody.SetLinearVelocity(velvec);

  autopilot();
}

//ball.rigidbody.PhysicsCollision.connect(handlecoll);
frame.Updated.connect(update);

print("-*-");

/*
  PID USER      PR  NI  VIRT  RES  SHR S %CPU %MEM    TIME+  COMMAND            
 2536 antont    20   0 1013m 176m  85m R   71  4.5  24:57.05 server             
 2590 antont    20   0 1010m 176m  84m S   53  4.5   7:59.68 viewer             
*/