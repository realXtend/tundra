'''
Created on 8.9.2009

@author: jonnena
'''

import gui
from httplib import HTTPException

from connection import WebDavClient
from connection import HTTPClient

class MainFunction():
    
    httpclient = None
    webdavclient = None
    
    identityurl = None
    webdavurl = None
    
    host = None
    identityType = None
    identity = None
    firstName = None
    lastName = None

    def __init__(self, host, identityType, identity = None, firstName = None, lastName = None):
        """ INIT INPUT PARAMETERS TO LOCALS """
        self.host = host
        self.identityType = identityType
        self.identity = identity
        self.firstName = firstName
        self.lastName = lastName
        """ START HTTP/WEBDAV CLIENTS """
        connectSuccess = self.connectClients()
        self.app = gui.Application(self.webdavclient, connectSuccess)
        if connectSuccess == False:
            self.identity = self.app.myIdentity
            self.host = self.app.myWebDav
            
    def connectClients(self):
        self.httpclient = HTTPClient()
        """ SETUP HTTP CLIENT AND CREATE QUERY STRING """
        if ( self.httpclient.setupConnection(self.host, self.identityType, self.identity, self.firstName, self.lastName) != False ):
            """ REQUEST WEBDAV AND IDENTITY URL's FROM HOST """
            try:
                self.identityurl, self.webdavurl = self.httpclient.requestIdentityAndWebDavURL()
            except HTTPException:
                return False
            if (self.identityurl != None and self.webdavurl != None):
                self.webdavclient = WebDavClient(self.identityurl, self.webdavurl)
                try:
                    self.webdavclient.setupConnection()
                    return True
                except HTTPException:
                    return False

if __name__ == '__main__':
    MainFunction("127.0.0.1:8002", "openid", "http://jonnenauha.com:8004/jonnenauha")
