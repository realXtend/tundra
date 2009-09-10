'''
Created on 8.9.2009

@author: jonnena
'''

from webdav import WebdavClient
from webdav.Connection import WebdavError, AuthorizationError

class WebDav(object):
    
    connection = None
    resource = None
    url = None
    user = None
    password = None
    
    def __init__(self):
        print "[Connection] Initilizing webdav connection to %s with user %s" % (self.url, self.user)
    
    def setConnection(self, url, user, password):
        self.url = url
        self.user = user
        self.password = ""# = password # change to None when really  using passwords
        if (self.connection != None):
            self.connection = None
        try:
            self.resource = WebdavClient.CollectionStorer(self.url, self.connection)
            self.tryDigestAuth()
            return True
        except WebdavError:
            return False
            
    def tryDigestAuth(self):
        authFailures = 0
        while authFailures < 2:
            try:
                self.resource.getCollectionContents()
            except AuthorizationError as e:                           
                if e.authType == "Basic":
                    self.resource.connection.addBasicAuthorization(self.user, self.password)
                elif e.authType == "Digest":
                    info = WebdavClient.parseDigestAuthInfo(e.authInfo)
                    self.resource.connection.addDigestAuthorization(self.user, self.password, realm=info["realm"], qop=info["qop"], nonce=info["nonce"])
                else:
                    raise           
            authFailures += 1
        
    def listResources(self, path):
        if (path != None):
            if (self.setCollectionStorerToPath(path)):
                try:
                    resourceList = self.resource.listResources()
                    return resourceList
                except WebdavError:
                    return False 
            else:
                return False
        else:
            try:
                resourceList = self.resource.listResources()
                return resourceList
            except WebdavError:
                return False 

    
    def setCollectionStorerToPath(self, path):
        try:
            self.resource = WebdavClient.CollectionStorer(self.url + path, self.resource.connection)
            return True
        except WebdavError:
            return False

    def setResourceStorerToPath(self, path):
        self.resource = WebdavClient.ResourceStorer(self.url + path, None)
        self.tryDigestAuth()
        return True
    
    def downloadFile(self, filePath, collectionWebDavPath, resourceWebDavName):
        if ( self.setCollectionStorerToPath(collectionWebDavPath) ):
            try:
                child = self.resource.getResourceStorer(resourceWebDavName)
                child.downloadFile(filePath + "/" + resourceWebDavName)
                return True
            except Exception:
                return False
        else:
            return False
        
    def uploadFile(self, filePath, collectionWebDavPath, resourceWebDavName):
        try:
            dataFile= open(filePath, 'rb')
        except IOError:
            return False
        if ( self.setCollectionStorerToPath(collectionWebDavPath) ):         
            self.resource.addResource(resourceWebDavName, dataFile.read())
            dataFile.close()
            return True
        else:
            return False
        
    def createDirectory(self, collectionWebDavPath, directoryName):
        if ( self.setCollectionStorerToPath(collectionWebDavPath) ):
            try:
                self.resource.addCollection(directoryName)
                return True
            except WebdavError:
                return False
            