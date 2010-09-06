
import sys
import rexviewer as r
from xml.dom.minidom import parseString

class XmlBaseParser:
    def __init__(self, xmlstring):
        self.xml = xmlstring
        pass
    def correctUtfCoding(self, xmlstr):
        #read encoding
        start = xmlstr.find("encoding") + 10
        stop = xmlstr.find("\"", start)
        utfCoding = xmlstr[start:stop]
        isAscii = self.is_ascii(xmlstr)
        startStr = xmlstr[:start]
        endStr = xmlstr[stop:]
        if(utfCoding=="utf-8"and isAscii):
            # as it should be
            pass
        elif(utfCoding=="utf-8"and not isAscii):
            # need to change it
            return startStr + "utf-16" + endStr
            pass
        elif(utfCoding=="utf-16"and not isAscii):
            # as it should be
            pass        
        elif(utfCoding=="utf-16"and isAscii):
            # need to change it
            return startStr + "utf-8" + endStr
            pass
        pass
    def is_ascii(self,s):
        return all(ord(c) < 128 for c in s)
        


class XmlStringDictionaryParser:
    
    def __init__(self, xmlstring):
        self.xml = xmlstring
        pass
        
    def parse(self):
        self.xml = self.correctUtfCoding(self.xml)
        d = {}
        #try:
        print self.xml
        if not (self.xml==None):
            dom = parseString(self.xml)
            dictionaryElem = dom.getElementsByTagName('dictionary')[0]
            items = dictionaryElem.getElementsByTagName('item')
            for item in items:
                keyElem = item.getElementsByTagName('key')[0]
                valueElem = item.getElementsByTagName('value')[0]
                keyStringElem = keyElem.getElementsByTagName('string')[0]
                valueStringElem = valueElem.getElementsByTagName('string')[0]
                d[keyStringElem.childNodes[0].nodeValue]=valueStringElem.childNodes[0].nodeValue
            return d
        # except:
            # r.logInfo("Failed to parse xml")
            # r.logInfo("Unexpected error:")
            # r.logInfo(str(sys.exc_info()[0]))
            # return None
            # #raise

    def correctUtfCoding(self, xmlstr):
        #read encoding
        start = xmlstr.find("encoding") + 10
        stop = xmlstr.find("\"", start)
        utfCoding = xmlstr[start:stop]
        isAscii = self.is_ascii(xmlstr)
        startStr = xmlstr[:start]
        endStr = xmlstr[stop:]
        
        if(utfCoding=="utf-8"and isAscii):
            # as it should be
            pass
        elif(utfCoding=="utf-8"and not isAscii):
            # need to change it
            return startStr + "utf-16" + endStr
            pass
        elif(utfCoding=="utf-16"and not isAscii):
            # as it should be
            pass        
        elif(utfCoding=="utf-16"and isAscii):
            # need to change it
            return startStr + "utf-8" + endStr
            pass
        #xmlstr
        pass
        
    def is_ascii(self,s):
        return all(ord(c) < 128 for c in s)
        

class XmlSceneRegionResponceParser(XmlBaseParser):
    def __init__(self, xmlstring):
        XmlBaseParser.__init__(self, xmlstring)
    def parse(self):
        self.xml = self.correctUtfCoding(self.xml)
        d = {}
        if not (self.xml==None):
            dom = parseString(self.xml)
            dictionaryElem = dom.getElementsByTagName('dictionary')[0]
            items = dictionaryElem.getElementsByTagName('item')
            for item in items:
                keyElem = item.getElementsByTagName('key')[0]
                valueElem = item.getElementsByTagName('value')[0]
                keyStringElem = keyElem.getElementsByTagName('string')[0]
                sceneRegion = valueElem.getElementsByTagName('SceneRegion')[0]
                sceneName = sceneRegion.getElementsByTagName('SceneName')[0]
                region = sceneRegion.getElementsByTagName('Region')[0]
                sceneUuid = sceneRegion.getElementsByTagName('SceneUuid')[0]
                sceneNameValue = sceneName.childNodes[0].nodeValue
                sceneUuidValue = sceneUuid.childNodes[0].nodeValue
                print region.childNodes.length
                regionValue = ""
                if(region.childNodes.length!=0):
                    regionValue = region.childNodes[0].nodeValue
                d[keyStringElem.childNodes[0].nodeValue]=(sceneNameValue,regionValue,sceneUuidValue)
        return d
    pass
        