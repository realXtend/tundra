import rexviewer as r
from circuits import Component
from PythonQt.QtUiTools import QUiLoader
from PythonQt.QtCore import QFile
from PythonQt.QtGui import QFileDialog, QLabel
import os, random

print "PictureCreator loading"

def SendGenericMessage(*args, **kw):
    return r.getServerConnection().SendGenericMessage(*args, **kw)

class PictureCreator(Component):
    uifile_path = "pymodules/FrameDialog.ui"
    drag_prompt_text = "Drag a picture from the\ninventory and drop here"
    create_prompt_text = "Click create frame to start"

    def __init__(self):
        Component.__init__(self)

        self.dialog = QUiLoader().load(QFile(self.uifile_path))
        props = r.createUiWidgetProperty(1) #1 is ModuleWidget, shown at toolb
        props.widget_name_ = "Create Frame"
        
        base_url = "./data/ui/images/menus/"
        props.SetMenuNodeIconNormal(base_url + "edbutton_OBJED_normal.png")
        props.SetMenuNodeIconHover(base_url + "edbutton_OBJED_hover.png")
        props.SetMenuNodeIconPressed(base_url + "edbutton_OBJED_click.png")
        
        self.proxy = r.createUiProxyWidget(self.dialog, props)
        uism = r.getUiSceneManager()
        if not uism.AddProxyWidget(self.proxy):
            print "Adding the ProxyWidget to the bar failed."

        find = self.dialog.findChild
        self.create_button = x = find("QPushButton", "createButton")
        x.connect("clicked()", self.on_create_clicked)

        self.close_button = x = find("QPushButton", "closeButton")
        x.connect("clicked()", self.on_close_clicked)

        self.drop_button = x = find("QPushButton", "dropButton")
        x.connect("clicked()", self.on_drop_clicked)
        x.setEnabled(False)

        self.hlayout = find("QHBoxLayout", "horizontalLayout")

        self.dnd_label = x = DragAndDropLabel()
        self.hlayout.addWidget(x)
        self.dnd_label.setText(self.create_prompt_text)
        x.show()

        print '***********'
        print 'finished picturecreator init'

    def on_genericmessage(self, name, data):
        if name != "FrameOpResponse_v0":
            return
        if len(data) != 2:
            return
        op, success = data
        if op == "CreateFrame" and success == 'ok':
            self.create_ok()
        elif op == "DropFrame" and success == 'ok':
            self.drop_ok()
        elif op == "SetPicture" and succes == 'ok':
            self.dnd_label.setText("Picture assigned")
        
    def on_create_clicked(self):
        print "create handler called"
        self.drop_button.setEnabled(True)
        self.create_button.setEnabled(False)
        SendGenericMessage("CreateFrame_v0", [])

    def create_ok(self):
        print "create ok"
        self.dnd_label.setText(self.drag_prompt_text)
        self.dnd_label.setAcceptDrops(True)
        print "set dnd label to prompt for & accept drops"

    def on_drop_clicked(self):
        SendGenericMessage("DropFrame_v0", [])

    def drop_ok(self):
        self.create_button.setEnabled(True)
        self.drop_button.setEnabled(False)
        self.dnd_label.setText(self.create_prompt_text)
        self.dnd_label.setAcceptDrops(False)
        
    def on_close_clicked(self):
        self.proxy.hide()

    def on_exit(self):
        self.proxy.hide()
        #uism = r.getUiSceneManager()
        #uism.RemoveProxyWidgetFromScene(self.proxy)

class DragAndDropLabel(QLabel):
    def __init__(self, *args, **kw):
        QLabel.__init__(self, *args, **kw)

    def accept(self, ev):
        return ev.mimeData().hasFormat("application/vnd.inventory.item")

    def dragEnterEvent(self, ev):
        if self.accept(ev):
            ev.acceptProposedAction()

    def dragMoveEvent(self, ev):
        if self.accept(ev):
            ev.acceptProposedAction()

    def dropEvent(self, ev):
        if not self.accept(ev):
            return

        mimedata = ev.mimeData()
        invitem = mimedata.data("application/vnd.inventory.item")

        data = invitem[4:].decode('utf-16-be') #XXX how it happens to be an win: should be explicitly encoded to latin-1 or preferrably utf-8 in the c++ inventory code
        asset_type, inv_id, inv_name, asset_ref = data.split(';')

        print asset_type, inv_id, inv_name, asset_ref
        
        ev.acceptProposedAction()
        self.setText(u"Assigning picture"+ inv_name)
        SendGenericMessage("SetPicture_v0", [asset_ref])
        print "sent genericmessage with asset ref", asset_ref
