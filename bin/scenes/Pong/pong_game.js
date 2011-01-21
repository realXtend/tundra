print("yo! here we go..");

var ball = scene.GetEntityByNameRaw("ball");
print(ball.Id);

var pad_a = scene.GetEntityByNameRaw("pad_a");
var pad_b = scene.GetEntityByNameRaw("pad_b"); //$('pad_b') - how about?

var t = ball.placeable.transform;
zerovec = new Vector3df();
t.pos = zerovec;
t.rot = zerovec;
ball.placeable.transform = t;

var r = ball.rigidbody;
//r.linearVelocity = new Vector3df(1.0, 0, 0);
r.SetLinearVelocity(new Vector3df(1.0, 0, 0));
print(r.GetLinearVelocity().x);

var motion_x = 1.0;
var motion_y = 0.0;
var move_force = 0.1;

function handlecoll(other, pos, nor, dist, imp, newcoll) {
//PhysicsCollision(Scene::Entity* otherEntity, const Vector3df& position, const Vector3df& normal, float distance, float impulse, bool newCollision);
  //print(nor);
  //print(nor.x() + " : " + nor.y());
  //print(pos.x() + " : " + pos.y());
  print(other.Id);
  //most ultimately lame HACK!
  if(other.Id == pad_b.Id)
    motion_x = -1.0;
  if(other.Id == pad_a.Id)
    motion_x = 1.0;

  t.rot = zerovec;

  print(motion_x);
}

function update(dt) {
  var placeable = ball.placeable;
  var rigidbody = ball.rigidbody;

  if ((motion_x != 0) || (motion_y != 0)) {
    var mag = 1.0 / Math.sqrt(motion_x * motion_x + motion_y * motion_y);
    var impulseVec = new Vector3df();
    impulseVec.x = mag * move_force * motion_x;
    impulseVec.y = -mag * move_force * motion_y;
    impulseVec = placeable.GetRelativeVector(impulseVec);
    rigidbody.ApplyImpulse(impulseVec);
  }
}

ball.rigidbody.PhysicsCollision.connect(handlecoll);
frame.Updated.connect(update);

print("-*-");
