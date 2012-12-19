
if (!apiversion)
{
    print("API version < 1.0. - before api had version info.");
}
else
{
    print("API version:" + apiversion.version);

    if (apiversion.major < 1 && apiversion.minor > 5)
        print("api version > 0.5 - at least release 1.0.7")
    if (apiversion.major >= 1)
        print("tundra sdk is frozen api 1.0 - now we are golden! ?");
    if (apiversion.major > 1)
        print("major api version >1? this really happened some day? did we break a lot?");
}

if (!applicationversion)
    print("API version < 1.0. - before api had application version info.");
else
    print("Tundra application identifier is " + application.fullIdentifier);
