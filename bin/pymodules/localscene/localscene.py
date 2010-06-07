
"""
Loads scene files to be locally in naali, currently all scene related files need to be copied to naali specific media folders by hand
"""

import rexviewer as r
from circuits import Component

from PythonQt.QtUiTools import QUiLoader
from PythonQt.QtCore import QFile

import window
import loader
import dotscenemanager
import sceneuploader
import scenedata

import PythonQt
import threading
import time

from window import LocalSceneWindow as LCwindow
from sceneuploader import SceneUploader as SUploader
from sceneuploader import SceneSaver as SSaver
from scenedata import SceneDataManager

from xml.dom.minidom import getDOMImplementation

# import circuits
# from circuits.core import handler
# from circuits import Event, Manager, Debugger

import Queue

        
class LocalScene(Component):
    def __init__(self):
        Component.__init__(self)

        # Create the queue, for feeding events for ui
        self.queue = Queue.Queue()

        # A timer to periodically read input
        self.timer = PythonQt.QtCore.QTimer()
        PythonQt.QtCore.QObject.connect(self.timer,
                           PythonQt.QtCore.SIGNAL("timeout()"),
                           self.periodicCall)
        self.timer.start(2000)        
        self.window = LCwindow(self, self.queue, self.endApplication)
        
        self.isrunning = 1
        
        self.uploadThread = None
        
        self.xsift = 127
        self.ysift = 127
        self.zsift = 25
        self.xscale = 1
        self.yscale = 1
        self.zscale = 1
        self.dotScene = None
        self.dsManager = None
        self.worldstream = None
        self.flipZY = False
        self.highlight = False
        self.uploader = None
        self.filename = ""        
        self.scenedata = None
        pass

    def loadScene(self, filename):
        # if material file exists copy needed files to needed locations
        self.scenedata = SceneDataManager(filename)
        if(self.scenedata.hasCopyFiles):
            self.scenedata.copyFilesToDirs()
            pass

        time.sleep(1)
        if(filename!=None):
            if(filename!=""):
                self.dotScene, self.dsManager = loader.load_dotscene(filename)
                self.dsManager.setHighlight(self.highlight)
                #enabled, xsift, ysift, zsift, xscale, yscale, zscale
                self.dsManager.setFlipZY(self.flipZY, self.xsift, self.ysift, self.zsift, self.xscale, self.yscale, self.zscale)

    def saveScene(self, filename):
        # set new mesh positions & scales to file, positions, scales are stored in DotSceneManager.nodes[].naali_ent.placeable.Position & Scale
        saver = SSaver()
        savename = filename+".saved"
        saver.save(savename, self.dsManager.nodes)

    def unloadScene(self):
        loader.unload_dotscene(self.dotScene)
        if(self.scenedata!=None and self.scenedata.hasCopyFiles == True):
            self.scenedata.removeFiles()
        pass
        
    def publishScene(self, filename=""):
        if(filename==""):
            filename = self.filename
        #print "publishing scene"
        if(self.worldstream==None):
            self.worldstream = r.getServerConnection()
        # try to get capability UploadScene
        uploadcap_url = self.worldstream.GetCapability('UploadScene')
        if(uploadcap_url==None or uploadcap_url==""):
            self.queue.put(('No upload capability', 'Check your rights to upload scene'))
            #self.window.displayMessage("No upload capability", "Check your rights to upload scene")
            return
        #print str(uploadcap_url)
        if(self.uploader==None):
            self.uploader=SUploader(uploadcap_url)
        self.uploader.uploadScene(filename, self.dotScene)
        print "unloading dot scene"
        self.queue.put(('__unload__', '__unload__scene__'))
        #loader.unload_dotscene(self.dotScene)
        #print "unloaded dot scene"
        self.queue.put(('scene upload', 'upload done'))
                
    def setxpos(self, x):
        self.xsift = x
        if(self.dsManager!=None):
            self.dsManager.setPosition(self.xsift, self.ysift, self.zsift)
        
    def setypos(self, y):
        self.ysift = y
        if(self.dsManager!=None):
            self.dsManager.setPosition(self.xsift, self.ysift, self.zsift)

    def setzpos(self, z):
        self.zsift = z
        if(self.dsManager!=None):
            self.dsManager.setPosition(self.xsift, self.ysift, self.zsift)

    def setxscale(self, x):
        self.xscale = x
        if(self.dsManager!=None):
            self.dsManager.setScale(self.xscale, self.yscale, self.zscale)

    def setyscale(self, y):
        self.yscale = y
        if(self.dsManager!=None):
            self.dsManager.setScale(self.xscale, self.yscale, self.zscale)

    def setzscale(self, z):
        self.zscale = z
        if(self.dsManager!=None):
            self.dsManager.setScale(self.xscale, self.yscale, self.zscale)        

    def checkBoxZYToggled(self, enabled):
        self.flipZY = enabled
        if(self.dsManager!=None):
            self.dsManager.setFlipZY(enabled, self.xsift, self.ysift, self.zsift, self.xscale, self.yscale, self.zscale)
        pass
        
    def on_exit(self):
        r.logInfo("Local Scene exiting...")
        self.window.on_exit()  
        r.logInfo("Local Done exiting...")

        
    def on_hide(self, shown):
        #print "on hide"
        pass
        
    def update(self, time):
        # print "here", time
        # self.window.processIncoming()
        pass

    def on_logout(self, id):
        r.logInfo("Local scene Logout.")
        
    def checkBoxHighlightToggled(self, enabled):
        self.highlight = enabled
        if(self.dsManager!=None):
            self.dsManager.setHighlight(enabled)

    def startUpload(self, filename):
        self.filename = filename
        self.uploadThread = threading.Thread(target=self.publishScene)
        self.uploadThread.start()
        pass
    
    def periodicCall(self):
        #Check every 2000 ms if there is something new in the queue.
        #self.timer.stop()
        #print "periodicCall"
        self.window.processIncoming()
        # if self.isrunning:
            # print "starting periodicCall"
            # self.timer.start(2000)
        if not self.isrunning:
            self.timer.stop()

    def endApplication(self):
        self.isrunning = 0

    def closeThread(self):
        pass
        
            
class SceneSaver:
    def __init__(self):
        self.impl = getDOMImplementation()
        
    def save(self, filename, nodes):
        #newdoc = self.impl.createDocument(None, "some_tag", None)
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
                position.setAttribute("x", str(oNode.naali_ent.placeable.Position.x()-127))
                position.setAttribute("y", str(oNode.naali_ent.placeable.Position.y()-127))
                position.setAttribute("z", str(oNode.naali_ent.placeable.Position.z()-25))
                                
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
        
        #f = open(filename + "test", 'w')
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
