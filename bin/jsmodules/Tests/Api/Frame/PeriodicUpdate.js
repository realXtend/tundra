print("Loading PeriodicUpdate.js");

frame.DelayedExecute(2.0).Triggered.connect(PeriodicUpdate);

function PeriodicUpdate()
{
    print("This text is printed every two seconds.");
    frame.DelayedExecute(2.0).Triggered.connect(PeriodicUpdate);
}
