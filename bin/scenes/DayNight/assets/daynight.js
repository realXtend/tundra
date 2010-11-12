/// This script assumes that it is in same entity where EC_EnvironmentLight component exist.
var environment_ = me.GetComponentRaw("EC_EnvironmentLight");
environment_.currentTimeAttr = 0.0;
environment_.fixedTimeAttr = true;
var speed_ = 1.0 / 60.0;

function Update(frametime) {

    if (environment_ != null) {
        // Day-night-cycle goes from 0-1.0
        var time = environment_.currentTimeAttr;
        time += speed_ * frametime;
        if (time > 1.0) {
            environment_.currentTimeAttr = 0.0;
        }
        else {
            environment_.currentTimeAttr = time;
        } 
    }
    
    

}

frame.Updated.connect(Update);