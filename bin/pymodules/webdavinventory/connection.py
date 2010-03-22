'''
Created on 8.9.2009

@author: jonnena
'''

import httplib
import urllib

from httplib import HTTPConnection, HTTPException
from webdav import WebdavClient
from webdav.Connection import WebdavError, AuthorizationError

import rexviewer #for logging

CABLEBEACH_IDENTITY_HEADER = "CableBeach-Identity"
CABLEBEACH_WEBDAV_HEADER = "CableBeach-WebDavInventory"

class HTTPClient(object):
    
    def __init__(self):
        self.httpclient = None
        self.identityQueryString = None
        self.storedHost = None
        self.storedIdentityType = None
        self.storedIdentity = None
    
    def setupConnection(self, host, identityType, identity = None, firstName = None, lastName = None):
        strict = None
        port = None
        timeout = 10
        if (self.createRequestURL(identityType, identity, firstName, lastName) == False):
            return False
        self.httpclient = HTTPConnection(host, port, strict, timeout)
        self.storedHost = host
    
    def createRequestURL(self, identityType, identity, firstName, lastName):
        self.identityQueryString = "/services/webdav?type=" + identityType + "&"
        if (identityType == "openid" and identity != None):
            self.identityQueryString += "identity=" + identity
            self.storedIdentityType = 0
            self.storedIdentity = identity
        elif (identityType == "normal" and firstName != None and lastName != None):
            self.identityQueryString += "firstname=" + firstName + "&lastname=" + lastName
            self.storedIdentityType = 1
            self.storedIdentity = firstName + " " + lastName
        else:
            return False
        
        
    def requestIdentityAndWebDavURL(self):
        identityurl = None
        webdavurl = None
        if self.httpclient is not None:
            try:
                self.httpclient.request("GET", self.identityQueryString)
            except httplib.socket.error:
                raise httplib.error
            response = self.httpclient.getresponse()
            if (response.status == httplib.OK):
                identityurl = response.getheader(CABLEBEACH_IDENTITY_HEADER)
                webdavurl = response.getheader(CABLEBEACH_WEBDAV_HEADER)
            elif (response.status == httplib.NOT_FOUND):
                identityurl = None
                webdavurl = None
            self.httpclient.close()
            return identityurl, webdavurl
        else:
            raise RuntimeError, "http client not there yet, you should call setupConnection first"

class WebDavClient(object):
        
    connection = None
    resource = None
    url = None
    user = None
    password = None
    
    def __init__(self, identityurl = None, webdavurl = None):
        self.user = identityurl
        self.url = webdavurl
        
    def setHostAndUser(self, user, url):
        self.user = user
        self.url = url
    
    def setupConnection(self):
        if (self.connection != None):
            self.connection = None
        try:
            self.resource = WebdavClient.CollectionStorer(self.url, self.connection)
            self.determineAuthentication()
            return True
        except WebdavError:
            raise HTTPException
            
    def determineAuthentication(self):       
        authFailures = 0
        while authFailures < 2:
            try:
                self.resource.getCollectionContents()
            except AuthorizationError as e:                           
                if e.authType == "Basic":
                    #self.password = QInputDialog.getText(None, "WebDav Inventory Password", "WebDav Inventory is asking for Basic authentication\n\nPlease give your password:", QLineEdit.Normal, "")
                    self.password = "" # CableBeach server does not check passwords yet
                    self.resource.connection.addBasicAuthorization(self.user, str(self.password))
                elif e.authType == "Digest":
                    #self.password = QInputDialog.getText(None, "WebDav Inventory Password", "WebDav Inventory is asking for Digest authentication\n\nPlease give your password:", QLineEdit.Normal, "")
                    self.password = "" # CableBeach server does not check passwords yet
                    info = WebdavClient.parseDigestAuthInfo(e.authInfo)
                    self.resource.connection.addDigestAuthorization(self.user, self.password, realm=info["realm"], qop=info["qop"], nonce=info["nonce"])
                else:
                    raise WebdavError
            except HTTPException:
                raise HTTPException
            authFailures += 1
            
    def resendAuthorization(self, AuthError):
        print ''
        print '---------------------------------------------------------'
        print 'AuthorizationError was raised, trying to resend auth data'
        print '---------------------------------------------------------'
        if AuthError.authType == "Basic":
            self.resource.connection.addBasicAuthorization(self.user, str(self.password))
        elif AuthError.authType == "Digest":
            info = WebdavClient.parseDigestAuthInfo(AuthError.authInfo)
            self.resource.connection.addDigestAuthorization(self.user, self.password, realm=info["realm"], qop=info["qop"], nonce=info["nonce"])
        else:
            raise WebdavError
        try:
            self.resource.getCollectionContents()
            print "----------- RE-AUTHENTICATION SUCCESFULL ------------\n"
        except AuthorizationError as e:
            print "-------- TRIED TO RE-AUTHENTICATE BUT FAILED --------\n"
            print "Please restart your viewer and reconnect to fix,"
            print "webdav inventory is in early dev state and "
            print "reauthentication is not fully working."
        
    def listResources(self, path = None):
        if (path != None):
            if (self.setCollectionStorerToPath(path)):
                try:
                    resourceList = self.resource.listResources()
                    return self.parseResourceList(resourceList)
                except AuthorizationError as AuthError:
                    self.resendAuthorization(AuthError)
                except WebdavError:
                    return WebdavError.__str__
            else:
                return 'False'
        else:
            try:
                resourceList = self.resource.listResources()
                return self.parseResourceList(resourceList)
            except AuthorizationError as AuthError:
                self.resendAuthorization(AuthError)
            except WebdavError:
                return WebdavError.__str__
 

    def parseResourceList(self, resourceList):
        keyvalues = []
        for itemPath in resourceList:
            type = resourceList[itemPath].getResourceType()
            if (itemPath[0] == "/"):
                itemPath = itemPath[1:]
            keyvalues.append(itemPath)
            keyvalues.append(type)
        return keyvalues

    def setCollectionStorerToPath(self, path):
        try:
            self.resource = WebdavClient.CollectionStorer(self.url + path, self.resource.connection)
            return True
        except AuthorizationError as AuthError:
            self.resendAuthorization(AuthError)
        except WebdavError:
            return False
    
    def downloadFile(self, filePath, collectionWebDavPath, resourceWebDavName):
        if ( self.setCollectionStorerToPath(collectionWebDavPath) ):
            try:
                webdavResource = self.resource.getResourceStorer(resourceWebDavName)
                webdavResource.downloadFile(filePath + "/" + resourceWebDavName)
                return 'True'
            except Exception:
                return 'False'
        else:
            return 'False'
        
    def uploadFile(self, filePath, collectionWebDavPath, resourceWebDavName):
        dataFile = None
        rexviewer.logDebug("WebDav Connection Uploading file: %s" % filePath)
        try:
            dataFile = open(filePath, 'rb')
        except IOError:
            dataFile.close()
            return 'False'
        if ( self.setCollectionStorerToPath(collectionWebDavPath) ):
            try:
                self.resource.addResource(resourceWebDavName, dataFile.read())
                dataFile.close()
                return 'True'
            except WebdavError, IOError:
                dataFile.close()
                return WebdavError.__str__
        else:
            dataFile.close()
            return 'False'

    def uploadFileBuffer(self, dataBuffer, collectionWebDavPath, resourceWebDavName):
        if ( self.setCollectionStorerToPath(collectionWebDavPath) ):
            try:
                self.resource.addResource(resourceWebDavName, dataBuffer)
                return 'True'
            except WebdavError, IOError:
                return WebdavError.__str__
        else:
            return 'False'

    def createDirectory(self, collectionWebDavPath, directoryName):
        if ( self.setCollectionStorerToPath(collectionWebDavPath) ):
            try:
                self.resource.addCollection(directoryName)
                return 'True'
            except AuthorizationError as AuthError:
                self.resendAuthorization(AuthError)
            except WebdavError:
                return WebdavError.__str__
        else:
            return 'False'
            
    def deleteResource(self, collectionWebDavPath, resourceWebDavName):
        if (self.setCollectionStorerToPath(collectionWebDavPath)):
            try:
                webdavResource = self.resource.getResourceStorer(resourceWebDavName)
                webdavResource.delete()
                return 'True'
            except AuthorizationError as AuthError:
                self.resendAuthorization(AuthError)
            except WebdavError:
                return WebdavError.__str__
        else:
            return 'False'
        
    def deleteDirectory(self, collectionWebDavPath, collectionName):
        if (self.setCollectionStorerToPath(collectionWebDavPath)):
            try:
                self.resource.deleteResource(collectionName)
                return 'True'
            except AuthorizationError as AuthError:
                self.resendAuthorization(AuthError)
            except WebdavError:
                return WebdavError.__str__
        else:
            return 'False'

    def copyDirectory(self, currentPath, newPath, itemName, deepCopy = 'False'):
        if (self.setCollectionStorerToPath(currentPath)):
            try:
                if (deepCopy == 'True'):
                    infinity = True
                else:
                    infinity = False
                newPath = self.url + newPath
                print 'COPY DIRECTORY ==============='
                print 'Item         : ' + itemName
                print 'Current path : ' + currentPath
                print 'New path     : ' + newPath
                print 'Deep copy    : ' + deepCopy

                webdavResource = self.resource.getResourceStorer(itemName)
                webdavResource.copy(newPath, infinity)
                # try also
                # webdavResource.move(newPath)
                return 'True'
            except WebdavError:
                return WebdavError.__str__
        else:
            return 'False'
            
    def renameResource(self, currentPath, newName, oldName):
        if (self.setCollectionStorerToPath(currentPath)):
            try:
                newName = self.url + urllib.quote(currentPath + newName)
                webdavResource = self.resource.getResourceStorer(oldName)
                webdavResource.move(newName)
                return 'True'
            except AuthorizationError as AuthError:
                self.resendAuthorization(AuthError)
            except WebdavError:
                return WebdavError.__str__
        else:
            return 'False'
            
    def moveResource(self, currentPath, newPath, resourceName):
        if (self.setCollectionStorerToPath(currentPath)):
            try:
                newPath = self.url + urllib.quote(newPath + resourceName)
                webdavResource = self.resource.getResourceStorer(resourceName)
                webdavResource.move(newName)
                return 'True'
            except AuthorizationError as AuthError:
                self.resendAuthorization(AuthError)
            except WebdavError:
                return WebdavError.__str__
        else:
            return 'False'
