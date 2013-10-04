set bundlePath to path to me
set posixBundlePath to POSIX path of bundlePath

-- Default handler when Tundra.app is double-clicked (normal launch)
do shell script posixBundlePath & "Contents/MacOS/Tundra --config tundra-client.json --trustserverstorages --accept_unknown_http_sources --accept_unknown_local_sources --fpslimit 60 --hide_benign_ogre_messages --nofilewatcher > /dev/null 2>&1&"

-- Handler for opening Tundra with tundra:// URL format
on open location this_URL
    set bundlePath to path to me
    set posixBundlePath to POSIX path of bundlePath
    
    do shell script posixBundlePath & "Contents/MacOS/Tundra --config tundra.json --login \"" & this_URL & "\" --trustserverstorages --accept_unknown_http_sources --accept_unknown_local_sources --fpslimit 60 --hide_benign_ogre_messages --nofilewatcher > /dev/null 2>&1&"
end open location

-- Handler for opening txml files with double-click:
on open anFile
    set bundlePath to path to me
    set posixBundlePath to POSIX path of bundlePath
    
    set posixFile to POSIX path of anFile
    do shell script posixBundlePath & "Contents/MacOS/Tundra --splash --file " & posixFile & " > /dev/null 2>&1&"
    log_event("Opening Tundra scene XML " & posixFile & " for viewing in a local server...")
end open

-- Helper function to format log messages for this script
on log_event(themessage)
    set theLine to (do shell script "date  +'%Y-%m-%d %H:%M:%S'" as string) & " " & themessage
    do shell script "echo " & theLine & " >> ~/Library/Logs/Tundra-launchevents.log"
end log_event