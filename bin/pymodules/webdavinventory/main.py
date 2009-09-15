'''
Created on 8.9.2009

@author: jonnena
'''

import gui

from connection import WebDavClient
from connection import HTTPClient
from circuits import Component

class MainFunction(object):

    def __init__(self, host, identityType, identity = None, firstName = None, lastName = None):
        self.httpclient = None
        self.webdavclient = None
        
        self.identityurl = None
        self.webdavurl = None
        
        """ INIT INPUT PARAMETERS TO LOCALS """
        self.host = host
        self.identityType = identityType
        self.identity = identity
        self.firstName = firstName
        self.lastName = lastName
        """ START HTTP/WEBDAV CLIENTS """
        connectSuccess = self.connectClients()
        self.app = gui.Application(self.webdavclient, self.httpclient, connectSuccess)

    def connectClients(self):
        self.httpclient = HTTPClient()
        """ SETUP HTTP CLIENT AND CREATE QUERY STRING """
        if ( self.httpclient.setupConnection(self.host, self.identityType, self.identity, self.firstName, self.lastName) != False ):
            """ REQUEST WEBDAV AND IDENTITY URL's FROM HOST """
            try:
                self.identityurl, self.webdavurl = self.httpclient.requestIdentityAndWebDavURL()
            except Exception:
                return False
            if (self.identityurl != None and self.webdavurl != None):
                self.webdavclient = WebDavClient(self.identityurl, self.webdavurl)
                try:
                    self.webdavclient.setupConnection()
                    return True
                except Exception:
                    return False

if __name__ == '__main__':
    """ OpenID way """
    #MainFunction("127.0.0.1:8002", "openid", "http://admino.com:8004/jonnenauha")
    """ OpenSim way """
    MainFunction("127.0.0.1:8002", "normal", None, "Jonne", "Nauha")