
import rexviewer as r

import httplib
import socket
import os
import zipfile
import tarfile
from socket import getaddrinfo
from socket import SOCK_STREAM
#from httplib import HTTPConnection, HTTPException

from poster.encode import multipart_encode
from poster.streaminghttp import register_openers
import urllib2
import time
import shutil
from xml.dom.minidom import getDOMImplementation

import constants
from constants import MESH_MODEL_FOLDER, MATERIAL_FOLDER, TEXTURE_FOLDER, TEMP_UPLOAD_FOLDER



# MESH_MODEL_FOLDER = "media/models"
# MATERIAL_FOLDER = "media/materials/scripts"
# TEXTURE_FOLDER = "media/textures"

# TEMP_UPLOAD_FOLDER = "UploadPackage" # folder for creating zip file
        

class SceneUploader:

    def __init__(self, cap_url, controller):
        self.host = None
        self.port = None
        self.path = None
        self.timeout = 10
        self.cap_url = cap_url
        self.file = None
        self.headers = {}
        self.progressBar = controller.window.progressBar
        
        # poster init
        register_openers()
        
        b = self.parseCapsUrl(cap_url)
        if(b==False):
            r.logInfo("Failed to get caps url for scene uploads")
        strict = None
        
        #self.httpclient = HTTPConnection(self.host, self.port, strict, self.timeout)
        #self.httpclient = HTTPConnection(self.host, self.port)
        #self.httpclient.set_debuglevel(1) # f or figuring out what goes wrong
        
    def uploadScene(self, filepath, dotScene, regionName = None, publishName = None):
        f = None
        self.file = filepath + ".zip"
        print "creating zip file"
        self.progressBar.setValue(1)
        self.progressBar.setFormat("progress: creating zip %p%")
        self.createZipFile(dotScene);
        print "creating zip file done"
        self.progressBar.setValue(2)
        self.progressBar.setFormat("progress: zip created %p%")
        #try:
        datagen, headers = multipart_encode({"uploadscene": open(self.file, "rb")})
        self.progressBar.setValue(3)
        self.progressBar.setFormat("progress: headers, data encoded %p%")
        #datagen, headers = multipart_encode({"uploadscene": open(self.file, "rb"), "USceneMethod":"Upload"})
        headers['USceneMethod']='Upload'
        if (not(regionName == None)):
            print "not(regionName == None))"
            headers['RegionName']=regionName
        else:
            headers['RegionName']=regionName
        if (not(publishName == None)):
            print "(not(publishName == None))"
            headers['PublishName']=publishName
        else:
            headers['PublishName']=publishName
        
        
        print headers
        
        request = urllib2.Request(self.cap_url, datagen, headers) # post
        self.progressBar.setValue(5)
        self.progressBar.setFormat("progress: sending request %p%")
        #print "------"
        r.logInfo(urllib2.urlopen(request).read())
        self.progressBar.setValue(7)
        self.progressBar.setFormat("progress: done %p%")
        self.progressBar.setValue(0)
        #self.progressBar.clear()
        #print "------"

        # except:
            # r.logInfo("uploadScene failed")

    def testGetAddrInfo(self, host, port):
        #print "Fetch addr info for ", host, " with port ", port
        for res in getaddrinfo(host, port, 0, SOCK_STREAM):
            af, socktype, proto, canonname, sa = res
            # print "af: ", af
            # print "socktype: ", socktype
            # print "proto: ", proto
            # print "canonname: ", canonname
            # print "sa: ", sa        
        
    def parseCapsUrl(self, cap_url):
        r.logInfo(cap_url)
        url_split = cap_url.split(':')
        if(url_split.__len__<3):
            r.logInfo("mallformed cap url for scene upload")
            return False
        surl = url_split[1]
        self.host = surl.split('/')[2]
        port_path = url_split[2]
        self.port = int(port_path.split('/')[0])
        self.path = "/".join(port_path.split('/')[1:])
        return True
        
    def getFile(self, filepath):
        f = open(filepath, 'r')
        contents = f.read()
        f.close()
        return contents
    
    
    def fileExists(self, filepath):
        return os.access(filepath, os.F_OK)
        pass
    
        
    def createZipFile(self, dotScene):
        # save setting first
        self.copyfiles(dotScene)
        #relativepath = MESH_MODEL_FOLDER.replace("/", os.sep)
        allreadyread = []
        print "----------------"
        #print self.file
        zf = zipfile.ZipFile(self.file, "w")
        for dirname, dirnames, filenames in os.walk(TEMP_UPLOAD_FOLDER):
            for filename in filenames:
                filepath = os.path.join(dirname, filename)
                zf.write(filepath)
        zf.close()
        #cleanup
        #os.remove(temp_upload_folder)

    def copyfiles(self, ds):
        # Copy files to temp dir 
        
        if(os.path.exists(TEMP_UPLOAD_FOLDER)==False):
            os.mkdir(TEMP_UPLOAD_FOLDER)
        else:
            #os.remove(TEMP_UPLOAD_FOLDER+os.sep+"*.*")
            shutil.rmtree("./" + TEMP_UPLOAD_FOLDER)
            time.sleep(1)
            os.mkdir(TEMP_UPLOAD_FOLDER)
        relativepath = MESH_MODEL_FOLDER.replace("/", os.sep)
        
        # print ds.fileName
        # split = ds.fileName.split('/')
        # name = split[-1]
        name = self.nameFromFilepath(ds.fileName)
        dstSceneFile = TEMP_UPLOAD_FOLDER + os.sep + name
        
        # if exists copy <scene_name>.material file to upload package
        sceneMaterialFilePath=ds.fileName[:-6] + ".material"
        if(self.fileExists(sceneMaterialFilePath)==True):
            materialname = self.nameFromFilepath(sceneMaterialFilePath)
            dstSceneMaterialFile = TEMP_UPLOAD_FOLDER + os.sep + materialname
            shutil.copyfile(sceneMaterialFilePath, dstSceneMaterialFile);
            # copy images in scene material file
            self.copyTextures(sceneMaterialFilePath, TEXTURE_FOLDER)
        
        saver = SceneSaver()
        saver.save(dstSceneFile, ds.dotscenemanager.nodes)
        
        for k, oNode in ds.dotscenemanager.nodes.iteritems():
            #print k
            dstFile = TEMP_UPLOAD_FOLDER + os.sep + oNode.entityMeshFile
            
            # try first load from scene folder
            sceneFilePath = os.path.dirname(ds.fileName) + os.sep + oNode.entityMeshFile
            pathToFile = relativepath + os.sep + oNode.entityMeshFile
            
            materialfile = ''
            
            if(self.fileExists(sceneFilePath)):
                shutil.copyfile(sceneFilePath, dstFile)
                materialfile = sceneFilePath[:-5] + ".material"
            elif(self.fileExists(pathToFile)):
                shutil.copyfile(pathToFile, dstFile)
                materialfile = pathToFile[:-5] + ".material"
            
            #print materialfile
            if(self.fileExists(materialfile)==True):
                dstFile = TEMP_UPLOAD_FOLDER + os.sep + oNode.entityMeshFile[:-5] + ".material"
                shutil.copyfile(materialfile, dstFile)
                self.copyTextures(materialfile, TEXTURE_FOLDER)
            
            # check material folder
            materialfile2 = MATERIAL_FOLDER + os.sep + oNode.entityMeshFile[:-5] + ".material"
            #print materialfile2
            if(self.fileExists(materialfile2)==True):
                dstFile = TEMP_UPLOAD_FOLDER + os.sep + oNode.entityMeshFile[:-5] + ".material"
                shutil.copyfile(materialfile2, dstFile)
                self.copyTextures(materialfile2, MATERIAL_FOLDER)

    def copyTextures(self, matfile, folder):
        list = self.getTexturesFromMaterialFile(matfile)
        #print list
        for name in list:
            #pathToFile = folder.replace('/', os.sep) + os.sep + name
            dstFile = TEMP_UPLOAD_FOLDER + os.sep + name
            
            dirpath = os.path.dirname(matfile)
            scenePath = dirpath + os.sep + name
            if(self.fileExists(scenePath)):
                #print "----------------------------"
                #print "copying from ", scenePath
                shutil.copyfile(scenePath, dstFile)
                continue
            
            pathToFile = TEXTURE_FOLDER.replace('/', os.sep) + os.sep + name
            if(self.fileExists(pathToFile)):
                shutil.copyfile(pathToFile, dstFile)
            else:
                r.logInfo("Failed to find texture file specified in material file:")
                r.logInfo(pathToFile)
    
    def getTexturesFromMaterialFile(self, matfile):
        txtList = []
        f = open(matfile, 'r')
        line = " "
        while(line!=None and line!=""):
            line = f.readline()
            
            if(line.startswith("import")):
                self.handleMaterialImportLine(line, txtList)
                
            if(line.startswith("material")):
                self.parseMaterialSection(f, line, txtList)
        f.close()
        return txtList
                        
    def handleMaterialImportLine(self, line, importTextList):
        # just read what file is imported and get all textures from it
        # might not be the best sollution in the long run
        ind = line.find("from")
        if(ind!=-1):
            filename = line[ind+4:]
            filename = filename.strip()
            pathToFolder = MATERIAL_FOLDER.replace("/", os.sep)
            pathToFile = pathToFolder + os.sep + filename
            pathToFile = pathToFile.replace("\"","")
            # read file and get .png's, .dds's, .tga's from it
            #print pathToFile
            f = open(pathToFile, 'r')
            line = " "
            while(line!=None and line!=""):
                line = f.readline()
                if(line.startswith("material")):
                    self.parseMaterialSection(f, line, importTextList)
            f.close()
                
                
    def parseMaterialSection(self, f, line, txtList):
        # parse material section, need to parse { and }'s
        # assume that last } is in beginning of line
        # so read all lines untill we find line starting with }
        # parse texture name from lines that end with .jpg, .dds, .png, .tga
        while(line.startswith('}')==False):
            line = f.readline()
            if (line==None):
                break
            #dir(line).__contains__('endswith')
            #print "line ", line
            if(line.endswith('.jpg\r\n') or line.endswith('.dds\r\n') or line.endswith('.png\r\n') or line.endswith('.tga\r\n')):
                txtList.append( line.split(' ')[-1][:-2])
            if(line.endswith('.jpg\n') or line.endswith('.dds\n') or line.endswith('.png\n') or line.endswith('.tga\n')):
                txtList.append( line.split(' ')[-1][:-1])
        #return txtList
            
                        
    def nameFromFilepath(self, path):
        split = path.split('/')
        name = split[-1]
        return name

    def _appendToZipFile(self, filename, appendfile):
        zf = zipfile.ZipFile(filename, "a") #, zipfile.ZIP_STORED, False)
        zf.write(appendfile, os.path.basename(appendfile))
        zf.close()

    def extractZipFile(self):
        zf = zipfile.ZipFile(self.file, "r")
        list = zf.namelist()
        for name in list:
            print name
            
        
    def createTarFile(self, dotScene):
        tar = tarfile.open(self.file, 'w:gz')
        scenefilename = self.file[:-4]
        tar.add(scenefilename)
        relativepath = MESH_MODEL_FOLDER.replace("/", os.sep)
        allreadyread = []
        for k, oNode in dotScene.dotscenemanager.nodes.iteritems():
            #print oNode.entityMeshFile
            print k
            pathToFile = relativepath + os.sep + oNode.entityMeshFile
            if(allreadyread.__contains__(pathToFile)==False):
                print pathToFile
                allreadyread.append(pathToFile)
                tar.add(pathToFile)
                materialfile = pathToFile[:-5] + ".material"
                if(self.fileExists(materialfile)==True):
                    tar.add(materialfile)
        tar.close()

        
    def parentFolderFromFilePath(self, path):
        return os.path.dirname(path)
        
    def copyLocalizedSceneFilesToNaalisFolders(self, sceneFilePath):
        
        pass
        
class SceneSaver:
    """ For uploading scene different from the saver in localscene """
    def __init__(self):
        self.impl = getDOMImplementation()
    
    def save(self, filename, nodes):
        newdoc = self.impl.createDocument(None, "scene formatVersion=\"\"", None)
        top_element = newdoc.documentElement
        nodesNode = newdoc.createElement('nodes')
        top_element.appendChild(nodesNode)
        
        if(nodes != None):
            for k, oNode  in nodes.iteritems():
                nodeNode = newdoc.createElement('node')
                nodeNode.setAttribute("name", k)
                nodeNode.setAttribute("id", oNode.id)
                
                position = newdoc.createElement('position')

                position.setAttribute("x", str(oNode.naali_ent.placeable.Position.x()))
                position.setAttribute("y", str(oNode.naali_ent.placeable.Position.y()))
                position.setAttribute("z", str(oNode.naali_ent.placeable.Position.z()))
                                
                nodeNode.appendChild(position)
                
                rotation = newdoc.createElement('rotation')
                rotation.setAttribute("qx", str(oNode.orientation.x))
                rotation.setAttribute("qy", str(oNode.orientation.y))
                rotation.setAttribute("qz", str(oNode.orientation.z))
                rotation.setAttribute("qw", str(oNode.orientation.w))
                nodeNode.appendChild(rotation)
                
                scale = newdoc.createElement('scale')
                scale.setAttribute("x", str(oNode.naali_ent.placeable.Scale.x()))
                scale.setAttribute("y", str(oNode.naali_ent.placeable.Scale.y()))
                scale.setAttribute("z", str(oNode.naali_ent.placeable.Scale.z()))
                nodeNode.appendChild(scale)
                
                entity = newdoc.createElement('entity')
                entity.setAttribute("name", oNode.entityNode.getAttribute("name"))
                entity.setAttribute("meshFile", oNode.entityNode.getAttribute("meshFile"))
                entity.setAttribute("static", oNode.entityNode.getAttribute("static"))
                nodeNode.appendChild(entity)
                nodesNode.appendChild(nodeNode)
        
        f = open(filename, 'w')
        # remove first line + change ending tag from </scene formatVersion=""> to </scene>
        contents = newdoc.toprettyxml()
        lines = contents.split('\n')
        lines = lines[1:]
        lines = lines[:-1]
        lines.remove("</scene formatVersion=\"\">")
        lines.append("</scene>")
        contents = '\n'.join(lines)
        f.write(contents)
        f.close()
