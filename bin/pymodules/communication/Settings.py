
#import ConfigParser
import os
#import string


class Settings:
    def __init__(self):
        self.connectionParams={}
##        self.conf = ConfigParser.ConfigParser()
        pass



    def LoadSettings(self): #testing
    #def GetSettings(self): #testing
##        self._loadSettings()
##        self._printSettings()
        d = self.loadCommunicationIni()
        print(d)
        for key in d.iterkeys():
            classkey = key.replace('-','_')
            setattr(self, classkey, d[key])
        self.connectionParams = d
        return d

    def SaveSettings(self, attr_N):
        splitArray = attr_N.split(':')
        #saveStr = ""
        lines = []
        for i in range(0, len(splitArray)/2):
            key = splitArray[i*2]
            val = splitArray[i*2+1]
            lines.append(key + "=" + val + "\n")
            #saveStr = saveStr + key + "=" + value + "\n"
            
        iniFile = open("data"+os.sep+"connection.ini","w")
        iniFile.writelines(lines)
        iniFile.close()

    def GetSettingsForConnecting(self):
        return self.connectionParams

    def GetSettingsForUI(self):
        d = self.loadCommunicationIni()
        print(d)
        self.connectionParams = d
        return self._getSettingsInOneString(self.connectionParams)

    def _getSettingsInOneString(self, d):
        """ Returning one string keys and values separated by ':' value corresponding to key comes always after the key and every other parameter is key starting from first """
        settingStr=""
        for key in d.iterkeys():
            settingStr = settingStr + key
            settingStr = settingStr + ":"
            settingStr = settingStr + str(d[key])
            settingStr = settingStr + ":"
        settingStr = settingStr[0:len(settingStr)-1]
        return settingStr
        
    
    def loadCommunicationIni(self):
        """ Doing it the more dynamic way """
        print "loading connection.ini file"
        try:
            accoutFileStr = file("data"+os.sep+"connection.ini").read()
            d = self.parse_account(accoutFileStr)
        except:
            pairs = [] 
            d = dict(pairs)
        return d

    def parse_account(self, s):
        lines = s.splitlines()
        pairs = []        
        manager = None
        protocol = None

        for line in lines:
            if(not(line.startswith('['))):
                if not line.strip():
                    continue
                #k, v = line.split(':', 1)
                k, v = line.split('=', 1)
                k = k.strip()
                v = v.strip()

                if k == 'manager':
                    manager = v
                    pairs.append((k, v))
                elif k == 'protocol':
                    protocol = v
                    pairs.append((k, v))
                else:
                    if k not in ("account", "password"):
                        if v.lower() == "false":
                            v = False
                        elif v.lower() == "true":
                            v = True
                        else:
                            try:
                                v = dbus.UInt32(int(v))
                            except:
                                pass
                    pairs.append((k, v))
                    
        d = dict(pairs)
##        a = d['account']
##        print a
        return d