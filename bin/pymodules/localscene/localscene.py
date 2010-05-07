#!/usr/bin/python

import rexviewer as r
from circuits import Component

from PythonQt.QtUiTools import QUiLoader
from PythonQt.QtCore import QFile

#import localscene.window
import window
import loader
import dotscenemanager

from window import LocalSceneWindow as LCwindow

from xml.dom.minidom import getDOMImplementation


class LocalScene(Component):
    def __init__(self):
        print "__init__"
        Component.__init__(self)
        self.window = LCwindow(self)
        self.xsift = 127
        self.ysift = 127
        self.zsift = 25
        self.xscale = 1
        self.yscale = 1
        self.zscale = 1
        self.dotScene = None
        self.dsManager = None
        self.flipZY = False
        self.highlight = False
        pass

    def loadScene(self, filename):
        if(filename!=None):
            if(filename!=""):
                self.dotScene, self.dsManager = loader.load_dotscene(filename)
                self.dsManager.setHighlight(self.highlight)
                #enabled, xsift, ysift, zsift, xscale, yscale, zscale
                self.dsManager.setFlipZY(self.flipZY, self.xsift, self.ysift, self.zsift, self.xscale, self.yscale, self.zscale)

    def saveScene(self, filename):
        # set new mesh positions & scales to file, positions, scales are stored in DotSceneManager.nodes[].naali_ent.placeable.Position & Scale
        saver = SceneSaver()
        saver.save(filename, self.dsManager.nodes)
        #saver.save(filename, None)

    def unloadScene(self):
        loader.unload_dotscene(self.dotScene)
        pass
        
    def publishScene(self):
        print "publishing scene"
        
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
        print "local scene exit"
        r.logInfo("Local Scene exiting...")
        self.window.on_exit()  
        r.logInfo("Local Done exiting...")

    def on_hide(self, shown):
        print "on hide"
        
    def update(self, time):
        #print "here", time
        pass

    def on_logout(self, id):
        r.logInfo("Local scene Logout.")
        
    def checkBoxHighlightToggled(self, enabled):
        self.highlight = enabled
        if(self.dsManager!=None):
            self.dsManager.setHighlight(enabled)


            
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
                
                #print dir(oNode.naali_ent)
                #print str(oNode.naali_ent.prim.MeshID)
                #print str(oNode.naali_ent)
                #print oNode.entityNode.length
                #print oNode.entityNode.count()
                # for i in range(oNode.entityNode.length):
                    # print str(oNode.entityNode[i])
                # print str(oNode.naali_ent.placeable.Position)
                # print dir(oNode.naali_ent.placeable.Position)
                # print oNode.naali_ent.placeable.Position.x
                # print oNode.naali_ent.placeable.Position.x()
                
                nodeNode = newdoc.createElement('node')
                nodeNode.setAttribute("name", k)
                nodeNode.setAttribute("id", oNode.id)
                #nodeNode.setAttribute("id", k)
                
                position = newdoc.createElement('position')
                
                # position.setAttribute("x", str(oNode.position[0]))
                # position.setAttribute("y", str(oNode.position[1]))
                # position.setAttribute("z", str(oNode.position[2]))
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
        print newdoc.toprettyxml()
        
        f = open(filename + "test", 'w')
        f.write(newdoc.toprettyxml())


        
        
        #if(nodes != None):
            #sceneroot = newdoc.createTextNode('scene formatVersion=\"\"')
            
        
        
        

