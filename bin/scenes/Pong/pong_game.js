print("yo! here we go..");

var ball = scene.GetEntityByNameRaw("ball");
print(ball.Id);

var bat_a = scene.GetEntityByNameRaw("bat_a");
var bat_b = scene.GetEntityByNameRaw("bat_b"); //$('bat_b') - how about?

var t = ball.placeable.transform;
zerovec = new Vector3df();
t.pos = zerovec;
t.rot = zerovec;
ball.placeable.transform = t;

var r = ball.rigidbody;
//r.linearVelocity = new Vector3df(1.0, 0, 0); //XXX NOTE: this fails *silently*
var v = new Vector3df();
v.x = 1.0;
r.SetLinearVelocity(v);

//var MAXSPEED = 10.0;
var motion_x = 1.0;
var motion_y = 0.2;
var speed = 16.0;
var prev_collpos = zerovec;

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
  dt = Math.min(0.1, dt);
  //get very annoying pauses of several seconds now on linux laptop
  //-- this at least makes it so that the ball doesn't escape after pauses

  var placeable = ball.placeable;
  var rigidbody = ball.rigidbody;

  var velvec = rigidbody.GetLinearVelocity();
  //print(velvec.x);
  var curspeed = Math.abs(velvec.x); //Math.sqrt(rigidbody.GetLinearVelocity().x
  dirvec = velvec.normalize();
  print(dirvec.x);
  var curdir = new Vector3df(); //null; //velvec.Normalize()
  //print(curspeed);

  if (velvec.x < 0)
    velvec.x = -1 * speed;
  else
    velvec.x = 1 * speed;
    
  r.SetLinearVelocity(velvec);

    /* placeable hack -- hopefully not needed anymore 'cause linearVel works
      t.pos.x += speed * motion_x * dt;
      t.pos.y += speed * motion_y * dt;
      placeable.transform = t;*/

  autopilot();
}

//ball.rigidbody.PhysicsCollision.connect(handlecoll);
frame.Updated.connect(update);

print("-*-");
