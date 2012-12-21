print("Loading 3d sound example script.");

var audio_asset_ref = "b31a9a52-fb05-4648-87de-60c8697db3ca";
// Sound file should be physically located at \bin\data\assets.
var local_audio = "file://rapinaa.ogg";

// Javascript engine will call this function before it destroy the script instance.
function OnScriptDestroyed()
{
    if (sound_rotator != undefined)
        sound_rotator.Stop();
}

//--------------------------------
//------DEFINE SOUND ROTATOR------
//--------------------------------
function SoundRotator(sound_ref, radius, speed, comp)
{
    this.SoundRef = sound_ref;    //Sound resource id or local asset name.
    this.LoopSound = true;        //Loop the sound
    this.Radius = radius;        //Distance from the origin.
    this.Speed = speed;         //Rotation speed.
    this.component = comp;        //EC_Placeable compoent.
    this.Rotation = 0.0;         //Current rotation in degreeses.
    this.SoundID = 0;            //Sound service id for sound asset.
}

SoundRotator.prototype.Start = function()
{
    var position = this.component.Position;
    this.SoundID = audio.PlaySound3D(this.SoundRef, 0, false, position);
    audio.SetLooped(this.SoundID, this.LoopSound);
    frame.Updated.connect(sound_rotator, sound_rotator.Update);
}

SoundRotator.prototype.Stop = function()
{
    audio.StopSound(this.SoundID);
    this.SoundID = 0;
    frame.Updated.disconnect(sound_rotator, sound_rotator.Update);
}

SoundRotator.prototype.Update = function(frame_time)
{
    var Origin = me.placeable.Position;
    if (Origin != undefined)
    {
        Origin.x += Math.sin(this.Rotation) * this.Radius;
        Origin.y += Math.cos(this.Rotation) * this.Radius;
        audio.SetPosition(this.SoundID, Origin);
    }
    
    var trans = me.mesh.nodeTransformation;
    if (trans != undefined)
    {
        trans.pos.x = Math.sin(this.Rotation) * this.Radius;
        trans.pos.y = Math.cos(this.Rotation) * this.Radius;
        var mode = me.mesh.GetUpdateMode();
        mode.value = 2;
        me.mesh.SetUpdateMode(mode);
        me.mesh.nodeTransformation = trans;
    }
    
    this.Rotation = this.Rotation + (this.Speed * frame_time);
    if (this.Rotation >= 360.0)
        this.Rotation -= 360.0;
    else if(this.Rotation <= -360.0)
        this.Rotation += 360.0;
}

// Wait until EC_Placeable object has been added to entity.
function componentAdded(entity, component)
{
    if (entity.id != me.id && component.typeName != "EC_OgrePleacable")
        return;
    
    sound_rotator = new SoundRotator(local_audio, 9.0, 1.0, component);
    sound_rotator.Start();
}

var sound_rotator = 0;
if(me.placeable)
    componentAdded(me, me.placeable);
else //If user login to server and script is run before EC_Placeable has been created this ensures that we get the actual component when it's added to scene.
    scene.ComponentAdded.connect(componentAdded);
