print("Loading sound example script.");

var sound_id = sound.PlaySound("b31a9a52-fb05-4648-87de-60c8697db3ca");
var pitch = sound.GetPitch(sound_id)
print("Sound's original pitch was " + pitch + " and setting it to " + pitch * 0.5);
sound.SetPitch(sound_id, pitch * 0.5);

frame.DelayedExecute(0.1).Triggered.connect(fadeSound);

function fadeSound(time)
{
    var gain = sound.GetGain(sound_id) * 0.9;
    sound.SetGain(sound_id, gain);

    // Check if sound is already near to zero and it's ok to stop the sound.
    if(gain >= 0.05)
    {
        frame.DelayedExecute(0.1).Triggered.connect(fadeSound);
    }
    else
    {
        print("Stop sound");
        sound.StopSound(sound_id);
    }
}