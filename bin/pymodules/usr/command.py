"""this is executed when you press '.' in the ogre window, the viewer main window.
used for quick testing of py commands."""

import rexviewer as r
import math

print "--- *** ---"

#print dir(r)

#some prim
idnum = 720011 #the cube most far away from the screen in Toni & Petri 's test sim
#idnum = 0
new_id = 9999999

#av ent
av_entid = 8880000

def rotate(e):
    o = e.orientation    
    newort = (o[0], o[1], o[2] + 0.5, o[3])
    print "Rotating to ort:", newort
    e.orientation = newort
    #assert e.orientation[2] > (oldz+0.9) #xxx some logic fail here?
    #print "TEST ORIENTATION SUCCEEDED", e.orientation[2], oldortz
    
def move(e):
    p = e.pos #.pos - the w.i.p. api has a shortcut now that instead of a placeable with loc,rot,scale it just gives loc now directly
    oldx = p[0] #p.x - Vector3 not wrapped (yet), just gives a tuple
    #p.x += 1 #change the x-coordinate
    newpos = (p[0] - 1, p[1], p[2])
    print "Moving to move to pos:", newpos
    e.pos = newpos

if 0:
    print "Testing taking a screenshot..."
    
    path = "pymodules/webserver/screenshot/"
    pic = "current.png"
    
    hmm = r.takeScreenshot(path, pic)
    
if 0:
    print "Testing event sending, camera for now..."#   , r.SwitchCameraState
    hmm = r.sendEvent(r.SwitchCameraState)
    print "test done?", hmm

if 0:
    print "Testing camera swap..."
    hmm = r.switchCameraState()
    #print hmm
    
if 0: #get entity
    #idnum = new_id
    print "Getting entity id", idnum,
    e = r.getEntity(idnum)
    print "got:", e
    #print dir(r)
    rotate(e)
    #move(e)

if 0: #test avatar tracking, works :)
    av_entid = 2628869553
    print "<:::",
    try:
        a = r.getEntity(av_entid)
    except:
        print "could find the avatar with the given id", av_entid
    else:
        print "Avatar pos:", a.pos,
        print ":::>"
        """
        perhaps some local script could track movement?
        make a sound of a nearby object of interest, 
        like when a pet or a friend moves?
        """
        
        if 0:
            #test what happens when we move the av
            #a.pos = a.pos[0] + 1, a.pos[1], a.pos[2]
            """crash, because of how network updates are coded in the internals: 
            XXX RexServerConnecion.cpp
            RexServerConnection::SendMultipleObjectUpdatePacket(std::vector<Scene::EntityPtr> entity_ptr_list)
            const Foundation::ComponentInterfacePtr &prim_component = entity_ptr_list[i]->GetComponent("EC_OpenSimPrim");
            (because avatars don't have the prim component"""
            
            #rotating the av
            rotate(a)

if 0: #push an event, input and/or chat
    #from eventsource import viewer
    #from modulemanager import m 
    import circuits_manager
    mm = circuits_manager.ComponentRunner.instance
    print mm
    
    #mm.INPUT_EVENT(r.MoveForwardPressed)
    
    #a chat message again now too
    mm.RexNetMsgChatFromSimulator("Bob", "- that's me, Bob.")
    
    #previous pyglet stuff, was an ncoming chat msg event
    #m.dispatch_event('on_chat', "input", "testing")
    #print viewer._event_stack

if 0: #create entity
    #not safe now:
    """
    New entity created:16:39:22 [Foundation] Error: Can't create entity with given i
d because it's already used: 9999999
Assertion failed: px != 0, file D:\k2\rex\viewer\trunk\external_libs\include\boo
st/shared_ptr.hpp, line 419
    """
    print "Testing entity creation"
    meshname = "axes.mesh"
    
    avatar = r.getEntity(r.getUserAvatarId())
    ent = r.createEntity(meshname)
    #print "New entity created:", ent, ent.pos
    ent.pos = avatar.pos
    ent.scale = 0.0, 0.0, 0.0
    #print "new pos", ent.pos, ent.scale

if 0: #placeable and text tests
    print "Testing..."
    e = r.getEntity(8880005)    

    #~ try:
        #~ e.pos = 1
    #~ except Exception, e:
        #~ print e
        
    #e.orientation = "well this ain't a quarternion."
    #e.scale = ("does", "this", "work")
    def test():
        print "this ain't a string..."
    
    e.text = "swoot"
    e.text = 1
    e.text = ("swoot", "ness")
    e.text = (1, 2)
    e.text = test
    
    e.pos = 1
    
    print e.text, e.pos, e.scale, e.orientation
    
if 0: #send chat
    r.sendChat("hello from new realXtend Naali!")
    #print "called sendchat ok"
    
if 0: #print test
    r.logInfo("this is a test print!")
    
if 0: #camera pitch
    dy = 0.1
    dp = 0
    #dp = -0.1
    #dp = 0.1
    r.setCameraYawPitch(dy, dp)
    print r.getCameraYawPitch()
        
if 0: #avatar set yaw (turn)
    a = -1.0
    print "setting avatar yaw with %f" % a
    r.setAvatarYaw(a)

if 0: #avatar rotation #XXX crashes when the avatar is not there! XXX
    x = 0
    y = 0 
    z = -1 #this is the actual rotation thingie
    w = 0
    print "rotating the avatar to", (x, y, z, w)    
    r.setAvatarRotation(x, y, z, w)
    
if 0: #create a new qt canvas
    try:
        c = r.c
    except: #initial run
        c = r.createCanvas()
        r.c = c
    else: #the canvas has already been created
        #import PythonQt
        #print globals()
        #print '=============='
        #print 'box' in globals()
        #print dir(PythonQt)
        #print box
        print "~~~"
        #print "test t:", test
        #print "canvas t:", __canvas__
    print c
    
if 0: #add a signal handler to the ui loaded above, i.e. create a slot and connect it
    r.c.label.text = "py-commanded"
    def changed(v):
        print "val changed to: %f" % v
    #print r.c.label.connect('clicked()', changed)
    print r.c.doubleSpinBox.connect('valueChanged(double)', changed)
    print r.c.children()
    print dir(r.c)
    
if 0: #for the box test ui created in code - works
    # define our own python method that appends the text from the line edit
    # to the text browser
    box = r.c
    def appendLine():
        box.browser.append(box.edit.text)
    box.button1.connect('clicked()', appendLine)
    box.edit.connect('returnPressed()', appendLine)
    
if 0: #python-ogre test - using the extension lib in the embedded context :o
    #import sys
    #sys.path.append('pymodules/ogre.zip')
    #didn't work for some reason yet - should .pyd s work from zips too?
    #apparently it should work: http://mail.python.org/pipermail/python-list/2008-March/653795.html
    
    #based on the info in http://www.ogre3d.org/addonforums/viewtopic.php?f=3&t=8743&hilit=embed
    import ogre.renderer.OGRE as ogre
    root = ogre.Root.getSingleton()
    #print dir(r)
    print root.isInitialised()
    rs = root.getRenderSystem()
    #rs.setAmbientLight(1, 1, 1)
    vp = rs._getViewport()
    #print vp
    bg = vp.getBackgroundColour()
    #only affects when not connected, when caelum is not there i figure
    vp.setBackgroundColour(ogre.ColourValue(0.1, 0.2, 0))
    
    cam = vp.getCamera()
    #print cam
    
    sm = root.getSceneManager("SceneManager")
    print sm
    
    def drawline():
        try:
            mcounter = r.mcounter
        except: #first execution
            print "first exec"
            mcounter = 1
        else:
            mcounter += 1
            #print "incremented manual object counter to", mcounter
        r.mcounter = mcounter
        print "Creating manual object index", mcounter
        mob =  sm.createManualObject("manual%d" % mcounter)
        mnode = sm.getRootSceneNode().createChildSceneNode("manual%d_node" % mcounter)
        
        try:
            mmaterial = r.mmaterial
        except: #first execution
            mmaterial = ogre.MaterialManager.getSingleton().create("manual1Material","debugger")
            mmaterial.setReceiveShadows(False)
            tech = mmaterial.getTechnique(0)
            tech.setLightingEnabled(True)
            pass0 = tech.getPass(0)
            pass0.setDiffuse(0, 0, 1, 0)
            pass0.setAmbient(0, 0, 1)
            pass0.setSelfIllumination(0, 0, 1)
            r.mmaterial = mmaterial
            print "created the manual material"
        else:
            pass
            #print "got the existing manual material"
            
        mob.begin("manual1Material", ogre.RenderOperation.OT_LINE_LIST)
        mob.position(40, 240, 55 - mcounter)
        mob.position(240, 10, 10 + mcounter)
        #etc 
        mob.end()
        mnode.attachObject(mob)
        
    drawline()

if 0: #pythonqt introspec
    #print "Importing PythonQt..."
    import PythonQt
    #print dir(PythonQt)
    #print "Importing PythonQt.QtGui..."
    #import PythonQt.QtGui as gui
    #print dir(gui)
    #import PythonQt.QtUiTools as uitools
    #print dir(uitools.QUiLoader)
    #print dir(gui.QTreeWidgetItem)

    UiWidgetProperties = PythonQt.__dict__['UiServices::UiWidgetProperties']
    print type(UiWidgetProperties), dir(UiWidgetProperties)
    #print UiWidgetProperties.WidgetType #the enum should be moved to be inside the class XXX

if 0: #QVector3D
    import PythonQt.QtGui
    #print dir(PythonQt.QtGui)
    v3 = PythonQt.QtGui.QVector3D()
    print v3
    print dir(v3)
    v3.setX(1)
    print v3.x()

    pointa = PythonQt.QtGui.QVector3D(0,0,0)
    pointb = PythonQt.QtGui.QVector3D(2, 2, 0)
    direction = PythonQt.QtGui.QVector3D(1, 1, 0)
    print pointa.distanceToLine(pointb, direction)
    
if 0: #QQuaterinion
    #Works...
    import PythonQt.QtGui
    q1 = PythonQt.QtGui.QQuaternion(1, 0, 0, 1)
    q2 = PythonQt.QtGui.QQuaternion(0.707, 0, 0.707, 0)
    print q1.toString(), q2.toString()
    
    #doesn't work
    #q3 = q1*q2
    #q1 *= q2
    #but this does! (is the same thing, different syntax)
    q1.__imul__(q2)
    print q1.toString()
    #~ print dir(q)

if 0:
    import PythonQt.QtCore
    
    point_a_tl = PythonQt.QtCore.QPoint(2,2)
    point_a_br = PythonQt.QtCore.QPoint(5,5)
    
    point_b_tl = PythonQt.QtCore.QPoint(3,3)
    point_b_br = PythonQt.QtCore.QPoint(7,7)
    
    rect_a = PythonQt.QtCore.QRect(point_a_tl, point_a_br)
    print "Rect A: ", rect_a.toString()
    rect_b = PythonQt.QtCore.QRect(point_b_tl, point_b_br)
    print "Rect B: ", rect_b.toString()
    print "intersects: ", rect_a.intersects(rect_b)
    
    rect_c = rect_a.intersected(rect_b)
    print "intersected:", rect_c.toString()

if 0:
    from PythonQt.QtGui import *

    group = QGroupBox()
    box = QVBoxLayout(group)
    print dir(box)
    push1 =  QPushButton(group)
    box.addWidget(push1)
    push2 =  QPushButton(group)
    box.addWidget(push2)
    check =  QCheckBox(group)
    check.text = 'check me'
    group.title = 'my title'
    push1.text = 'press me'
    push2.text = 'press me2'
    box.addWidget(check)
    group.show()

if 0:
    box = r.c.widget
    def funk(item):
        print "got index...", item
        box.treeWidget.currentItem().setText(0, "doooood")
    r.c.widget.treeWidget.disconnect('activated(QModelIndex)', r.c.itemActivated)
    r.c.itemActivated = funk
    r.c.widget.treeWidget.connect('activated(QModelIndex)', funk)
    print type(r.c)

if 0:
    box = r.c.widget.treeWidget
    box.clear()
    
if 0: #populating the EditGui window
    from PythonQt.QtGui import *
    from PythonQt.QtCore import QPoint
    box = r.c.widget
    box.label.text = "hmm"

    children = []
    children.append(QTreeWidgetItem(box.treeWidget))
    children[0].setText(0, "swoot")
    children.append(QTreeWidgetItem(box.treeWidget))
    children.append(QTreeWidgetItem(box.treeWidget))
    children.append(QTreeWidgetItem(box.treeWidget))
    print len(children)
    #~ def test(item, idx):
        #~ print "worked...", item, idx
    #~ box.treeWidget.connect('itemActivated(QTreeWidgetItem)', test)
    #box.treeWidget.connect('itemActivated', test)
    #box.treeWidget.itemClicked.connect(box.treeWidget.itemClickedSetSelected)
    
    #~ pos = QPoint(0,1)
    #~ thingie5 = box.treeWidget.indexAt(pos)  
    #print box.treeWidget.findChild
    #index = box.treeWidget.currentIndex()
    #print box.treeWidget.activated(index), index
    #print dir(box.treeWidget)

if 0:
    box = r.c.widget
    #print box, dir(box.treeWidget)
    text = box.treeWidget.currentItem().text(0)
    print text
    print type(text), r.c.widgetList
    print r.c.widgetList[720010]

if 0: #QtUI::UICanvas::External ?! not here...
    from PythonQt.QtUiTools import QUiLoader
    from PythonQt.QtCore import QFile, QIODevice
    #print dir(quil)  
    #print dir(PythonQt.QtCore.QIODevice)
    loader = QUiLoader()
    canvas = r.createCanvas(0)
    
    file = QFile("pymodules/usr/editobject.ui")
    iodev = QIODevice(file)

    widget = loader.load(file)
    canvas.AddWidget(widget)
    canvas.Show()
    
if 0: #collada load testing
    import collada
    print collada.Collada
    try:
        pycollada_test
    except:
        import pycollada_test
    else:
        pycollada_test = reload(pycollada_test)

if 0: #canvas size edit
    box = r.c
    #print dir(box.widget.size)
    #print box.widget.size.width()#, box.widget.size.height
    #print type(box.canvas)
    #box.canvas.size.setHeight(600)#
    #box.canvas.adjustSize()
    width = box.widget.size.width()
    height = box.widget.size.height()
    print width, height
    box.canvas.resize(width, height)
    #print box.canvas.resize(380, 250)
    
if 0: #ogre cam test and vectors
    import ogre.renderer.OGRE as ogre
    root = ogre.Root.getSingleton()
    #print dir(r)
    print root.isInitialised()
    rs = root.getRenderSystem()
    
    vp = rs._getViewport()
    
    
    #~ print dir(ogre.Vector3)
    #~ vec = ogre.Vector3(5, 5, 5).normalisedCopy() 
    #~ print vec, type(vec)
    cam = vp.getCamera()
    
    #print dir(cam)
    #print cam.Orientation, cam.DerivedOrientation, cam.getOrientation()
    #~ print "\n"
    #~ print cam.getRight(), cam.getUp()

if 0:
    canvas = r.c.canvas
    for child in canvas.children():
        child.delete()
    canvas.close()
    #canvas.deleteLater()
    
if 0: #pygame window test
    try:
        r.pygame
    except AttributeError: #first run
        import pygame
        r.pygame = pygame
        s = pygame.display.set_mode((320, 200))
        print s,
        r.pygame_s =  s #screen
        print r.pygame_s
        
    else:
        s = r.pygame_s
    print s
    
    s.fill((64, 95, 73))
    pygame.display.flip()
    
    #works :)=======

if 0: #testing the removal of canvases
    canvas = r.c.canvas
    modu = r.getQtModule()
    #print dir(modu)
    bool = modu.RemoveCanvasFromControlBar(canvas)
    print bool
    
if 0:
    print "Testing..."
    e = r.getEntity(8880001)  
    print e
    start_x = e.pos[0]
    start_y = e.pos[1]
    start_z = e.pos[2]
    end_x = e.pos[0]
    end_y = e.pos[1]
    end_z = e.pos[2]
        
    r.sendObjectAddPacket(start_x, start_y, start_z, end_x, end_y, end_z)

if 0: 
    id = r.getUserAvatarId()
    ent = r.getEntity(id)
    print "User's avatar_id:", id
    print "Avatar's mesh_name:", ent.mesh
    ent.mesh = "cruncah1.mesh"
    
if 0:
    print r.getCameraUp()
    print r.getCameraRight()

if 0: #test changing the mesh asset a prim is using
    ent_id = 1659586053 #penkki, arkku was: 2461025163
    #print arkku_id, type(arkku_id)
    ent = r.getEntity(ent_id)
    print "Test entity:", ent
    print ent.mesh
    ent.mesh = 1 #should raise an exception
    ruukku = "681b1680-fab5-4203-83a7-c567571c6acf"
    penkki = "04d335b6-8f0c-480e-a941-33517bf438d8"
    ent.mesh = penkki #"35da6174-8743-4026-a83e-18b23984120d"
    print "new mesh set:", ent.mesh
    
    print "sending prim data update to server"
    r.sendRexPrimData(ent.id) #arkku
    print "..done", ent.mesh
    
if 0: #testing vector3/quat wrapping 
    #import PythonQt
    #print PythonQt.__dict__.has_key("Core::Vector3df")
    #Vector3 = PythonQt.__dict__["Core::Vector3df"]
    from PythonQt import *
    from editgui.conversions import *
    
    print "Creating Vectors:"
    vec = Vector3df(0, 0, 0)
    print "vec1 (%.1f, %.1f, %.1f)" % (vec.x(), vec.y(), vec.z())
    
    vec2 = Vector3df(1, 1, 1)
    print "vec2 (%.1f, %.1f, %.1f)" % (vec2.x(), vec2.y(), vec2.z())
    
    print "distance between the two", vec.getDistanceFrom(vec2)
    
    vec.setx(1.0)
    vec.sety(1.0)
    vec.setz(1.0)
    
    print "changed vec1 (%.1f, %.1f, %.1f)" % (vec.x(), vec.y(), vec.z())
    print "new distance between the two", vec.getDistanceFrom(vec2)
    
    vec.setx(4.0)
    vec.sety(4.0)
    vec.setz(4.0)
    print "changed vec1 (%.1f, %.1f, %.1f)" % (vec.x(), vec.y(), vec.z())
    print "another new distance between the two", vec.getDistanceFrom(vec2)
    
    print "Creating quats:"
    quat = Quaternion(0, 0, 0, 0)
    print "quat (%.4f, %.4f, %.4f, %.4f)" % (quat.x(), quat.y(), quat.z(), quat.w())
    
    quat.setx(2.0)
    quat.sety(2.0)
    quat.setz(3.0)
    quat.setw(2.0)
    print "changed quat (%.4f, %.4f, %.4f, %.4f)" % (quat.x(), quat.y(), quat.z(), quat.w())
    
    print "Quat to Euler:"
    x = 0.707
    y = 0
    z = 0
    w = 0.707
    quat = Quaternion(x, y, z, w)
    euls = Vector3df(0,0,0)
    quat.toEuler(euls)
    euls2 = quat_to_euler((x, y, z, w))
    print "eulers (%.1f, %.1f, %.1f)" % (math.degrees(euls.x()), math.degrees(euls.y()), math.degrees(euls.z()))
    print "eulers2", euls2
    
    print "Euler to Quat:"
    x = math.radians(0)
    y = math.radians(90)
    z = math.radians(0)
    
    euler = Vector3df(x, y, z)
    #~ quat_from_euler = Quaternion(euler)
    #~ print "quat from eulers (%.4f, %.4f, %.4f, %.4f)" % (quat_from_euler.x(), quat_from_euler.y(), quat_from_euler.z(), quat_from_euler.w())

    quat_from_euler = Quaternion(x, y, z)
    print "quat from eulers (%.4f, %.4f, %.4f, %.4f)" % (quat_from_euler.x(), quat_from_euler.y(), quat_from_euler.z(), quat_from_euler.w())
        
    from editgui.conversions import *
    euler = euler_to_quat((0, 90, 0))
    print "quat from eulers", euler
    
if 0:
    avatar = r.getEntity(r.getUserAvatarId())
    avatar.text = "Swoot"
    import PythonQt as qt
    ent = r.getEntity(1392229722)
    print ent.name, ent.pos, ent.scale, ent.orientation, ent.prim
    pos = qt.Vector3df(ent.pos[0], ent.pos[1], ent.pos[2])
    print pos, pos.x(), pos.y(), pos.z()
    ent.pos = pos
    
if 0: #property editor tests
    #print r.c
    #print r, dir(r)
    pe = r.getPropertyEditor()
    #print pe, pe.setObject, pe.show
    pe.setObject(r.c.widget)
    pe.show()
    
if 0: #getting args from outside to run tests automatically
    import os
    naaliargs = os.getenv('NAALIARGS')
    print naaliargs

if 0:
    #print r.c.widget.move_button, dir(r.c.widget.move_button)
    #r.c.canvas.Show()
    print r.c.widget.move_button.isChecked(), r.c.widget.rotate_button.isChecked(), r.c.widget.scale_button.isChecked()
    #print  dir(r.c.widget.move_button)
    r.c.widget.move_button.setChecked(False)
    
if 0:
    from editgui.vector3 import Vector3 as V3
    fov = r.getCameraFOV()
    rightvec = V3(r.getCameraRight())
    campos = V3(r.getCameraPosition())
    ent = r.getEntity(r.getUserAvatarId())
    entpos = V3(ent.pos)
    width, height = r.getScreenSize()
    
    x = 613
    y = 345
    normalized_width = 1/width
    normalized_height = 1/height
    
    #print x * normalized_width
    
    length = (campos-entpos).length
    worldwidth = (math.tan(fov/2)*length) * 2

    #print campos, entpos, length, fov, width, height
    
    ent1 = r.createEntity("cruncah.mesh")
    ent1.pos = pos.x, pos.y+worldwidth/2, pos.z
    ent2 = r.createEntity("cruncah.mesh")
    ent2.pos = pos.x, pos.y+worldwidth/2, pos.z
    #~ newpos = 
    #~ print newpos
    
    
if 0: #bounding box tests
    #robo 1749872183
    #ogrehead 1749872798
    ent = r.getEntity(1749871222)#r.getUserAvatarId())
    from editgui.vector3 import Vector3 as V3
    #~ print ent.boundingbox
    bb = list(ent.boundingbox)
    print bb
    #~ scale = list(ent.scale)
    #~ min = V3(bb[0], bb[1], bb[2])
    #~ max = V3(bb[3], bb[4], bb[5])
    #~ height = abs(bb[4] - bb[1]) + scale[0]#*1.2
    #~ width = abs(bb[3] - bb[0]) + scale[1] #*1.2
    #~ depth = abs(bb[5] - bb[2]) + scale[2]#*1.2
    #~ #print ent.pos, 
    #~ print min, max, height, width, depth
    
    #~ r.box = r.createEntity("Selection.mesh")
    #~ r.box.pos = ent.pos
    
    #~ r.box.scale = height, width, depth#depth, width, height
    #~ r.box.orientation = ent.orientation
    
    
    #~ min_ent = r.createEntity("cruncah1.mesh")
    #~ min_ent.scale = 0.3, 0.3, 0.3
    #~ min_ent.pos = pos[0] + min.x, pos[1] + min.y, pos[2] + min.z 
    
    #~ max_ent = r.createEntity("cruncah1.mesh")
    #~ max_ent.scale = 0.3, 0.3, 0.3
    #~ max_ent.pos = pos[0] + max.x, pos[1] + max.y, pos[2] + max.z
    
if 0: #login - for running tests automatically
    print "starting opensim login"
    #user, pwd, server = "Test User", "test", "localhost:9000"
    user, pwd, server = "d d", "d", "world.evocativi.com:8002"
    r.startLoginOpensim(user, pwd, server)
    
if 0: #getserver test
    #print dir(r)
    #print "YO", r.getTrashFolderId()
    #r.deleteObject(2351241440)
    worldstream = r.getServerConnection()
    
    #print worldstream, dir(worldstream), worldstream.SendObjectDeRezPacket
    worldstream.SendObjectDeRezPacket(2891301779, r.getTrashFolderId())
    #ent = r.getEntity(r.getUserAvatarId())
    #worldstream.SendObjectDeletePacket(1278500474, True)
    #~ ent = r.getEntity(2208825114)
    #~ print ent
    
if 0: #undo tests
    e = r.getEntity(1752805599)
    print e.prim, e.uuid
    e_uuid = "d81432f2-28f3-4e05-ac8a-abb4b625dbe4-"
    worldstream = r.getServerConnection()
    #print worldstream, dir(worldstream), worldstream.SendObjectDeRezPacket
    worldstream.SendObjectUndoPacket(e.uuid)
    
if 0: #undo tests and ent.uuid
    e = r.getEntity(1752805599)
    print e, e.uuid, e.editable  
    worldstream = r.getServerConnection()
    #print worldstream, dir(worldstream), worldstream.SendObjectDeRezPacket
    worldstream.SendObjectSelectPacket(ent.id)
    
if 0: #updateflag checks, duplicate tests
    e = r.getEntity(2054915991)
    print e, e.uuid, e.editable, e.updateflags
    ws = r.getServerConnection()
    #print dir(ws)
    x, y, z = e.pos
    ws.SendObjectDuplicatePacket(e.id, e.updateflags, 1, 1, 1)
    
if 0: #proxywidget signal connecting
    #~ from PythonQt.QtUiTools import QUiLoader
    #~ from PythonQt.QtCore import QFile
    #~ #prop = r.getUiWidgetProperty()
    #~ #print prop, dir(prop), prop.widget_name_
    #~ loader = QUiLoader()
    #~ uifile = QFile("pymodules/editgui/editobject.ui")
    #~ ui = loader.load(uifile)
    #~ uiprops = r.createUiWidgetProperty()
    #~ uiprops.widget_name_ = "WOOT"
    #~ widget = r.createUiProxyWidget(ui, uiprops)
    #~ print widget, dir(widget)
    
    #~ uism = r.getUiSceneManager()
    #~ if uism.AddProxyWidget(widget):
        #~ print "WORKED!"
    
    #~ modu =  r.getQtModule()
    #~ print modu, dir(modu)
    #~ whee = modu.whee()
    #~ print whee, dir(whee)#, whee.about()
    
    print r.c, r.c.proxywidget, dir(r.c.proxywidget)
    def whee(boo):
        print boo
    r.c.proxywidget.connect('Visible(bool)', whee)

if 0: #get entity by (prim) uuid
    #uuid = "cac0a9bf-2ee3-427a-bf2b-5a2f17cb3155"
    e = r.getEntityByUUID(uuid)
    print e, "by uuid", uuid

if 0: #search where a given texture is used
    #uuid = "cac0a9bf-2ee3-427a-bf2b-5a2f17cb3155" #antont local fishworld screen
    #uuid = "3edf2f27-411e-4a80-af8d-a422c014532e" #prim school project test display
    uuid = 'a07893e6-3631-4ee0-b9a4-1a4e07eed5be' #mesh
    #print applyUICanvasToSubmeshesWithTexture(canvas, uuid)
    
if 0:
    #print r.c, dir(r.c)
    print r.manager
    print dir(r.manager)
    
    channels = r.manager.channels
    #print channels
    for item in channels:
        for handler in r.manager._getHandlers(item):
            print handler.channels#dir(handler)
            
if 0:
    import PythonQt
    from PythonQt.QtGui import QTreeWidgetItem, QInputDialog, QLineEdit
    box = r.c.widget.findChild("QVBoxLayout")
    print box, dir(box), box.name
    line = QLineEdit()
    box.addWidget(line)

if 0:
    r.randomTest()
    #print qm, dir(qm)
    #~ print r.c.widget, r.c.proxywidget
    #~ pe = r.getPropertyEditor()
    #~ #print pe, pe.setObject, pe.show
    #~ pe.setObject(r.c.proxywidget)
    #~ pe.show()

if 0:
    worldstream = r.getServerConnection()
    print "send drop bomb:", worldstream.SendGenericMessage("DropBomb", ["here", "soon", "BIG"])

if 0: 
    print r.c, dir(r.c)
    print r.c.widget
    print dir(r.c.proxywidget)
    r.c.proxywidget.hide()
    
if 0: #qprim
    #qprim = r.getQPrim(1680221423)
    e = r.getEntity(1680221423)
    qprim = e.prim
    mats = qprim.Materials
    print mats
    
    #~ qprim.Materials = mats
    
    #~ edited_mats = mats
    
    #~ keys = {}
    #~ id = 0
    for key in mats.itervalues():
        if key[1] == "":
            print "swoot"
        #~ id += 1
    
    #~ #print keys, mats.keys(), mats[keys[0]]

if 0: #qplaceable
    id = 2138143966
    #qplace = r.getQPlaceable(id)
    e = e.getEntity(id)
    qplace = e.placeable
    print qplace, qplace.Position

    oldz = qplace.Position.z()
    print oldz, "==>",

    import PythonQt.QtGui
    from PythonQt.QtGui import QVector3D
    change_v = QVector3D(0, 0, 0.1)
    #dir shows __add__ but for some reason doesn't work out of the box :(
    #"unsupported operand type(s) for +=: 'QVector3D' and 'QVector3D'"
    #qplace.Position += change_v
    #qplace.Position + change_v

    #bleh and this changes the val in the vec, but doesn't trigger the *vec* setter, 
    #so no actual change in Naali internals
    #qplace.Position.setZ(oldz + 0.1)
  
    newpos = qplace.Position
    newpos.setZ(oldz + 0.1)
    qplace.Position = newpos
    print qplace.Position.z(), "."    
        
if 0:
    from PythonQt.QtCore import QFile, QSize
    from PythonQt.QtGui import QLineEdit, QHBoxLayout,  QLabel, QPushButton, QSizePolicy, QIcon
    
    box = r.c.widget.findChild("QHBoxLayout", "meshLine")
    #print box.layoutSpacing
    button = QPushButton()

    icon = QIcon("pymodules/editgui/ok.png")
    icon.actualSize(QSize(16, 16))
    
    button.setSizePolicy(QSizePolicy.Fixed, QSizePolicy.Fixed)
    button.setMaximumSize(QSize(16, 16))
    button.setMinimumSize(QSize(16, 16))

    button.text = ""
    button.name = "Apply"
    button.setIcon(icon)
    box.addWidget(button)
    
    #~ line = r.c.widget.findChild("QLineEdit", "meshLineEdit")
    #~ print line.sizePolicy.horizontalPolicy(), QSizePolicy.Expanding
    
if 0:
    print "Test"
    PRIMTYPES = {
        "0": "Texture", 
        "45": "Material"
    }
    
    def swoot():
        print "booyah!"
    
    def noswoot():
        print "!booyah!"
        
    from PythonQt.QtUiTools import QUiLoader
    from PythonQt.QtCore import QFile, QSize
    from PythonQt.QtGui import QLineEdit, QHBoxLayout, QComboBox, QLabel
    
    loader = QUiLoader()
    uifile = QFile("pymodules/editgui/materials.ui")
    ui = loader.load(uifile)
    uism = r.getUiSceneManager()
    uiprops = r.createUiWidgetProperty()
    uiprops.show_at_toolbar_ = False
    uiprops.widget_name_ = "Test"
    uiprops.my_size_ = QSize(ui.size.width(), ui.size.height())
    pw = r.createUiProxyWidget(ui, uiprops)
    uism.AddProxyWidget(pw)
    r.formwidget = ui.gridLayoutWidget
    r.pw = pw
    r.pw.show()
    
    #print dir(r.formwidget), r.formwidget.rowCount()

    #~ qprim = r.getQPrim(2985471908)
    #~ mats = qprim.Materials
    #~ print mats#, r.formwidget.formLayout.children() 
    #qprim.Materials = mats
    #~ r.elements = []
    #~ indx = 1
    #~ for tuple in mats.itervalues():
        
        #~ print tuple, tuple[0] == "45"
        #combo = QComboBox()
        #combo.addItem("Material")
        #combo.addItem("Texture")
        #~ line = QLineEdit()
        #~ line.text = tuple[1]
        #~ line.name = "lineEdit_"+str(indx)
        #~ indx += 1
        #~ label = QLabel()
        #~ label.name = PRIMTYPES[tuple[0]]#tuple[0]
        #~ label.text = PRIMTYPES[tuple[0]]
        #~ r.elements.append((label, line))
        #~ r.formwidget.formLayout.addRow(label, line)
    
    #print r.elements
    #~ #print dir(r.formwidget)
    #~ stuff =  r.formwidget.children()
    #~ for thingie in stuff:
        #~ print thingie.name#, thingie.name == "formLayout"
        #~ if thingie.name != "formLayout":
            #~ thingie.delete()


    #~ r.pw.show()

if 0:
    box = r.formwidget.findChild("QGridLayout", "gridLayout")
    print box.rowCount()
    
if 0:
    from PythonQt.QtCore import QFile, QSize
    from PythonQt.QtGui import QLineEdit, QHBoxLayout, QComboBox, QLabel, QPushButton, QSizePolicy, QIcon
    
    box = r.formwidget.findChild("QGridLayout", "gridLayout")
    #print box.rowCount()

    label = QLabel()
    label.text = "n/a"
    
    row = 3
    
    box.addWidget(label, row, 0)
    #print r.c.materialDialogFormWidget
    line = QLineEdit()#QLineEdit()
    line.text = "whee"
    line.name = "whee"
    
    box.addWidget(line, row, 1)


    button = QPushButton()
    icon = QIcon("pymodules/editgui/ok.png")
    icon.actualSize(QSize(16, 16))
    button.setSizePolicy(QSizePolicy.Fixed, QSizePolicy.Fixed)
    button.setMaximumSize(QSize(16, 16))
    button.setMinimumSize(QSize(16, 16))
    button.text = ""
    button.name = "Apply"
    button.setIcon(icon)
    
    box.addWidget(button, row, 2)
    
    
    button = QPushButton()
    icon = QIcon("pymodules/editgui/cancel.png")
    icon.actualSize(QSize(16, 16))
    button.setSizePolicy(QSizePolicy.Fixed, QSizePolicy.Fixed)
    button.setMaximumSize(QSize(16, 16))
    button.setMinimumSize(QSize(16, 16))
    button.text = ""
    button.name = "Cancel"
    button.setIcon(icon)
    
    box.addWidget(button, row, 3)

if 0:
    from PythonQt.QtUiTools import QUiLoader
    from PythonQt.QtCore import QFile, QSize
    from PythonQt.QtGui import QLineEdit, QHBoxLayout, QComboBox, QLabel, QPushButton, QSizePolicy, QIcon
    loader = QUiLoader()
    uism = r.getUiSceneManager()
    
    #~ uifile3 = QFile("pymodules/editgui/tab.ui")
    #~ ui3 = loader.load(uifile3)
    #~ uiprops3 = r.createUiWidgetProperty()
    #~ uiprops3.show_at_toolbar_ = False
    #~ uiprops3.widget_name_ = "tab"
    #~ uiprops3.my_size_ = QSize(ui3.size.width(), ui3.size.height())
    #~ pw3 = r.createUiProxyWidget(ui3, uiprops3)
    #~ r.pw3 = pw3
    #~ r.tab_manager = ui3.tabWidget
    #~ uism.AddProxyWidget(pw3)
    #~ r.pw3.show()

    uifile = QFile("pymodules/editgui/materials.ui")
    ui = loader.load(uifile)
    uiprops = r.createUiWidgetProperty()
    uiprops.show_at_toolbar_ = False
    uiprops.widget_name_ = "Test"
    uiprops.my_size_ = QSize(ui.size.width(), ui.size.height())
    pw = r.createUiProxyWidget(ui, uiprops)
    r.pw = pw    
    
    uifile2 = QFile("pymodules/editgui/editobject.ui")
    ui2 = loader.load(uifile2)
    uiprops2 = r.createUiWidgetProperty()
    uiprops2.show_at_toolbar_ = False
    uiprops2.widget_name_ = "editobject"
    uiprops2.my_size_ = QSize(ui2.size.width(), ui2.size.height())
    pw2 = r.createUiProxyWidget(ui2, uiprops2)
    r.pw2 = pw2

    r.tab_manager.addTab(r.pw, QIcon("pymodules/editgui/cancel.png"), "test")
    r.tab_manager.addTab(r.pw2,  QIcon("pymodules/editgui/cancel.png"),  "editobject")
    
if 0:
    from PythonQt.QtGui import QLineEdit, QHBoxLayout
    box =  r.c.materialDialogFormWidget.formLayout
    hor = QHBoxLayout()
    line = QLineEdit()
    hor.addWidget(line)
    print hor

if 0:
    from PythonQt.QtGui import QLineEdit, QHBoxLayout, QComboBox, QLabel
    combo = QComboBox()
    combo.addItem("aaaa")
    combo.addItem("bbbb")
    combo.addItem("cccc")
    combo.addItem("dddd")
    
    print combo.currentIndex, combo.findText("ccCc")

if 0:
    print r.c.propedit
    #~ r.c.propedit.setObject(r.c.propedit)
    #~ r.c.propedit.show()
    #~ props = r.createUiWidgetProperty()
    #~ props.show_at_toolbar_ = False
    #~ props.widget_name_ = "property editor"
    #~ r.test = r.createUiProxyWidget(r.c.propedit, props)
    #~ r.test.show()
    uism = r.getUiSceneManager()
    uism.AddProxyWidget(r.test)
    print r.test
    r.test.show()


if 0:
    import PythonQt
    from PythonQt.QtUiTools import QUiLoader
    from PythonQt.QtCore import QFile, QSize
    from PythonQt.QtGui import QLineEdit, QHBoxLayout, QComboBox, QLabel, QPushButton, QSizePolicy, QIcon, QWidget
    loader = QUiLoader()
    uifile = QFile("pymodules/objectedit/selection.ui")
    ui = loader.load(uifile)
    uism = r.getUiSceneManager()
    uiprops = r.createUiWidgetProperty(2)
    uiprops.widget_name_ = "Thingie Rect"
    
    #uiprops.my_size_ = QSize(width, height) #not needed anymore, uimodule reads it
    proxy = r.createUiProxyWidget(ui, uiprops)
    uism.AddProxyWidget(proxy)
    proxy.setWindowFlags(0)
    
    ui.show()
    
    r.c.ui = ui
    print r.c.ui.geometry
    #~ r.c.ui.setGeometry(10, 60, 400, 400)
    
    #~ r.c.ui.hide()
    #~ r.c.ui = None
    
    #~ print r.c.ui
    #~ r.c.ui.setGeometry

    
if 0:
    for ent in r.c.sels:
        print ent.id
    worldstream = r.getServerConnection()
    print dir(worldstream)
    id1 = 1250116908
    id2 = 1250116909
    ids = [id1, id2]
    worldstream.SendObjectLinkPacket(ids)
    #~ worldstream.SendObjectDelinkPacket(ids)

if 0:
    import PythonQt.QtGui
    id = 2302910681
    ent = r.getEntity(id)
    pos = PythonQt.QtGui.QVector3D(0, 0, 1)
    print ent.placeable.Position.toString(), ent.placeable.Orientation.toString(), pos.toString()
    ent.placeable.Position = ent.placeable.Position.__add__(pos)
    
    print ent.placeable.Position.toString(), ent.placeable.Orientation.toString(), pos.toString()
    r.networkUpdate(id)

if 0:
    import PythonQt.QtGui
    a = PythonQt.QtGui.QVector3D(5, 5, 1)
    b = PythonQt.QtGui.QVector3D(5, 5, 0)
    print a == b, a.toString(), b.toString()
    
if 0:
    ent = r.getEntity(1876645602)
    mesh = ent.mesh
    if mesh is not None:
        print "swoot"

if 0: #create a new component, hilight
    e = r.getEntity(1876645543)
    try:
        e.highlight
    except AttributeError:
        print e.createComponent("EC_Highlight")
        print "created a new Highlight component"

    h = e.highlight
    print type(h), h
    
    h.Show()
    #h.Hide()
    
    vis = h.IsVisible()
    if vis:
        print "vis"
    else:
        print "not"
        
if 0: #log level visibility
    r.logDebug("Debug")
    #r.logWarning("Warning") #not implement now, should add i guess
    r.logInfo("Info")