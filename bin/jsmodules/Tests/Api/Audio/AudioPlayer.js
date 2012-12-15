print("Loading AudioPlayer example script.");

// Javascript engine will call this function before it destroy the script instance.
function OnScriptDestroyed()
{
    //No cleaning atm.
}

//------------------------------
//------DEFINE AUDIOPLAYER------
//------------------------------
function AudioPlayer(filepath)
{
    // ENUMS
    this.SoundTypeEnum  = {"Triggered" : 0, "Ambient" : 1, "Voice" : 2}; 
    this.SoundStateEnum = {"Stopped" : 0, "Pending" : 1, "Playing" : 2};
    
    // MEMBER VARIABLES
    this.AudioRef    = filepath;     // Audio file id or physical location.
    this.LocalFile   = true;         // Do we request sound asset from the server.
    this.SoundType   = this.SoundTypeEnum.Ambient;
    this.SoundState  = this.SoundStateEnum.Stopped;
    this.FadeTime    = 1;             // Fade time in seconds.
    this.FadeCurTime = 0;             // Current fade time to check when we can stop sound fade effect.
    this.SoundID     = 0;             // Sound service will return sound_id when we begin to play it.
    this.OldSoundID  = 0;             // When we want to switch two sounds in smooth way this is used.
}

// Play audio clip as ambient sound.
AudioPlayer.prototype.PlaySound = function(audio_ref)
{
    if (audio_ref == null)
        audio_ref = this.AudioRef;
    this.SoundID = audio.PlaySound(audio_ref, this.SoundType, this.LocalFile);
    this.SoundState = this.SoundStateEnum.Playing;
    this.SoundType = this.SoundTypeEnum.Ambient;
}
    
// Position should be a float3 that contain x, y and z value as in int format.
AudioPlayer.prototype.PlaySound3D = function(position, audio_ref)
{
    if (audio_ref == null)
        audio_ref = this.AudioRef;
    this.SoundID = audio.PlaySound3D(audio_ref, this.SoundType, this.LocalFile, position);
    this.SoundState = this.SoundStateEnum.Playing;
    this.SoundType = this.SoundTypeEnum.Triggered;
}
    
AudioPlayer.prototype.StopSound = function(sound_id)
{
    if (sound_id == null)
        sound_id = this.SoundID;
    audio.StopSound(sound_id);
    this.SoundState = this.SoundStateEnum.Stopped;
}
    
//Volume value should be between 0.0 - 1.0.
AudioPlayer.prototype.SetVolume = function(volume, sound_id)
{
    if (sound_id == null)
        sound_id = this.SoundID;
    audio.SetGain(sound_id, volume);
}
    
//Pitch value should be between 0.0 - 1.0.
AudioPlayer.prototype.SetPitch = function(pitch, sound_id)
{
    if (sound_id == null)
        sound_id = this.SoundID;
    audio.SetPitch(sound_id, pitch);
}
    
//Set 3D sound position.
AudioPlayer.prototype.SetPosition = function(pos, sound_id)
{
    if (sound_id == null)
        sound_id = this.SoundID;
    audio.SetPosition(sound_id, pos);
}
    
//If we want to make sure that sound asset is ready when we are playing it we can use this before we start playing the sound.
//This method is not needed if we are using local sound assets.
AudioPlayer.prototype.RequestSoundAsset = function(audio_ref)
{
    if (audio_ref == null)
        audio_ref = this.AudioRef;
    audio.RequestSoundResource(audio_ref);
}

//Switch to another sound should do transform smoothly.
AudioPlayer.prototype.SwitchToSound = function(sound_ref)
{
    this.OldSoundID = this.SoundID;
    this.PlaySound(sound_ref);
    this.SetVolume(0.0, this.SoundID);
    frame.DelayedExecute(0.1).Triggered.connect(this, this.FadeSounds);
}

//fucntion for decreasing old sound and increasing a new sound volume values.
AudioPlayer.prototype.FadeSounds = function()
{
    if(this.FadeCurTime >= this.FadeTime)
    {
        this.FadeCurTime = 0;
        this.StopSound(this.OldSoundID);
        this.OldSoundID = 0;
        return;
    }
    var step = 0.1 / this.FadeTime
    this.FadeCurTime = this.FadeCurTime + 0.1;
    var gainValue = this.FadeCurTime / this.FadeTime;
    this.SetVolume(1.0 - gainValue, this.OldSoundID);
    this.SetVolume(gainValue, this.SoundID);
    frame.DelayedExecute(0.1).Triggered.connect(this, this.FadeSounds);
}

var audio_asset_ref = "b31a9a52-fb05-4648-87de-60c8697db3ca";
// Sound file should be physically located at \bin\data\assets.
var local_audio  = "file://rat-a-tat.ogg";
var local_audio2 = "file://rat-a-tat.ogg";

var sound_player = new AudioPlayer(local_audio);
// We use local sound asset in this example. set this to true if you want to use asset references.
sound_player.LocalFile = false;
// We could request sound asset before the actual sound is played. This is recomented if we want to sound be played as fast as possible.
//sound_player.RequestSoundAsset();
sound_player.PlaySound();
//Switch from sound to another by slowly faiding those two sounds.
sound_player.SwitchToSound(local_audio2);
