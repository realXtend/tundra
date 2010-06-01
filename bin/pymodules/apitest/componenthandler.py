import circuits
import rexviewer as r
import naali

class DynamiccomponentHandler(circuits.Component):
    GUINAME = "DynamicComponent handler"

    def __init__(self):
        circuits.Component.__init__(self)
        obid = None 
        self.comp = None
        if obid is not None:
            ent = r.getEntity(obid)
            self.add_component(ent)

        self.widget = None
        self.initgui()
        if self.widget is not None:
            self.registergui()

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
        print "Scene added:", name#,
        s = naali.getScene(name)

        #s.connect("ComponentInitialized(Foundation::ComponentInterface*)", self.onComponentInitialized)
        s.connect("ComponentAdded(Scene::Entity*, Foundation::ComponentInterface*, Foundation::ComponentInterface::ChangeType)", self.onComponentAdded)

    #def onComponentInitialized(self, comp):
    #    print "Comp inited:", comp

    def onComponentAdded(self, entity, comp, changetype):
        print "Comp added:", entity, comp, changetype
        #print comp.className()
        if comp.className() == "EC_DynamicComponent":
            comp.connect("OnChanged()", self.onChanged)
            self.comp = comp
            print "DYNAMIC COMPONENT FOUND", self.comp

    def add_component(self, ent):
        try:
            ent.dynamic
        except AttributeError:
            ent.createComponent("EC_DynamicComponent")
        print ent.dynamic
            
        comp = ent.dynamic
        #print dir(d)
        #d.AddAttribute()
        print d.GetAttribute()

        d.connect("OnChanged()", self.onChanged)
        self.comp = comp

    def on_exit(self):
        uism = r.getUiSceneManager()
        uism.RemoveProxyWidgetFromScene(self.proxywidget)


