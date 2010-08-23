import rexviewer as r
from PythonQt.QtGui import QLineEdit

PRIMTYPES_REVERSED = {
    "Material": "45",
    "Wav": "17",
    "Ogg": "1",
    "Texture": "0"
}

class DragDroppableEditline(QLineEdit):
    def __init__(self, mainedit, *args):
        self.mainedit = mainedit #to be able to query the selected entity at drop
        QLineEdit.__init__(self, *args)
        self.old_text = ""
        
        self.combobox = None #throw into another class...
        self.buttons = []
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
    r.sendRexPrimData(ent.id)
    #~ r.logDebug("Mesh asset UUID after prim data sent to server: %s" % ent.mesh)

def applyaudio(ent, audiouuid):
    try:
        ent.sound
    except AttributeError:
        ent.prim.MeshID = audiouuid #new
    else:
        ent.audio.SetMesh(audiouuid)        
    r.sendRexPrimData(ent.id)
