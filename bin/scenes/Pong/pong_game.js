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
//r.linearVelocity = new Vector3df(1.0, 0, 0);
//XXX TODO: doesn't work for some reason, test more and check internals
//works from ecedit gui but not with this call.
//r.SetLinearVelocity(new Vector3df(1.0, 0, 0));

//var MAXSPEED = 10.0;
var motion_x = 1.0;
var motion_y = 0.2;
var speed = 10.0;
var prev_collpos = zerovec;

function notprevcoll(pos) {
  return (Math.abs(prev_collpos.x - pos.x) > 1 ||
          Math.abs(prev_collpos.y - pos.y) > 1);
}

function autopilot() {
  var ta = bat_a.placeable.transform;
  var tb = bat_b.placeable.transform;
  var ball_y = ball.placeable.transform.pos.y;
  ta.pos.y = ball_y;
  tb.pos.y = ball_y;
  bat_a.placeable.transform = ta;
  bat_b.placeable.transform = tb;
}

function handlecoll(other, pos, nor, dist, imp, newcoll) {
//PhysicsCollision(Scene::Entity* otherEntity, const Vector3df& position, const Vector3df& normal, float distance, float impulse, bool newCollision);
  print("---coll:---");
  print(pos.x + " : " + pos.y);
  if (notprevcoll(pos)) {
    prev_collpos = pos;
    //print(nor);
    print(nor.x + " : " + nor.y);

    //motion_x = -nor.x;
    //motion_y = -nor.y;
    if (Math.abs(nor.x) == 1) {
      motion_x = -motion_x;
    }
    else if (Math.abs(nor.y) == 1) {
      motion_y = -motion_y;
    }
    else {
      print("unknown coll nor -- not straight wall?");
    }
  }
  else {
    ball.rigidbody.mass = 0;
  }

  t.rot = zerovec;

  print(motion_x + " - " + motion_y);
  print("/");
}

function update(dt) {
  dt = Math.min(0.1, dt);
  //get very annoying pauses of several seconds now on linux laptop
  //-- this at least makes it so that the ball doesn't escape after pauses

  var placeable = ball.placeable;
  var rigidbody = ball.rigidbody;

  /*var velvec = rigidbody.GetLinearVelocity();
  print(velvec.x);
  var curspeed = Math.abs(velvec.x); //Math.sqrt(rigidbody.GetLinearVelocity().x
  var curdir = new Vector3df(); //null; //velvec.Normalize()
  print(curspeed);*/

  t = placeable.transform;

  if (notprevcoll(t.pos)) {
    ball.rigidbody.mass = 1.0;
  }

  if ((motion_x != 0) || (motion_y != 0)) {
      //var mag = 1.0; // / Math.sqrt(motion_x * motion_y); // + curdir.x * curdir.y);
      /*var impulseVec = new Vector3df();
      print(mag + " - " + curdir.x);
      impulseVec.x = mag * move_force * motion_x; //curdir.x;
      impulseVec.y = -mag * move_force * motion_y; //curdir.y;
      impulseVec = placeable.GetRelativeVector(impulseVec);
      rigidbody.ApplyImpulse(impulseVec);*/

      t.pos.x += speed * motion_x * dt;
      t.pos.y += speed * motion_y * dt;
      placeable.transform = t;
  }

  autopilot();
}

ball.rigidbody.PhysicsCollision.connect(handlecoll);
frame.Updated.connect(update);

print("-*-");
