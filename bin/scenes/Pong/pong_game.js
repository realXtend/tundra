print("yo! here we go..");

var ball = scene.GetEntityByNameRaw("ball");
print(ball.Id);

var bat_a = scene.GetEntityByNameRaw("bat_a");
var bat_b = scene.GetEntityByNameRaw("bat_b"); //$('bat_b') - how about?

//nice to have in code to reinit correctly after live updates when devving
var t = ball.placeable.transform;
zerovec = new Vector3df();
t.pos = zerovec;
t.rot = zerovec;
ball.placeable.transform = t;

//r.linearVelocity = new Vector3df(1.0, 0, 0); //XXX NOTE: this fails *silently*
var v = new Vector3df();
v.x = 1.0;
v.y = 0.2;
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
YAY not needed anymore (?)*/

function update(dt) {
  var placeable = ball.placeable; //note: could and probably should keep refs to these, and not fetch every time
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
