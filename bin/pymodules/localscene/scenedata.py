"""
Class for maintaining data for loading scene files to naali folders,
so that files can be added to naali specific folders and removed from
them, 
if folder that scene is uploaded from has material file with same name as scene
file, certain files from scene folder will be copied to naali folders

 - from scene file read mesh file names, copy to MESH_MODEL_FOLDER
 - from material file read lines starting with texture, copy texture to TEXTURE_FOLDER
 - copy matreial file to MATERIAL_FOLDER
 
Note: Naali does not dynamically load textures, they seem to be loaded when launching Naali, 
so for viewing local scene with textures, you still need to copy all files manually to their
folders. If not files are copied to their folders but Naali only shows meshes without textures.
"""


from constants import MESH_MODEL_FOLDER, MATERIAL_FOLDER, TEXTURE_FOLDER, TEMP_UPLOAD_FOLDER

import os
from xml.dom import minidom 
import shutil
import rexviewer as r
#from xml.dom.minidom import getDOMImplementation



class SceneDataManager:


    def __init__(self, scenefile):
        self.sceneFile = scenefile
        self.folderPath = os.path.dirname(scenefile)
        self.xmlDoc = minidom.parse(self.sceneFile)
        self.materialFile = scenefile[:-6] + ".material"
        
        self.meshes = []
        self.textures = []
        self.materials = []
        
        # overlapping items
        self.olMeshes = []
        self.olTextures = []
        self.olMaterials = []
        self.hasCopyFiles = False
        
        if(self.hasMaterialFile(scenefile)):
            self.readMeshes()
            self.readTextures()
            # assume 1 material file
            self.materials.append(os.path.basename(self.materialFile))
            self.hasCopyFiles = True


    def hasMaterialFile(self, scenefile):
        mfile = scenefile[:-6] + ".material"
        return os.access(mfile, os.F_OK)
        
        
    def copyFilesToDirs(self):
        for m in self.meshes:
            srcPath = self.folderPath + os.sep + m
            dstPath = MESH_MODEL_FOLDER + os.sep + m
            self.safeCopy(srcPath, dstPath, m, self.olMeshes)
        for t in self.textures:
            srcPath = self.folderPath + os.sep + t
            dstPath = TEXTURE_FOLDER + os.sep + t
            self.safeCopy(srcPath, dstPath, t, self.olTextures)
        for material in self.materials:
            srcPath = self.folderPath + os.sep + material
            dstPath = MATERIAL_FOLDER + os.sep + material
            self.safeCopy(srcPath, dstPath, m, self.olMaterials)
    
    
    def safeCopy(self, srcPath, dstPath, fname, olList):
        # check if file exists
        if(os.access(dstPath, os.F_OK)):
            #r.logInfo("File allready exists:")
            #r.logInfo(dstPath)
            olList.append(fname)
        else:
            #print "Copying ", srcPath, " to ", dstPath
            if(os.access(srcPath, os.F_OK)):
                shutil.copyfile(srcPath, dstPath)
            else:
                r.logInfo("File defined does not exist:")
                r.logInfo(fname)
                olList.append(fname) # not removed when unloading
            
    
    
    def removeFiles(self):
        for m in self.meshes:
            if(self.olMeshes.__contains__(m)==False):
                rmPath = MESH_MODEL_FOLDER + os.sep + m
                os.remove(rmPath)
        for t in self.textures:
            if(self.olTextures.__contains__(t)==False):
                rmPath = TEXTURE_FOLDER + os.sep + t
                os.remove(rmPath)
        for material in self.materials:
            if(self.olMaterials.__contains__(material)==False):
                rmPath = MATERIAL_FOLDER + os.sep + material
                os.remove(rmPath)
            
    
    def readMeshes(self):
        alreadyAdded = []
        elems = self.xmlDoc.getElementsByTagName('entity')
        for e in elems:
            meshfile = e.getAttribute('meshFile')
            if(alreadyAdded.__contains__(meshfile)==False):
                self.meshes.append(meshfile)
                alreadyAdded.append(meshfile)
        # print "-----------"
        # print "self.meshes"
        # print "-----------"
        # print self.meshes
            
    def readTextures(self):
        #print self.materialFile
        alreadyAdded = []
        f = open(self.materialFile)
        for line in f.readlines():
            sline = line.strip()
            if(sline.startswith('texture')):
                split = sline.split(' ')
                if(split.__len__()==2):
                    #print split[1]
                    if(alreadyAdded.__contains__(split[1])==False):
                        #print split[1]
                        self.textures.append(split[1])
                        alreadyAdded.append(split[1])
                        
        # print "-------------"
        # print "self.textures"
        # print "-------------"
        # print self.textures
    