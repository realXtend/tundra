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
            self.loadjs(js_src)

    def loadjs(self, srcurl):
        print "js source url:", srcurl
        f = urllib2.urlopen(srcurl)
        code = f.read()
        print code
        ctx = {
            'component': self.comp
            }
        naali.runjs(code, ctx)
        print "-- done with js"

        self.jsloaded = True

