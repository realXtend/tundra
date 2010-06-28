
"""
Class for handling estate and region setting modifications
with EstateRegionSettingsModification capability
"""

import rexviewer as r

# from poster.encode import encode_string #,multipart_encode
# from poster.streaminghttp import register_openers
import urllib2
import urllib


class EstateSettings:

    def __init__(self, worldstream, controller):
        self.worldstream = worldstream
        self.controller = controller
        pass
        
    def setAccessMode(self, mode):
        # TODO: execute this stuff under thread
        estatemod_url = self.worldstream.GetCapability('EstateRegionSettingsModification')
        #print "estatemod_url: ", estatemod_url
        if estatemod_url == "":
            self.controller.queue.put(('noCapability', 'Check your rights to modify estate settings'))
            return
        if mode == True:
            values = {'PublicAccess' : 'True'}
        else:
            values = {'PublicAccess' : 'False'}
        try: 
            resp = self.sendRequest(estatemod_url, values)
            self.processResponseAccessModeResp(resp)
        except:
            pass
        pass
        
    def sendRequest(self, estatemod_url, values):
        try: 
            data = urllib.urlencode(values)
            req = urllib2.Request(estatemod_url, data)
            response = urllib2.urlopen(req)
            respdata = response.read()
            return respdata
        except:
            return "failed"
            
    def processResponseAccessModeResp(self, resp):
        try:
            if resp.startswith('Success:'):
                # do check that ui setting is right
                setting = resp.split('=')[1]
                self.controller.queue.put(('estateSetting', setting))
            elif resp.startswith('Failure:'):
                failMessage = resp.split('=')[1]
                #print failMessage
                self.controller.queue.put(('failedEstateSetting', failMessage))
            else:
                self.controller.queue.put(('unknownResponse', 'Server send unknown response'))
            pass
        except:
            self.controller.queue.put(('malformedResp', 'Something went wrong with processing estate setting responce'))
            
            
    def fetchEstates(self):
        hasCap, cap = self.checkCapability()
        if hasCap:
            values = {'GetEstates' : 'True'}
            try: 
                resp = self.sendRequest(cap, values)
                self.processFetchEstatesResp(resp)
            except:
                self.controller.queue.put(('malformedResp', 'Something went wrong with processing estate setting responce'))
            pass

    def setRegionEstate(self, estateData):
        id = estateData.split("=")[0]
        name = estateData.split("=")[1]
        hasCap, cap = self.checkCapability()
        if hasCap:
            values = {'SetRegionEstate' : id}
            try: 
                resp = self.sendRequest(cap, values)
                self.processSetEstatesResp(resp, name)
            except:
                self.controller.queue.put(('malformedResp', 'Something went wrong with setting estate'))
            pass
        
        
        
    def checkCapability(self):
        estatemod_url = self.worldstream.GetCapability('EstateRegionSettingsModification')
        #print "estatemod_url: ", estatemod_url
        if estatemod_url == "":
            self.controller.queue.put(('noCapability', 'Check your rights to set access'))
            return False, ""
        else:
            return True, estatemod_url
            
    def processFetchEstatesResp(self, resp):
        try:
            if resp.startswith('Success:'):
                split = resp.split('\n')
                currentEstateIdAndName = split[1].split(':')[1]
                estates = split[3:]
                self.controller.queue.put(('currentEstateIdAndName', currentEstateIdAndName))
                self.controller.queue.put(('estates', estates))
                
            elif resp.startswith('Failure:'):
                failMessage = resp.split('=')[1]
                self.controller.queue.put(('failedEstateSetting', failMessage))
            else:
                self.controller.queue.put(('unknownResponse', 'Server send unknown response'))
            pass
        except:
            self.controller.queue.put(('malformedResp', 'Something went wrong with processing fetch estates responce'))
            
    def processSetEstatesResp(self, resp, name):
        try:
            if resp.startswith('Success:'):
                #TODO: parse resp
                split = resp.split(':')
                id = split[2]
                self.controller.queue.put(('currentEstateIdAndName', id + "=" + name))
            elif resp.startswith('Failure:'):
                failMessage = resp.split('=')[1]
                self.controller.queue.put(('failedEstateSetting', failMessage))
            else:
                self.controller.queue.put(('unknownResponse', 'Server send unknown response'))
            pass
        except:
            self.controller.queue.put(('malformedResp', 'Something went wrong with processing fetch estates responce'))
        