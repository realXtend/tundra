import json
import rexviewer as r
import naali
import urllib2

from componenthandler import DynamiccomponentHandler

class JavascriptHandler(DynamiccomponentHandler):
    GUINAME = "Javascript Handler"

    def __init__(self):
        DynamiccomponentHandler.__init__(self)
        self.jsloaded = False

    def onChanged(self):
        print "-----------------------------------"
        ent = r.getEntity(self.comp.GetParentEntityId())
        datastr = self.comp.GetAttribute()
        #print "GetAttr got:", datastr

        data = json.loads(datastr)
        js_src = data.get('js_src', None)
        if not self.jsloaded and js_src is not None:
            jscode = self.loadjs(js_src)
            
            print jscode

            ctx = {
                #'entity'/'this': self.entity
                'component': self.comp
                }

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
            
            self.jsloaded = True

    def loadjs(self, srcurl):
        print "js source url:", srcurl
        f = urllib2.urlopen(srcurl)
        return f.read()

