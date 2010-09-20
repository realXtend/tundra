import rexviewer as r
from PythonQt.QtGui import QLineEdit

PRIMTYPES_REVERSED = {
    "Material": "45",
    "Texture": "0"
}

class DragDroppableEditline(QLineEdit):
    def __init__(self, mainedit, *args):
        self.mainedit = mainedit #to be able to query the selected entity at drop
        QLineEdit.__init__(self, *args)
        self.old_text = ""
        
        self.combobox = None #throw into another class...
        self.buttons = []
        self.spinners = []
        self.index = None 

    def accept(self, ev):
        return ev.mimeData().hasFormat("application/vnd.inventory.item")
    #XXX shouldn't accept any items, but just the right asset types
    #or better yet: accept drop to anywhere in the window and 
    #determine what to do based on the type?

    def dragEnterEvent(self, ev):
        if self.accept(ev):
            ev.acceptProposedAction()

    def dragMoveEvent(self, ev):
        if self.accept(ev):
            ev.acceptProposedAction()

    def dropEvent(self, ev):
        print "Got meshid_drop:", self, ev
        if not self.accept(ev):
            return

        mimedata = ev.mimeData()
        invitem = str(mimedata.data("application/vnd.inventory.item"))

        #some_qbytearray_thing = invitem[:4] #struct.unpack('i', 
        data = invitem[4:].decode('utf-16-be') #XXX how it happens to be an win: should be explicitly encoded to latin-1 or preferrably utf-8 in the c++ inventory code
        #print data
        asset_type, inv_id, inv_name, asset_ref = data.split(';')

        ent = self.mainedit.active #is public so no need for getter, can be changed to a property if needs a getter at some point
        if ent is not None:
            self.doaction(ent, asset_type, inv_id, inv_name, asset_ref)

            self.update_text(inv_name)

        ev.acceptProposedAction()

    def update_text(self, name):
        """called also from main/parent.select() when sel changed"""
        
        ent = self.mainedit.active
        if ent is not None:
            self.text = name #XXX add querying inventory for name
        else:
            self.text = "N/A"
            
        self.old_text = name
        self.deactivateButtons()

    def doaction(self, ent, asset_type, inv_id, inv_name, asset_ref):
        pass
        
    def applyAction(self):
        print self, "applyAction (not implemented yet in this class) !"
    
    def cancelAction(self):
        #print self, "cancelAction!"
        self.text = self.old_text
        self.deactivateButtons()
        
    def deactivateButtons(self):
        for button in self.buttons:
            button.setEnabled(False)
        
class MeshAssetidEditline(DragDroppableEditline):
    def doaction(self, ent, asset_type, inv_id, inv_name, asset_ref):
        #print "doaction in MeshAssetidEditline-class..."
        applymesh(ent, asset_ref)
        self.deactivateButtons()
    
    def applyAction(self):
        #print self, "applyAction!"
        ent = self.mainedit.active
        if ent is not None:
            applymesh(ent, self.text)
            self.deactivateButtons()
            self.mainedit.window.animation_widget.show()
            self.mainedit.window.animationline.deactivateButtons()

class SoundAssetidEditline(DragDroppableEditline):
    def doaction(self, ent, asset_type, inv_id, inv_name, asset_ref):
        self.deactivateButtons()
    
    def applyAction(self):
        ent = self.mainedit.active
        if ent is not None:
            applyaudio(ent, self.text, self.spinners[0].value, self.spinners[1].value)
            self.deactivateButtons()

    def update_soundradius(self, radius):
        ent = self.mainedit.active
        if ent is not None:
            self.spinners[0].setValue(radius)
        else:
            self.spinners[0].setValue(3.0)

    def update_soundvolume(self, volume):
        ent = self.mainedit.active
        if ent is not None:
            self.spinners[1].setValue(volume)
        else:
            self.spinners[1].setValue(3.0)

class AnimationAssetidEditline(DragDroppableEditline):
    def doaction(self, ent, asset_type, inv_id, inv_name, asset_ref):
        self.deactivateButtons()
    
    def applyAction(self):
        ent = self.mainedit.active
        if ent is not None:
            applyanimation(ent, self.text, self.combobox.currentText, self.spinners[0].value) 
            self.deactivateButtons()
            self.mainedit.window.updateAnimation(ent)

    def update_animationrate(self, rate):
        ent = self.mainedit.active
        if ent is not None:
            self.spinners[0].setValue(rate)
        else:
            self.spinners[0].setValue(1.0)
        
class UUIDEditLine(DragDroppableEditline):
    def doaction(self, ent, asset_type, inv_id, inv_name, asset_ref):
        matinfo = (asset_type, asset_ref)
        self.applyMaterial(ent, matinfo, self.name)
        self.deactivateButtons()

    def applyMaterial(self, ent, matinfo, index):
        qprim = ent.prim
        mats = qprim.Materials
        mats[index] = matinfo
        qprim.Materials = mats
        r.sendRexPrimData(ent.id)
        
    def applyAction(self):
        ent = self.mainedit.active
        if self.combobox is not None and ent is not None:
            qprim = ent.prim
            mats = qprim.Materials
            asset_type_text = self.combobox.currentText
            
            asset_type = PRIMTYPES_REVERSED[asset_type_text] #need to encode to something else?
            mats[self.index]  = (asset_type, self.text)
            qprim.Materials = mats
            
            r.sendRexPrimData(ent.id)
            
            self.deactivateButtons()
            
        
def applymesh(ent, meshuuid):
    try:
        ent.mesh
    except AttributeError:
        ent.prim.MeshID = meshuuid #new
        #r.logDebug("Mesh asset UUID after before sending to server: %s" % ent.mesh)
    else:
        ent.mesh.SetMesh(meshuuid)        
        #ent.prim.MeshID = meshuuid # change meshuuid
    r.sendRexPrimData(ent.id)
    #~ r.logDebug("Mesh asset UUID after prim data sent to server: %s" % ent.mesh)

def applyanimation(ent, animationuuid, animationname, animationrate):
    ent.prim.AnimationPackageID = animationuuid
    ent.prim.AnimationName = animationname
    ent.prim.AnimationRate = animationrate

    try:
        ent.mesh
    except:
        return

    try:
        ac = ent.animationcontroller
    except:
        ent.createComponent('EC_OgreAnimationController')
        ac = ent.animationcontroller
        ac.SetMeshEntity(ent.mesh)
    r.sendRexPrimData(ent.id)


def applyaudio(ent, audiouuid, soundRadius, soundVolume):
    try:
        ent.sound
    except AttributeError:
        ent.prim.SoundID = audiouuid
        # default radius and volume
        ent.prim.SoundRadius = soundRadius
        ent.prim.SoundVolume = soundVolume
    else:
        ent.prim.SoundID = audiouuid
        ent.prim.SoundRadius = soundRadius
        ent.prim.SoundVolume = soundVolume
        ent.sound.SetSound(audiouuid, ent.placeable.Position, ent.prim.SoundRadius, ent.prim.SoundVolume)
    soundRulerUpdate(ent)
    r.sendRexPrimData(ent.id)

def soundRulerUpdate(ent):
    try:
        ent.soundruler
    except AttributeError:
        ent.createComponent("EC_SoundRuler")

    sr = ent.soundruler
    sr.Show()
    sr.UpdateSoundRuler()
