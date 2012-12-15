print("Loading simple audio example script.");

var audio_ref = "b31a9a52-fb05-4648-87de-60c8697db3ca";
var sound_id = audio.PlaySound(audio_ref); //Change sound asset id here
var pitch = audio.GetPitch(sound_id)
print("Sound's original pitch was " + pitch + " and setting it to " + pitch * 0.5);
audio.SetPitch(sound_id, pitch * 0.5);

frame.DelayedExecute(0.1).Triggered.connect(fadeSound);
//frame.DelayedExecute(1.0).Triggered.connect(CheckAudioState);

function fadeSound(time)
{
    var gain = audio.GetGain(sound_id) * 0.9;
    audio.SetGain(sound_id, gain);

    // Check if sound is already near to zero and it's ok to stop the sound.
    if(gain >= 0.05)
    {
        frame.DelayedExecute(0.1).Triggered.connect(fadeSound);
    }
    else
    {
        print("Stop sound");
        audio.StopSound(sound_id);
        //frame.DelayedExecute(1.0).Triggered.connect(CheckAudioState);
    }
}

function CheckAudioState()
{
    sound_id = audio.PlaySound(audio_ref);
    audio.SetPitch(sound_id, pitch * 0.5);
    frame.DelayedExecute(0.1).Triggered.connect(fadeSound);
}