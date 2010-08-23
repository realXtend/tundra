import circuits
import rexviewer as r
import naali
import urllib2 #for js_src downloading

"""a registry of component handlers, by type"""
handlertypes = {}
def register(compname, handlertype):
    handlertypes[compname] = handlertype

import animsync
register(animsync.COMPNAME, animsync.AnimationSync)

import door
register(door.COMPNAME, door.DoorHandler)

class ComponenthandlerRegistry(circuits.BaseComponent):
    def __init__(self):
        circuits.BaseComponent.__init__(self)
    #    self.handlerinstances = []

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
            if comp.Name in handlertypes:
                handlertypes[comp.Name](entity, comp, changetype)
                #self.handlerinstances.append(h)

            #if the data was there already, could do this.
            #but it's not - must now listen to onChanged and check instead
            #jssrc = comp.GetAttribute("js_src")
            #print "JS SRC:", jssrc
            #if jssrc is not None:
            #    self.apply_js(jssrc)
            jscheck = make_jssrc_handler(entity, comp, changetype)
            comp.connect("OnChanged()", jscheck)

def make_jssrc_handler(entity, comp, changetype):
    #def handle_js():
    class JsHandler(): #need a functor so that can disconnect itself
        def __call__(self):
            jssrc = comp.GetAttribute("js_src")
            print "JS SRC:", jssrc
            if jssrc is not None:
                apply_js(jssrc, comp)
            comp.disconnect("OnChanged()", self)
    return JsHandler()

def apply_js(jssrc, comp):
    jscode = loadjs(jssrc)

    print jscode

    ctx = {
        #'entity'/'this': self.entity
        'component': comp
    }

    ent = r.getEntity(comp.GetParentEntityId())
    try:
        ent.touchable
    except AttributeError:
        pass
    else:
        ctx['touchable'] = ent.touchable
    try:
        ent.placeable
    except:
        pass
    else:
        ctx['placeable'] = ent.placeable
            
    naali.runjs(jscode, ctx)
    print "-- done with js"

def loadjs(srcurl):
    print "js source url:", srcurl
    f = urllib2.urlopen(srcurl)
    return f.read()
