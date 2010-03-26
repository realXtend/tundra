"""
making a new popup-window / dialog from qtscript,
within an existing application in the script engine host qt app.

to call basic qt library things from qtscript, they need to be added to the api
- it's done in qs_eval in http://qt.gitorious.org/qt-labs/qtscriptgenerator/blobs/1ff84cc76c56097a73c66bc9e40ae3417fbf1300/qtbindings/qs_eval/main.cpp
for an example js code only widget see e.g. AnalogClock.qs there,
http://qt.gitorious.org/qt-labs/qtscriptgenerator/blobs/1ff84cc76c56097a73c66bc9e40ae3417fbf1300/examples/AnalogClock.qs

didn't do that yet, am first testing loading a .ui file and having .js handlers for that.
this tech is similar to http://doc.trolltech.com/4.5/script-calculator.html
"""

import sys
from PyQt4 import QtGui#, QtCore
from PyQt4.QtScript import QScriptEngine, QScriptValue

app = QtGui.QApplication(sys.argv)
window = QtGui.QMainWindow()
window.setWindowTitle("Naali qtscript customdialog test")

window.show()

"""exposing ui loading myself 'cause don't have qtscript bindings at hand yet"""
from PyQt4 import uic
uis = []
def loadui(ctx, eng):
    global uis #to keep gc from clearing this :o
    uifile = ctx.argument(0).toString() #expects a single arg: name of .ui file
    ui = uic.loadUi(uifile) #, window)
    ui.show()
    uis.append(ui)
    #return QScriptValue(eng, 0)
    return engine.newQObject(ui)

def debugprint(ctx, eng): #for javascript to print to console
    print ctx.argument(0).toString()
    return QScriptValue(eng, 0)

engine = QScriptEngine()
eprop = engine.globalObject().setProperty
eprop("loadui", engine.newFunction(loadui))
eprop("print", engine.newFunction(debugprint))

engine.evaluate(""" //the javascript source
ui = loadui("dialog.ui");
print(ui);

function changed(v) {
    print("val changed to: " + v);
}

print(ui.doubleSpinBox.valueChanged);
ui.doubleSpinBox['valueChanged(double)'].connect(changed);
print("connecting to doubleSpinBox.valueChanged ok from js (?)");
""")

if engine.hasUncaughtException():
    bt = engine.uncaughtExceptionBacktrace()
    print "Traceback:"
    print "\n".join(["  %s" % l for l in list(bt)])
    print engine.uncaughtException().toString()
        
#~ ui = uic.loadUi("dialog.ui") #, window)
#~ ui.show()
#~ #ui.setupUi(window)

else:
    app.exec_()
