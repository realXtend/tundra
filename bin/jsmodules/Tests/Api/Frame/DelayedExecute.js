print("Loading DelayedExecute.js");

function DelayedMessage(time)
{
    print(time + " seconds has passed.");
}

frame.DelayedExecute(1.0).Triggered.connect(DelayedMessage);
