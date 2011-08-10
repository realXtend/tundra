import sys
import os

def getFileContent(filePath):
    try:
        f = open(filePath, 'r')
        c = f.read()
        f.close()
        return c
    except IOError as e:
        print "IOError on input file:", filePath
        print e
    return None

def saveNewContent(filePath, newContent):
    try:
        f = open(filePath, "w+")
        f.write(newContent)
        f.close()
    except IOError as e:
        print "IOError on writing to file:", filePath
        print e
            
def replace(origin, toFind, replaceWith):
    count = origin.count(toFind)
    if count > 0:
        countStr = str(count)
        if count < 10:
            countStr = "0" + countStr
        print countStr, "x", toFind, "->", replaceWith
        return origin.replace(toFind, replaceWith)
    return origin    

def printAdditionalHelp():
    msg = """
Notes for manual converions that cant be automated (easily at least):

* camera.AutoSetPlaceable() is removed,
  it will auto attach to EC_Placeable in the same entity.
* asset.AddAssetStorage() is removed from AssetAPI,
  but can be found now from asset module.
  framework.GetModuleByName("Asset").AddAssetStorage("name=MyAssets;type=HttpAssetStorage; \
                                                      src=http://www.myserver.com/;[default;])"
  or console.ExecuteCommand("AddAssetStorate(name=MyAssets;type=HttpAssetStorage;
                                             src=http://www.myserver.com/;[default;]")
"""
    print msg

if __name__ == "__main__":

    # Add a list of folders you want to look for .js files and convert.
    sourceDirs = ["chat"]

    # Add your desination sub folder here.
    # "coverted" is the default.
    outDir = "converted"

    files = {}

    if not os.path.isdir(outDir):
        os.mkdir(outDir)

    for sourceDir in sourceDirs:
        files[sourceDir] = []
        for filePath in os.listdir(sourceDir):
            if os.path.isfile(os.path.join(sourceDir, filePath)):
                if filePath.endswith(".js"):
                    files[sourceDir].append(filePath)

    for (sourceDir, fileList) in files.iteritems():
        for filePath in fileList:
            fullPath = os.path.join(sourceDir, filePath)
            c = getFileContent(fullPath)
        
            if c != None:
                o = c
    
                # me entity
                o = replace(o, "me.GetName(", "me.Name(")
                
                # framework
                o = replace(o, "framework.GetModuleQObj(", "framework.GetModuleByName(")

                # debug -> console
                o = replace(o, "debug.Log(", "console.LogInfo(")
                o = replace(o, "debug.LogError(", "console.LogError(")
                o = replace(o, "debug.LogWarning(", "console.LogWarning(")
                o = replace(o, "debug.LogDebug(", "console.LogDebug(")

                # entity
                o = replace(o, ".GetOrCreateComponentRaw(", ".GetOrCreateComponent(")
                o = replace(o, ".GetComponentRaw(", ".GetComponent(")
                o = replace(o, ".HasComponent(", ".GetComponent(")
                o = replace(o, ".GetEntityByNameRaw(", ".GetEntityByName(")

                # asset
                o = replace(o, "asset.GetAssetCache().GetDiskSource(", "asset.GetAssetCache().GetDiskSourceByRef(")
                o = replace(o, "asset.GetAssetCache().NewCookieJar(", "browserplugin.CreateCookieJar(")

                # ec name changes
                o = replace(o, "\"EC_OgreCamera\"", "\"EC_Camera\"")
                o = replace(o, ".ogrecamera.", ".camera.")
                
                # ec attribute changes
                # * EC_HighLight
                #   hopefully this wont create too much false positives :E as in you naming normal vars .*Attr
                o = replace(o, ".textAttr", ".text")
                o = replace(o, ".fontAttr", ".font")
                o = replace(o, ".fontSizeAttr", ".fontSize")
                o = replace(o, ".fontColorAttr", ".fontColor")
                o = replace(o, ".backgroundColorAttr", ".backgroundColor")
                o = replace(o, ".borderColorAttr", ".borderColor")
                o = replace(o, ".borderThicknessAttr", ".borderThickness")
                o = replace(o, ".positionAttr", ".position")
                o = replace(o, ".usingGradAttr", ".usingGrad")
                o = replace(o, ".gradStartAttr", ".gradStart")
                o = replace(o, ".gradEndAttr", ".gradEnd")
                
                # new math lib
                o = replace(o, "new Vector3df()", "new float3()")

                # browser ui moved to own plugin
                o = replace(o, "ui.EmitAddAction(", "browserplugin.AddAction(")
                o = replace(o, "ui.AddAction(", "browserplugin.AddAction(")
                o = replace(o, "ui.AddAction.connect(", "browserplugin.ActionAddRequest.connect(")
                o = replace(o, "ui.EmitOpenUrl(", "browserplugin.OpenUrl(")
                o = replace(o, "ui.OpenUrl(", "browserplugin.OpenUrl(")
                o = replace(o, "ui.OpenUrl.connect(", "browserplugin.OpenUrlRequest.connect(")

                # mumble voip
                o = replace(o, "communications_service.InWorldVoiceSession(", "mumblevoip.Session(")

                # Only replace the file if something was reaplced
                if str(c) != str(o):
                    newDir = os.path.join(outDir, sourceDir);
                    newPath = os.path.join(newDir, filePath)
                    if not os.path.isdir(newDir):
                        os.mkdir(newDir)
                    print "-- Converted:", fullPath
                    print "-- Saving file as:", newPath , "\n"
                    saveNewContent(newPath, o)
            else:
                continue

    printAdditionalHelp()
                
