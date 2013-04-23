
function log(msg)
{
    console.LogInfo("[Tests::VercionCheck]: " + msg);
}

print("");

if (!apiversion)
    log("API version < 1.0, cannot query API version!");
else
    log("API version: string=" + apiversion.version + " major=" + apiversion.major + " minor=" + apiversion.minor + " major patch=" + apiversion.majorPatch + " minor patch=" + apiversion.minorPatch);

if (!applicationversion)
    log("API version < 1.0, cannot query Application information!");
else
    log("Tundra application identifier = " + application.fullIdentifier);

print("");