import rexviewer as r
from circuits import Component
from PythonQt.QtUiTools import QUiLoader
from PythonQt.QtCore import QFile
from PythonQt.QtGui import QColorDialog
from PythonQt.QtGui import QColor

class TextCreator(Component):
    uifile_path = "pymodules/CreateTextDialog.ui"
    maxlength = 300 # chars
    default_color = QColor(200, 0, 0)

    def __init__(self):
        Component.__init__(self)
        self.dialog = QUiLoader().load(QFile(self.uifile_path))
        props = r.createUiWidgetProperty(1) #1 is ModuleWidget, shown at toolb
        props.widget_name_ = "Create Text"
        
        base_url = "./data/ui/images/menus/"
        props.SetMenuNodeIconNormal(base_url + "edbutton_OBJED_normal.png")
        props.SetMenuNodeIconHover(base_url + "edbutton_OBJED_hover.png")
        props.SetMenuNodeIconPressed(base_url + "edbutton_OBJED_click.png")
        
        self.proxy = r.createUiProxyWidget(self.dialog, props)
        uism = r.getUiSceneManager()
        if not uism.AddProxyWidget(self.proxy):
            print "Adding the ProxyWidget to the bar failed."
    
        self.create_button = self.dialog.findChild("QPushButton", "createButton")
        self.create_button.connect("clicked()", self.on_create_clicked)

        self.drop_button = self.dialog.findChild("QPushButton", "dropButton")
        self.drop_button.connect("clicked()", self.on_drop_clicked)
        self.drop_button.setEnabled(False)

        self.dialog.findChild("QLineEdit", "lineEdit").setMaxLength(self.maxlength)
        self.colorpicker = QColorDialog()
        self.colorpicker.connect("colorSelected(QColor)", self.on_color_selected)
        self.dialog.findChild("QPushButton", "pushButton").connect("clicked()", self.colorpicker.show)
        self.last_color_selected = self.default_color

    def on_color_selected(self, color):
        print "on_color_selected called with color=", color
        self.last_color_selected = color

    def on_ok(self):
        print 'on-ok called'
        text = self.dialog.findChild("QLineEdit", "lineEdit").text
        worldstream = r.getServerConnection()
        c = self.last_color_selected
        color_string = "%f %f %f" % (c.redF(), c.greenF(), c.blueF())
        worldstream.SendGenericMessage("CreateTexto_v0", [text[:self.maxlength].encode('utf-8'), color_string])
        print 'sent genericmessage'

    def on_create_clicked(self):
        self.on_ok()
        self.drop_button.setEnabled(True)
        self.create_button.setEnabled(False)

    def on_drop_clicked(self):
        self.drop_button.setEnabled(False)
        self.create_button.setEnabled(True)
        worldstream = r.getServerConnection()
        worldstream.SendGenericMessage("DropText_v0", [])
        
    def on_cancel(self):
        print 'cancel :('

    def on_exit(self):
        self.proxy.hide()
        uism = r.getUiSceneManager()
        uism.RemoveProxyWidgetFromScene(self.proxy)
