/// This script assumes that it is in same entity where EC_EnvironmentLight component exist.

var speed_ = 1.0 / 60.0;

var environment_ = null;

function FindEnvironmentLight()
{
  if (environment_ == null)
  {
    environment_ = me.GetComponent("EC_EnvironmentLight");
    if (environment_ != null)
    {
      environment_.currentTimeAttr = 0.5; // Start at midday.
      environment_.fixedTimeAttr = true; // Don't do hardcoded time animation in C++.
    }
  }
}

function Update(frametime)
{
  FindEnvironmentLight();
  
  if (environment_ != null)
  {
    // Day-night-cycle goes from 0-1.0
    var time = environment_.currentTimeAttr;
    time += speed_ * frametime;
    time = time - Math.floor(time);
    environment_.currentTimeAttr = time;
  }
}

frame.Updated.connect(Update);
