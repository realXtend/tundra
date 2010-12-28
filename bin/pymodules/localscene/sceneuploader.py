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
from dotscene import findComponentAttribute
from dotscene import findNodes
import urllib2
import time
import shutil
import math

from xml.dom.minidom import getDOMImplementation
from xml.dom import Node
import sceneactionsxml

import constants
from constants import MESH_MODEL_FOLDER, MATERIAL_FOLDER, TEXTURE_FOLDER, TEMP_UPLOAD_FOLDER


# MESH_MODEL_FOLDER = "media/models"
# MATERIAL_FOLDER = "media/materials/scripts"
# TEXTURE_FOLDER = "media/textures"

# TEMP_UPLOAD_FOLDER = "UploadPackage" # folder for creating zip file
        

class SceneUploader:

    def __init__(self, cap_url, controller):
        self.controller = controller
        self.host = None
        self.port = None
        self.path = None
        self.timeout = 10
        self.cap_url = cap_url
        self.file = None
        self.headers = {}
        self.progressBar = controller.window.progressBar
        self.sceneData = None
        
        # poster init
        register_openers()
        
        b = self.parseCapsUrl(cap_url)
        if(b==False):
            r.logInfo("Failed to get caps url for scene uploads")
        strict = None
        
        self.appDataUploadFolder=None
        
        
    def uploadScene(self, filepath, dotScene, sceneData, regionName = None, publishName = None):
        f = None
        self.file = filepath + ".zip"
        self.sceneData = sceneData
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
        print headers
        
        request = urllib2.Request(self.cap_url, datagen, headers) # post
        self.progressBar.setValue(5)
        self.progressBar.setFormat("progress: sending request %p%")
        #r.logInfo(urllib2.urlopen(request).read())
        resp = urllib2.urlopen(request).read()
        print resp
        parser = sceneactionsxml.XmlStringDictionaryParser(resp)
        #parser = sceneactionsxml.XmlSceneRegionResponceParser(resp)
        d = parser.parse()
        self.handleErrors(d)

        self.progressBar.setValue(7)
        self.progressBar.setFormat("progress: done %p%")
        self.progressBar.setValue(0)
        #self.progressBar.clear()
        
    def handleErrors(self, d):
        if d==None:
            self.controller.queue.put(('scene upload', 'server sent malformed responce'))
            return
        if not d.has_key('error'):
            self.controller.queue.put(('scene upload', 'server sent malformed responce'))
        if(d['error']!='None'):
            self.controller.queue.put(('scene upload', d['error']))
        
        
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
        uploadPathLen = len(self.appDataUploadFolder)
        for dirname, dirnames, filenames in os.walk(self.appDataUploadFolder):
            # print "--"
            # print dirname
            # print dirnames
            # print filenames
            # print "--"
            zipfolder = dirname[uploadPathLen:]
            zipfolder = os.path.normpath(zipfolder)
            for filename in filenames:
                filepath = os.path.join(dirname, filename)
                zippath = None
                if(zipfolder==''):
                    zippath = TEMP_UPLOAD_FOLDER + os.sep + os.path.basename(filepath)
                else:
                    zippath = TEMP_UPLOAD_FOLDER + os.sep + zipfolder + os.sep + os.path.basename(filepath)
                #zf.write(filepath)
                zf.write(filepath, zippath)
        zf.close()
        #cleanup
        #os.remove(temp_upload_folder)

    def copyfiles(self, ds):
        # Copy files to temp dir 
        self.appDataUploadFolder=r.getApplicationDataDirectory()+ os.sep + TEMP_UPLOAD_FOLDER
        
        if(os.path.exists(self.appDataUploadFolder)==False):
            os.mkdir(self.appDataUploadFolder)
        else:
            #os.remove(self.appDataUploadFolder+os.sep+"*.*")
            #shutil.rmtree("./" + self.appDataUploadFolder)
            shutil.rmtree(self.appDataUploadFolder)
            time.sleep(1)
            os.mkdir(self.appDataUploadFolder)
        relativepath = MESH_MODEL_FOLDER.replace("/", os.sep)
        
        # print ds.fileName
        # split = ds.fileName.split('/')
        # name = split[-1]
        name = self.nameFromFilepath(ds.fileName)
        dstSceneFile = self.appDataUploadFolder + os.sep + name
        
        # if exists copy <scene_name>.material file to upload package
        sceneMaterialFilePath=ds.fileName[:-6] + ".material"
        materialname = self.nameFromFilepath(sceneMaterialFilePath)
        dstSceneMaterialFile = self.appDataUploadFolder + os.sep + materialname
        if self.sceneData.sceneDotMaterial == False:
            if(self.fileExists(sceneMaterialFilePath)==True):
                shutil.copyfile(sceneMaterialFilePath, dstSceneMaterialFile);
                # copy images in scene material file
                self.copyTextures(sceneMaterialFilePath, TEXTURE_FOLDER)
        else:
            # copy Scene.material as <scene_name>.material file
            shutil.copyfile(self.sceneData.materialFile, dstSceneMaterialFile)
            self.copyTextureList(self.sceneData.textures, self.sceneData.materialFile)
            pass
        
        saver = SceneSaver()
        saver.save(dstSceneFile, ds.dotscenemanager.nodes)
        
        # copy  rex file
        rexFile = ds.fileName[:-6] + ".rex"
        rexDstFile = dstSceneFile[:-6] + ".rex"
        # print "--------------"
        # print rexFile
        # print rexDstFile
        # print "--------------"
        if(self.fileExists(rexFile)==True):
            shutil.copyfile(rexFile, rexDstFile)
        
        for k, oNode in ds.dotscenemanager.nodes.iteritems():
            #print k
            self.handleNodePath(oNode.entityMeshFile)
            dstFile = self.appDataUploadFolder + os.sep + oNode.entityMeshFile
            dstFile = os.path.normpath(dstFile)
            
            # try first load from scene folder
            sceneFilePath = os.path.dirname(ds.fileName) + os.sep + oNode.entityMeshFile
            pathToFile = relativepath + os.sep + oNode.entityMeshFile
            sceneFilePath = os.path.normpath(sceneFilePath)
            pathToFile = os.path.normpath(pathToFile)
            
            materialfile = ''
            
            if(self.fileExists(sceneFilePath)):
                shutil.copyfile(sceneFilePath, dstFile)
                materialfile = sceneFilePath[:-5] + ".material"
            elif(self.fileExists(pathToFile)):
                shutil.copyfile(pathToFile, dstFile)
                materialfile = pathToFile[:-5] + ".material"
            
            #print materialfile
            if(self.fileExists(materialfile)==True):
                dstFile = self.appDataUploadFolder + os.sep + oNode.entityMeshFile[:-5] + ".material"
                shutil.copyfile(materialfile, dstFile)
                self.copyTextures(materialfile, TEXTURE_FOLDER)
            
            # check material folder
            materialfile2 = MATERIAL_FOLDER + os.sep + oNode.entityMeshFile[:-5] + ".material"
            #print materialfile2
            if(self.fileExists(materialfile2)==True):
                dstFile = self.appDataUploadFolder + os.sep + oNode.entityMeshFile[:-5] + ".material"
                shutil.copyfile(materialfile2, dstFile)
                self.copyTextures(materialfile2, MATERIAL_FOLDER)
                
            #copy collision mesh if specified
            if(oNode.entityCollisionFile!=None and oNode.entityCollisionFile!=""):
                collisionPath = os.path.dirname(materialfile) + os.sep + oNode.entityCollisionFile
                if(self.fileExists(collisionPath)):
                    dstFile = self.appDataUploadFolder + os.sep + oNode.entityCollisionFile
                    shutil.copyfile(collisionPath, dstFile)
                else:
                    print "Collision file specified, but not found"

    def copyTextureList(self, list, matfile):
        for name in list:
            #pathToFile = folder.replace('/', os.sep) + os.sep + name
            dstFile = self.appDataUploadFolder + os.sep + name
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
        pass
                    
    def copyTextures(self, matfile, folder):
        list = self.getTexturesFromMaterialFile(matfile)
        self.copyTextureList(list, matfile)
        
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

    def handleNodePath(self, path):
        """ Check if path is more than just filename, then we need to create folder structure, so we can copy objects """
        # path = "test/test2/file"
        path = os.path.normpath(path)
        folders=[]
        basePath, fileName = os.path.split(path)
        while(basePath!=''):
            basePath, folder = os.path.split(basePath)
            folders.append(folder)
            pass
        # create folder structure to uploadfolder
        folders.reverse()
        baseFolder = self.appDataUploadFolder
        for f in folders:
            newFolder = os.path.join(baseFolder, f)
            os.mkdir(newFolder)
            baseFolder = newFolder
        pass
        
        
class SceneSaver:
    """ For uploading scene different from the saver in localscene """
    def __init__(self):
        self.impl = getDOMImplementation()
    
    def save(self, filename, nodes):
        from PythonQt.QtGui import QQuaternion
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
                # XXX counter the 'fix' done in loading the scene
                # loader.py in def create_naali_meshentity()
                #ort = oNode.naali_ent.placeable.Orientation * QQuaternion(1, 0, 0, -1) * QQuaternion(1, 0, 0, -1)
                #ort = oNode.naali_ent.placeable.Orientation * QQuaternion(math.sqrt(0.5),0,0,math.sqrt(0.5)) * QQuaternion(math.sqrt(0.5),0,0,math.sqrt(0.5))
                rotate90z = QQuaternion(1,0,0,-1)
                rotate90z.normalize()
                ort = oNode.naali_ent.placeable.Orientation * rotate90z * rotate90z
                
                rotation.setAttribute("qx", str(ort.x()))
                rotation.setAttribute("qy", str(ort.y()))
                rotation.setAttribute("qz", str(ort.z()))
                rotation.setAttribute("qw", str(ort.scalar()))
                nodeNode.appendChild(rotation)
                
                scale = newdoc.createElement('scale')
                scale.setAttribute("x", str(oNode.naali_ent.placeable.Scale.x()))
                
                if filename.endswith(".blend.scene"):
                    scale.setAttribute("y", str(oNode.naali_ent.placeable.Scale.z()))
                    scale.setAttribute("z", str(oNode.naali_ent.placeable.Scale.y()))
                else:
                    scale.setAttribute("y", str(oNode.naali_ent.placeable.Scale.y()))
                    scale.setAttribute("z", str(oNode.naali_ent.placeable.Scale.z()))
                nodeNode.appendChild(scale)
                
                entity = newdoc.createElement('entity')
                # entity.setAttribute("name", oNode.entityNode.getAttribute("name"))
                # entity.setAttribute("meshFile", oNode.entityNode.getAttribute("meshFile"))
                # if oNode.entityNode.hasAttribute("collisionFile"):
                    # entity.setAttribute("collisionFile", oNode.entityNode.getAttribute("collisionFile"))
                # if oNode.entityNode.hasAttribute("collisionPrim"):
                    # entity.setAttribute("collisionPrim", oNode.entityNode.getAttribute("collisionPrim"))
                
                #copy entitys attributes
                eAttrs = oNode.entityNode.attributes
                self.setElementAttributes(entity, eAttrs)

                # print oNode.entityNode.toxml()
                # print oNode.entityNode.hasChildNodes()
                self.copySubStructure(newdoc, oNode.entityNode, entity)                
                        
                if(entity.attributes.has_key("static")==False):
                    entity.setAttribute("static", oNode.entityNode.getAttribute("static"))
                
                # print "-------------------------"
                # print entity.toxml()
                
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
        
    
    def getChildElems(self, element):
        elems = []
        if element.hasChildNodes():
            nodes = element.childNodes
            for node in nodes:
                if(node.nodeType == Node.ELEMENT_NODE):
                    elems.append(node)
        return elems
        
    def setElementAttributes(self, element, attributes):
        keys = attributes.keys()
        for k in keys:
            name = attributes[k].nodeName
            value = attributes[k].nodeValue
            element.setAttribute(k, value)
        
    def copySubStructure(self, newdoc, element1, element2):
        if element1.hasChildNodes():
            childs1 = self.getChildElems(element1)
            if(childs1!=None):
                for ch1 in childs1:
                    ch2 = newdoc.createElement(ch1.nodeName)
                    self.setElementAttributes(ch2, ch1.attributes)
                    self.copySubStructure(newdoc, ch1, ch2)
                    element2.appendChild(ch2)
            else:
                pass
        else:
            #print "%s has no child elements ---------------" % element1.nodeName
            pass
        