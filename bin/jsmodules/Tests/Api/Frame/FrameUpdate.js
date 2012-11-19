print("Loading FrameUpdate.js");

frame.Updated.connect(OnFrameUpdated);

function OnFrameUpdated(frameTime)
{
    print("Time elapsed since last frame " + frameTime);
}
