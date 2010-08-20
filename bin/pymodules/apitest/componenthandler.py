import circuits
import rexviewer as r
import naali

class DynamiccomponentHandler(circuits.BaseComponent):
    GUINAME = "DynamicComponent handler"

    def __init__(self):
        self.comp = None

        circuits.BaseComponent.__init__(self)
        self.widget = None
        self.proxywidget = None
        self.initgui()

    def initgui(self):
        pass #overridden in subclasses

    def registergui(self):
        uism = r.getUiSceneManager()
        self.proxywidget = r.createUiProxyWidget(self.widget)
        self.proxywidget.setWindowTitle(self.GUINAME)
        if not uism.AddWidgetToScene(self.proxywidget):
            print "Adding the ProxyWidget to the bar failed."
        uism.AddWidgetToMenu(self.proxywidget, self.GUINAME, "Developer Tools")

    @circuits.handler("on_sceneadded")
    def on_sceneadded(self, name):
        #print "Scene added:", name#,
        s = naali.getScene(name)

        #s.connect("ComponentInitialized(Foundation::ComponentInterface*)", self.onComponentInitialized)
        s.connect("ComponentAdded(Scene::Entity*, Foundation::ComponentInterface*, AttributeChange::Type)", self.onComponentAdded)

    #def onComponentInitialized(self, comp):
    #    print "Comp inited:", comp

    def onComponentAdded(self, entity, comp, changetype):
        #print "Comp added:", entity, comp, changetype
        #print comp.className()
        if comp.className() == "EC_DynamicComponent":
            print "comp Name:", comp.Name
            if self.comp is None: #XXX check that the component name is of interest
                comp.connect("OnChanged()", self.onChanged)
                self.comp = comp
                print "DYNAMIC COMPONENT FOUND", self.comp
            else:
                print "ANOTHER DynamicComponent found - only one supported now, ignoring", entity, comp

    @circuits.handler("on_logout")
    def on_logout(self, idt):
        if self.comp is not None:
            try:
                self.comp.disconnect("OnChanged()", self.onChanged)
            finally: #disconnect fails if the entity had been deleted
                self.comp = None

    @circuits.handler("on_exit")
    def on_exit(self):
        if self.proxywidget is not None:
            uism = r.getUiSceneManager()
            uism.RemoveWidgetFromScene(self.proxywidget)


