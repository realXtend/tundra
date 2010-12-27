"""
This is possibly temporary fix for scenes exported from blender
"""

from xml.dom import minidom, Node
import sys
import os
import time
#from dotscene import findNodes
import multiprocessing

from multiprocessing import Process, Queue

import subprocess

# qthreading
# import PythonQt

# class BlenderExportHandler(PythonQt.QtCore.QRunnable):
    # def __init__(self, filename, controller):
        # self.fileName = filename
        # self.controller = controller
    # def run(self):
        # print "starting Qt thread for handling blender export"
        # meshHandler = MeshHandler(self.fileName)
        # if meshHandler.checkOgreXmlConverter():
            # if meshHandler.checkIfConverted()==False:
                # print "clean up empty facelists"
                # meshHandler.cleanupEmptyFaceLists()
                # meshHandler.changeXYZs()
                # meshHandler.remakeMeshes()
                # # meshHandler.rewriteScales()
                # meshHandler.writeConversionCheck()
        # self.controller.queue.put(('__continue_load__', filename))
        # #print "Hello world from thread", QThread.currentThread()

class BlenderExportprocessHandler():
    pass
        
# do 
# - empty faces cleanup from .mesh.xml files
# - do coordinate system translation to .mesh.xml's
# - convert cleaned, and translated mesh.xml's to meshes
# - rewrite scales to .scene file

class MeshHandler:
    def __init__ (self, fileName):
        self.fileName = fileName
        self.folder = os.path.dirname(fileName)
        self.meshXmls = []
        self.getMeshXmlFiles()
        self.root = None
        self.state = "init"
        pass
        
    def getState(self):
        return self.state
        
    def start(self):
        self.doFixes()
        
    def doFixes(self):
        self.fixWithoutMeshMagick()
        #self.fixWithMeshMagick():        

    def fixWithoutMeshMagick(self):
        self.state = "started cleaning"
        self.cleanupEmptyFaceLists()
        self.state = "started -xzy translation"
        self.changeXYZs()
        self.state = "remake meshes"
        self.remakeMeshes()
        self.state = "meshes generated"
        # self.rewriteScales()
        self.writeConversionCheck()
        self.state = "end"
        pass
        
    def fixWithMeshMagick(self):
        pass
        
    def checkIfConverted(self):
        # do conversion file check (write conversion file after all convs are done)
        # if there's conversion file with datetime newer than scene files datetime, scene is allready converted
        convPath = os.path.dirname(self.fileName) + os.sep + "converted"
        if(os.access(convPath, os.F_OK)):
            scenestat = os.stat(self.fileName)
            sceneTime = scenestat.st_mtime
            convstat = os.stat(convPath)
            convTime = convstat.st_mtime
            if(convTime>sceneTime):
                return True
        return False
        
    def writeConversionCheck(self):
        # create or update modified time
        f = open(os.path.dirname(self.fileName) + os.sep + "converted", "w")
        f.close()
        pass
        
    def checkOgreXmlConverter(self):
        (dummy, stdout_and_stderr) = os.popen4('OgreXmlConverter')
        help_print = stdout_and_stderr.read()
        if(help_print.startswith("\nOgreXMLConvert: Converts data between XML and OGRE binary formats.")):
            print "found OgreXmlConverter"
            return True
        print "can't found OgreXmlConverter, is OgreXmlConverter on the commandline path?"
        return False
        
    def checkPythonExe(self):
        (dummy, stdout_and_stderr) = os.popen4('python --help')
        help_print = stdout_and_stderr.read()
        if(help_print.find("usage: python [option]")!=-1):
            return self.checkPythonVersion()
        return False

    def checkPythonVersion(self):
        i,k = os.popen4("python -V")
        i.close()
        line = k.readline()
        print line
        spl = line.split(' ')
        versionNums = spl[1].split('.')
        if versionNums[0]>=2:
            return True
        return False
        
    def cleanupEmptyFaceLists(self):
        for xml in self.meshXmls:
            #print "cleaning up mesh", xml
            self.state = "cleaning up mesh " + xml
            self.removeEmptyFaceLists(xml)
        pass
        
    def changeXYZs(self):
        #print "converting vertexes.. xyz -> -xzy"
        for xml in self.meshXmls:  
            self.state = "converting vertexes " + xml
            self.convertFile(xml)
        pass
        
    def remakeMeshes(self):
        for mesh in self.meshXmls:
            command = 'OgreXmlConverter ' + '\"' + mesh + '\"'
            print command
            #(dummy, stdout_and_stderr) = os.popen4('OgreXmlConverter ' + mesh)
            (dummy, stdout_and_stderr) = os.popen4(command)
            output = stdout_and_stderr.read()
            # xfname = mesh + ".log"
            # f = open(xfname, w)
            # f.write(output)
            # f.close()
            if (output.find("MeshSerializer export successful.")!=-1):
                #print "remade ", mesh
                self.state = "remade " + mesh
            else:
                #print "remake failed for ", mesh
                self.state = "remake failed for " + mesh
                print output
            pass
        pass
            
    def rewriteScales(self):
        """ Edit .scene file scale attribute for all nodes """
        print "rewriting scales to scene file.."
        doc = minidom.parse(self.fileName)
        # nodes = self.findNodes(minidom.parse(self.fileName).documentElement,'nodes')
        nodes = self.findNodes(doc.documentElement,'nodes')
        self.root = nodes[0].childNodes
        for node in self.root:
            if node.nodeType == Node.ELEMENT_NODE and node.nodeName == 'node':
                scaleNode = self.findNodes(node, 'scale')[0]
                sy = scaleNode.getAttribute('y')
                sz = scaleNode.getAttribute('z')
                scaleNode.setAttribute('y', sz)
                scaleNode.setAttribute('z', sy)
        #self.saveDocToFile(doc, self.fileName, ".converted")
        self.saveDocToFile(doc, self.fileName)#, ".converted")
            
    def getMeshXmlFiles(self):
        listing = os.listdir(self.folder)
        xmlMeshes=[]
        for name in listing:
            if name.endswith(".mesh.xml"):
                self.meshXmls.append(self.folder + os.sep + name)
        pass

    def convertFile(self, filePath):
        doc = minidom.parse(filePath)
        docElem = doc.documentElement
        sharedgeometry = self.findNodes(docElem, 'sharedgeometry')[0]
        vertexbuffer = self.findNodes(sharedgeometry, 'vertexbuffer')[0]
        vertexes = self.findNodes(vertexbuffer, 'vertex')
        for vertex in vertexes:
            position = self.findNodes(vertex, 'position')[0]
            normal = self.findNodes(vertex, 'normal')[0]
            self.convert3DNode(position)
            self.convert3DNode(normal)
        self.saveDocToFile(doc, filePath)#, ".converted")
        pass
        
    def convert3DNode(self, node):
        x = float(node.getAttribute('x'))
        y = float(node.getAttribute('y'))
        z = float(node.getAttribute('z'))
        # do conversion x -> -x, y -> z, z -> y
        #   -1 0 0
        #    0 0 1
        #    0 1 0
        node.setAttribute('x', str(-x))
        node.setAttribute('y', str(z))
        node.setAttribute('z', str(y))
        # node.setAttribute('x', str(x))
        # node.setAttribute('y', str(z))
        # node.setAttribute('z', str(-y))
        return node        

    def saveDocToFile(self, doc, filePath, fileEnd = ""):
        saveFile = filePath + fileEnd
        f = open(saveFile, 'w')
        f.write(doc.toxml())
        f.close()
        pass

    def removeEmptyFaceLists(self, filePath):
        #print "removeEmptyFaceLists(filePath):"
        doc = minidom.parse(filePath)
        docElem = doc.documentElement
        submeshes = self.findNodes(docElem, 'submeshes')[0]
        # print submeshes
        submeshlist = self.findNodes(submeshes, 'submesh')
        # print len(submeshlist)
        for smesh in submeshlist:
            nodes = self.findNodes(smesh, 'faces')
            if len(nodes)!=0:
                facesNode = nodes[0]
                attr = facesNode.getAttribute('count')
                if(facesNode.getAttribute('count')=="0"):
                    submeshes.removeChild(smesh)
                    smesh.unlink()
                    #print "removed empty face list"
                    self.state = "removed empty face list"
        #self.saveDocToFile(doc, filePath, ".converted")
        self.saveDocToFile(doc, filePath)
        pass

    def findNodes (self, root, name):
        out=minidom.NodeList()
        if root.hasChildNodes:
            nodes = root.childNodes
            for node in nodes:
                if node.nodeType == Node.ELEMENT_NODE and node.nodeName == name:
                    out.append(node)
        return out

        

def f(q, q2):
    while(1):
        val=q2.get()
        if(val=='quit'):
            break
        q.put('received something')
    q.put([42, None, 'hello'])
        
        
def startProcess(fileName):
    # f = open('fileName', 'w')
    # f.write(fileName)
    # f.flush()
    # f.close()
    # runfile= os.getcwd() + "/pymodules/localscene/meshhandler.py"
    # theproc = subprocess.Popen(['python', runfile])

    q = Queue()
    q2 = Queue()
    p = Process(target=f, args=(q,q2,))
    p.start()
    print 'process started'
    for i in range(5):
        #parent_conn.stdin.write('%d\n' % i)
        q2.put('%d\n' % i)
        #output = process.stdout.readline()
        #output = child_conn.recv()
        output = q.get()
        print output
        time.sleep(1)

    q2.put('quit')
    print(q.get())    # prints "[42, None, 'hello']"
    p.join()
    

