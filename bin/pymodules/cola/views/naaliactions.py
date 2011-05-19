from PythonQt import QtGui
from cola import qtutils
from cola.views.log import LogView
import naali
from naali import ui

class ActionView(QtGui.QWidget):
    """Provides a dialog for naali actions."""
    def __init__(self, title='', parent=None):
        QtGui.QWidget.__init__(self, parent)
        self.setWindowTitle(title)
        self.vboxlayout = QtGui.QVBoxLayout(self)
        logview = qtutils.logger()
        if logview:
            logview.clear()
            self.vboxlayout.addWidget(logview)
        else:
            print "Error adding logview"
        
        
        self._layth1 = QtGui.QHBoxLayout()
        
        self.button_close = QtGui.QPushButton(self)
        self.button_close.setText('Done')
        self._layth1.addWidget(self.button_close)
         
        self.vboxlayout.addLayout(self._layth1)        
        
        self.setLayout(self.vboxlayout)
        
        self.button_close.connect('clicked()',self.hide)
        self.button_close.setEnabled(False)
        
    def open(self):
        self.setVisible(True);

        
        
        
        
        
        