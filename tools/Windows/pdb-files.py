import os
import os.path
import sys
import zipfile
import shutil

"""Finds all .pdb files and copies them to ./Tundra-pdb """

targetdir = "Tundra-pdb"
copydir = os.path.join(os.getcwd(), targetdir)
rootdir = os.getcwd()[0:-6] # strip the /tools from the end

fileList = []
ignoredebugdirs = True
count = 0
totalsize = 0

if os.path.isdir(copydir):
    print ("Out directory '%s' already exits. Please rename it so you don't lose any previous data or delete the folder by hand." % (copydir))
    sys.exit()
    
print ("\nAll pdb files are stored to: %s" %(copydir))
print ("Please wait, this might take several minutes...")

for root, subFolders, files in os.walk(rootdir):
    for file in files:
        name, ext = os.path.splitext(file)
        if ext != '.pdb':
            continue
        fileabs = os.path.join(root,file)
        if ignoredebugdirs:
            parentdirname = os.path.dirname(fileabs)
            parentdirname = parentdirname[parentdirname.rfind("\\")+1:]
            if parentdirname == 'Debug':
                continue
        if file:
            count = count +1
            totalsize = totalsize + os.path.getsize(fileabs)
            fileList.append(fileabs)

print ("Copying files...")

for fname in fileList:
    achivefilename = fname[len(rootdir)+1:]
    outpath = os.path.join(copydir, achivefilename)
    outdir = outpath[:outpath.rfind("\\")]
    print "  * " + targetdir + "\\" + achivefilename
    if not os.path.exists(outdir):
        os.makedirs(outdir)
    shutil.copy(fname, outpath)

print ("\nTotally %i pdb files was found with total size %.1f megabytes." % (count, totalsize/1024/1024))
print ("Press enter to exit...");
fakein = raw_input()