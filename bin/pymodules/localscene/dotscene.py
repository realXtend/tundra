#!/usr/bin/python
"""
NOTE: 
this has been copied from python-ogre demos and modified a little to work without direct access to ogre,
by antont / Toni Alatalo for the realXtend Naali viewer project. original docstring is below.
this is used by loader making fake ogre scenemanager, (mesh) entity and node classes which are used to
create mesh entities in Naali (which also uses ogre underneath).
"""

"""
this file parses .scene node (dotscene) files and
creates them in OGRE with user data

Doesn't do any fancy stuff (skydome, XODE, lightmapping, etc) but you can use this as a base for adding those features.)

cpp:
http://www.ogre3d.org/wiki/index.php/DotScene_Loader_with_User_Data_Class
"""
from xml.dom import minidom, Node
import string
#import ogre.renderer.OGRE as ogre
from PythonQt.QtGui import QQuaternion as Quaternion
from PythonQt.QtGui import QVector3D as Vector

import dotscenemanager
from dotscenemanager import DotSceneManager as DSManager


"""
self.dotscene = DotScene(self.fileName, self.sceneManager, rootnode)
"""     

class DotScene:
    def __init__ (self, fileName, sceneManager, rootNode=None, prefix = ''):
        self.fileName = fileName
        self.sceneManager = sceneManager
        self.cameras = []
        self.lights = []
        self.prefix = prefix # used to prefix the node name when creating nodes
        nodes = self.findNodes(minidom.parse(self.fileName).documentElement,'nodes')
        self.root = nodes[0].childNodes
        self.dotscenemanager = DSManager()
        
        if rootNode:
            self.rootNode = rootNode
        else:            
            self.rootNode= self.sceneManager.getRootSceneNode()
                    
        self.parseDotScene()
    
    # allows self['nodeName'] to reference xml node in '<nodes>'
    def __getitem__ (self,name):
        return self.findNodes(self.root,name)

    def parseDotScene (self):
        # TODO: check DTD to make sure you get all nodes/attributes
        # TODO: Use the userData for sound/physics
        for node in self.root:
            if node.nodeType == Node.ELEMENT_NODE and node.nodeName == 'node':
                attachMe = None
                
                realName =  node.attributes['name'].nodeValue
                # create new scene node
                newNode = self.rootNode.createChildSceneNode() # self.prefix + realName)

                #position it
                pos = self.findNodes(node, 'position')[0].attributes
                newNode.position = Vector(float(pos['x'].nodeValue), float(pos['y'].nodeValue), float(pos['z'].nodeValue))
                
                # rotate it
                try:
                    rot = self.findNodes(node, 'rotation')[0].attributes
                    newNode.orientation = Quaternion(float(rot['qw'].nodeValue), float(rot['qx'].nodeValue),
                                                            float(rot['qy'].nodeValue), float(rot['qz'].nodeValue))
#                     print float(rot['qw'].nodeValue), float(rot['qx'].nodeValue), float(rot['qy'].nodeValue),float(rot['qz'].nodeValue)
                except IndexError: # probably doesn't have rotation attribute
                    rot = self.findNodes(node, 'quaternion')[0].attributes
                    newNode.orientation = Quaternion(float(rot['w'].nodeValue), float(rot['x'].nodeValue), 
                                                          float(rot['y'].nodeValue), float(rot['z'].nodeValue))
#                     print float(rot['w'].nodeValue), float(rot['x'].nodeValue), float(rot['y'].nodeValue), float(rot['z'].nodeValue)
                
                # scale it
                scale = self.findNodes(node, 'scale')[0].attributes
                newNode.scale = Vector(float(scale['x'].nodeValue), float(scale['y'].nodeValue), float(scale['z'].nodeValue))
                
                # is it a light?
                #try:
                #    thingy = self.findNodes(node, 'light')[0].attributes
                #    name = str(thingy['name'].nodeValue)
                    #attachMe = self.sceneManager.createLight(name)
                    #ltypes={'point':ogre.Light.LT_POINT,'directional':ogre.Light.LT_DIRECTIONAL,'spot':ogre.Light.LT_SPOTLIGHT,'radPoint':ogre.Light.LT_POINT}
                    #try:
                    #    attachMe.type = ltypes[thingy['type'].nodeValue]
                    #except IndexError:
                    #    pass
                    
                #    lightNode = self.findNodes(node, 'light')[0]

                #    try:
                #        tempnode = self.findNodes(lightNode, 'colourSpecular')[0]
                #        attachMe.specularColour = (float(tempnode.attributes['r'].nodeValue), float(tempnode.attributes['g'].nodeValue), float(tempnode.attributes['b'].nodeValue), 1.0)
                #    except IndexError:
                #        pass
                    
                #    try:
                #        tempnode = self.findNodes(lightNode, 'colourDiffuse')[0]
                #        attachMe.diffuseColour = (float(tempnode.attributes['r'].nodeValue), float(tempnode.attributes['g'].nodeValue), float(tempnode.attributes['b'].nodeValue), 1.0)
                #    except IndexError:
                #        pass
                    
                #    try:
                #        tempnode = self.findNodes(lightNode, 'colourDiffuse')[0]
                #        attachMe.diffuseColour = (float(tempnode.attributes['r'].nodeValue), float(tempnode.attributes['g'].nodeValue), float(tempnode.attributes['b'].nodeValue), 1.0)
                #    except IndexError:
                #        pass
                #    self.lights.append( attachMe )
                    
                #    print 'added light: "%s"' % name
                #except IndexError:
                #    pass
                
                # is it an entity?
                try:
                    thingy = self.findNodes(node, 'entity')[0].attributes
                    name = str(thingy['name'].nodeValue)
                    mesh = str(thingy['meshFile'].nodeValue)
                    attachMe = self.sceneManager.createEntity(name,mesh)
                    print 'added entity: "%s" %s' % (name, mesh)
                except IndexError:
                    pass
                #except: #ogre.OgreFileNotFoundException: # mesh is missing
#                     print "Missing Mesh:",mesh                     
                #    pass

                # is it a camera?
                # TODO: there are other attributes I need in here
                try:
                    thingy = self.findNodes(node, 'camera')[0].attributes
                    name = str(thingy['name'].nodeValue)
                    fov =  float(thingy['fov'].nodeValue)
                    projectionType= str(thingy['projectionType'].nodeValue)
                    attachMe = self.sceneManager.createCamera(name)
                    try:
                        tempnode = self.findNodes(node, 'clipping')[0]
                        attachMe.nearClipDistance = float(tempnode.attributes['near'].nodeValue)
                        attachMe.farClipDistance = float(tempnode.attributes['far'].nodeValue)
                    except IndexError:
                        pass
                    ##attachMe.setFOVy ( ogre.Radian( fov ) )  #fOVy = fov
                    
                    self.cameras.append ( attachMe )
                    print 'added camera: "%s" fov: %f type: %s clipping: %f,%f' % (name, fov, projectionType,attachMe.nearClipDistance,attachMe.farClipDistance)
                except IndexError:
                    pass
                
                # Add id & entity data
                #try:
                newNode.id = node.getAttribute('id')
                newNode.entityNode = self.findNodes(node, 'entity')[0]
                #entity = self.findNodes(node, 'entity')
                newNode.entityName = newNode.entityNode.getAttribute("name")
                newNode.entityMeshFile = newNode.entityNode.getAttribute("meshFile")
                newNode.entityStatic = newNode.entityNode.getAttribute("static")
                #except err:
                #    print "parsing id or mesh data failed", err
                #    pass
                
                # attach it to the scene
                #try:
                if attachMe is not None:
                    newNode.attachObject(attachMe)
                    self.dotscenemanager.nodes[realName] = newNode
                #except:
                    #print "could not attach:",realName
                

        
    def findNodes (self,root, name):
        out=minidom.NodeList()
        if root.hasChildNodes:
            nodes = root.childNodes
            for node in nodes:
                if node.nodeType == Node.ELEMENT_NODE and node.nodeName == name:
                    out.append(node)
        return out

if __name__ == "__main__":
    import sys
    sys.exit(main(sys.argv))
