
# Class handling other actions than scene uploading, which is done in sceneuploader.py

import rexviewer as r

from poster.encode import multipart_encode
from poster.streaminghttp import register_openers
import urllib2
import sceneactionsxml



class SceneActions:
    
    def __init__(self, cap_url, controller):
        self.controller = controller
        self.cap_url = cap_url        
        strict = None
        # poster init
        register_openers()
        self.sceneActions = None # sceneactions.SceneActions()
        self.currentSceneAction = ""
        self.sceneActionParam = ""
        self.ongoingGetUploadSceneList = False
        self.ongoingDeleteServerScene = False
        self.ongoingUnloadServerScene = False
        self.ongoingGetLoadServerScene = False
        pass

    def timeout(self, func, args=(), kwargs={}, timeout_duration=60.0, default=None):
        import threading
        class InterruptableThread(threading.Thread):
            def __init__(self):
                threading.Thread.__init__(self)
                self.result = None

            def run(self):
                try:
                    self.result = func(*args, **kwargs)
                except:
                    self.result = default

        it = InterruptableThread()
        it.start()
        it.join(timeout_duration)
        if it.isAlive():
            return default
        else:
            return it.result

        
    def redoRegisterOpeners(self):
        register_openers()
        
    def runSceneAction(self):
        if(self.isOngoingAction(self.currentSceneAction)==False):
            ret = self.timeout(self.sendRequest, (self.currentSceneAction, self.sceneActionParam))
            if(ret==None):
                self.controller.queue.put(('scene upload', 'timed out'))
            #self.sendRequest(self.currentSceneAction, self.sceneActionParam)
            self.currentSceneAction=""
            self.controller.queue.put(('scene upload', 'scene action done'))

    def isOngoingAction(self, action):
        if(action=="GetUploadSceneList"):            
            if(self.ongoingGetUploadSceneList == False):
                return False
            else:
                self.ongoingGetUploadSceneList = True
                return True
            pass
        elif(action=="DeleteServerScene"):
            if(self.ongoingDeleteServerScene == False):
                return False
            else:
                self.ongoingDeleteServerScen = True
                return True
            pass
        elif(action=="UnloadServerScene"):
            if(self.ongoingUnloadServerScene == False):
                return False
            else:
                self.ongoingUnloadServerScen = True
                return True
            pass
        elif(action=="LoadServerScene"):
            if(self.ongoingGetLoadServerScene == False):
                return False
            else:
                self.ongoingGetLoadServerScen = True
                return True
            pass
        return False;
        
        
    def sendRequest(self, action, param):
        if(param==None):
            param="None"
        if(action=="UnloadServerScene"):
            key, region = param
            param = key + ":" + region
        datagen, headers = multipart_encode({"uploadscene": param})
        headers['USceneMethod']=action
        request = urllib2.Request(self.cap_url, datagen, headers) # post
        responce = urllib2.urlopen(request).read()
        r.logInfo(responce)
        if(action=="GetUploadSceneList"):
            parser = sceneactionsxml.XmlSceneRegionResponceParser(responce)
            d = parser.parse()
            if d.has_key('error'):
                self.handleErrors(d)
            self.controller.window.setServerScenes(d)
            self.ongoingGetUploadSceneList = False
            pass
        elif(action=="DeleteServerScene"):
            parser = sceneactionsxml.XmlStringDictionaryParser(responce)
            d = parser.parse()
            self.ongoingDeleteServerScene = False
            self.handleErrors(d)
            pass
        elif(action=="UnloadServerScene"):
            parser = sceneactionsxml.XmlStringDictionaryParser(responce)
            d = parser.parse()
            self.ongoingUnloadServerScene = False
            self.handleErrors(d)
            pass
        elif(action=="LoadServerScene"):
            parser = sceneactionsxml.XmlStringDictionaryParser(responce)
            d = parser.parse()
            self.ongoingGetLoadServerScene = False
            self.handleErrors(d)
            pass
        return 1
            
    def handleErrors(self, d):
        #print d
        if not d.has_key('error'):
            self.controller.queue.put(('scene upload', 'server sent malformed responce'))
        if(d['error']!='None'):
            self.controller.queue.put(('scene upload', d['error']))
        
        