import os
import os.path
import sys
import zipfile

'''
Seeks for *.pdb files from current directory and all child directories.
All found pdb files are copied to 'pdb-files.zip' file with their relative file paths.
'''

fileList = []
rootdir = os.curdir
zip_file_name = "pdb-files.zip"

if os.path.isfile(zip_file_name):
    print ("file '%s' already exits. Please remove the file before run this script." % (zip_file_name))
    sys.exit()
print ("All pdb files are stored to: %s" %(zip_file_name))
print ("Please wait, this might take several minutes...")

count = 0
total_size = 0
for root, subFolders, files in os.walk(rootdir):
    for file in files:
        name, ext = os.path.splitext( file )
        if ext != '.pdb':
            continue	
        if file:
            count = count +1
            total_size = total_size + os.path.getsize(os.path.join(root,file))
            fileList.append(os.path.join(root,file))

zout = zipfile.ZipFile(zip_file_name, "w")            
for fname in fileList:
    zout.write(fname)
zout.close()

print ("Totally %i pdb files was found with total size %.1f megabytes." % (count, total_size/1024/1024))
