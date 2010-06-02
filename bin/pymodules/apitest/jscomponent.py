import json
import rexviewer as r
import urllib2

from componenthandler import DynamiccomponentHandler

class JavascriptHandler(DynamiccomponentHandler):
    GUINAME = "Javascript Handler"

    def onChanged(self):
        ent = r.getEntity(self.comp.GetParentEntityId())
        datastr = self.comp.GetAttribute()
        print "GetAttr got:", datastr

        data = json.loads(datastr)
        js_src = data.get('js_src', None)
        if js_src is not None:
            print "js source url:", js_src
            f = urllib2.urlopen(js_src)
            print f.read()

    def on_exit(self):
        pass
