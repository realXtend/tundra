print("Loading frame example script.");

function delayedMessage(time)
{
    print(time + " seconds has passed.");
}

var delayed = frame.DelayedExecute(1.0);
delayed.Triggered.connect(delayedMessage);