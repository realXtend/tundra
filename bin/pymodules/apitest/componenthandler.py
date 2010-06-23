import circuits
import rexviewer as r
import naali

class DynamiccomponentHandler(circuits.Component):
    GUINAME = "DynamicComponent handler"

    def __init__(self):
        circuits.Component.__init__(self)
        self.comp = None
        self.widget = None
        self.proxywidget = None
        self.initgui()

    def initgui(self):
        pass #overridden in subclasses

    def registergui(self):
        uism = r.getUiSceneManager()
        uiprops = r.createUiWidgetProperty(1)
        uiprops.widget_name_ = self.GUINAME
        self.proxywidget = r.createUiProxyWidget(self.widget, uiprops)
        if not uism.AddProxyWidget(self.proxywidget):
            print "Adding the ProxyWidget to the bar failed."

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
            if self.comp is None:
                comp.connect("OnChanged()", self.onChanged)
                self.comp = comp
                print "DYNAMIC COMPONENT FOUND", self.comp
            else:
                print "ANOTHER DynamicComponent found - only one supported now, ignoring", entity, comp

    def on_logout(self, idt):
        if self.comp is not None:
            self.comp.disconnect("OnChanged()", self.onChanged)
            self.comp = None

    def on_exit(self):
        if self.proxywidget is not None:
            uism = r.getUiSceneManager()
            uism.RemoveProxyWidgetFromScene(self.proxywidget)


