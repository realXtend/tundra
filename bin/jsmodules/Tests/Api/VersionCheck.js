
function log(msg)
{
    console.LogInfo("[Tests::VercionCheck]: " + msg);
}

print("");

log("Application version: " + application.version);
log("  major       = " + application.majorVersion); 
log("  minor       = " + application.minorVersion); 
log("  major patch = " + application.majorPatchVersion);
log("  minor patch = " + application.minorPatchVersion);

print("");