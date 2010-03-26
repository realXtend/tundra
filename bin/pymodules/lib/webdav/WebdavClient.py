# pylint: disable-msg=R0904,W0142,W0511,W0104,C0321,E1103,W0212
# 
# Copyright 2008 German Aerospace Center (DLR)
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
 

"""
This module contains the classes ResourceStorer and CollectionStorer for accessing WebDAV resources.
"""


from davlib import XML_CONTENT_TYPE

from urlparse import urlsplit
import types

from webdav import Constants
from webdav.WebdavResponse import LiveProperties
from webdav.WebdavRequests import createFindBody, createUpdateBody, createDeleteBody, createSearchBody
from webdav.Condition import ConditionTerm
from webdav.Connection import Connection, WebdavError, AuthorizationError
from webdav.VersionHandler import VersionHandler

from webdav.acp.Privilege import Privilege
from webdav.acp.Acl import ACL
from webdav.NameCheck import validateResourceName, WrongNameError



__version__ = '$Revision: 13 $'[11:-2]


def switchUnicodeUrlOn(switch):
    """
    Configure whether to use unicode (UTF-8) encoded URLs (default) or
    Latin-1 encoded URLs.
    
    @param switch: 1 if unicode URLs shall be used
    """
    assert switch == 0 or switch == 1, "Pass boolean argument, please."
    Constants.CONFIG_UNICODE_URL = switch

def parseDigestAuthInfo(authInfo):
    import re
    
    info = dict();
    info["realm"] = re.search('realm="([^"]+)"', authInfo).group(1)
    info["qop"] = re.search('qop="([^"]+)"', authInfo).group(1)
    info["nonce"] = re.search('nonce="([^"]+)"', authInfo).group(1)
    
    return info

class ResourceStorer(object):
    """
    This class provides client access to a WebDAV resource
    identified by an URI. It provides all WebDAV class 2 features which include
    uploading data, getting and setting properties qualified by a XML name space,
    locking and unlocking the resource. 
    This class does not cache resource data. This has to be performed by its clients.
    
    @author: Roland Betz
    """
        
    # Instance properties
    url = property(lambda self: str(self.connection) + self.path, None, None, "Resource's URL")

    def __init__(self, url, connection=None, validateResourceNames=True):
        """
        Creates an instance for the given URL
        User must invoke validate() after construction to check the resource on the server.
        
        @param url: Unique resource location for this storer.
        @type  url: C{string}
        @param connection: this optional parameter contains a Connection object 
            for the host part of the given URL. Passing a connection saves 
            memory by sharing this connection. (defaults to None)
        @type  connection: L{webdav.Connection}
        @raise WebdavError: If validation of resource name path parts fails.
        """

        assert connection == None or isinstance(connection, Connection)
        parts = urlsplit(url, allow_fragments=False)
        self.path = parts[2]
        self.validateResourceNames = validateResourceNames
        
        # validate URL path
        for part in self.path.split('/'):
            if part != '' and not "ino:" in part: # explicitly allowing this character sequence as a part of a path (Tamino 4.4)
                if self.validateResourceNames:
                    try:
                        validateResourceName(part)
                    except WrongNameError:
                        raise WebdavError("Found invalid resource name part.")
                self.name = part
        # was: filter(lambda part: part and validateResourceName(part), self.path.split('/'))
        # but filter is deprecated
        
        self.defaultNamespace = None     # default XML name space of properties
        if connection:
            self.connection = connection
        else:
            conn = parts[1].split(":")
            if len(conn) == 1:
                self.connection = Connection(conn[0], protocol = parts[0])  # host and protocol
            else:
                self.connection = Connection(conn[0], int(conn[1]), protocol = parts[0])  # host and port and protocol
        self.versionHandler = VersionHandler(self.connection, self.path)
        

    def validate(self):
        """
        Check whether URL contains a WebDAV resource
        Uses the WebDAV OPTIONS method.
        
        @raise WebdavError: L{WebdavError} if URL does not contain a WebDAV resource
        """
        #davHeader = response.getheader(HTTP_HEADER_DAV)
        davHeader = self.getSpecificOption(Constants.HTTP_HEADER_DAV)
        self.connection.logger.debug("HEADER DAV: %s" % davHeader)
        if not(davHeader) or davHeader.find("2") < 0:   # DAV class 2 supported ?
            raise WebdavError("URL does not support WebDAV", 0)

    def options(self):
        """
        Send an OPTIONS request to server and return all HTTP headers.
        
        @return: map of all HTTP headers returned by the OPTIONS method.
        """
        response = self.connection.options(self.path)
        result = {}
        result.update(response.msg)
        self.connection.logger.debug("OPTION returns: " + str(result.keys()))
        return result
        
    def _getAclSupportAvailable(self):
        """
        Returns True if the current connection has got ACL support.
        
        @return: ACL support (True / False)
        @rtype: C{bool}
        """
        options = self.getSpecificOption(Constants.HTTP_HEADER_DAV)
        if options.find(Constants.HTTP_HEADER_OPTION_ACL) >= 0:
            return True
        else:
            return False
        
    aclSupportAvailable = property(_getAclSupportAvailable)
        
    def _getDaslBasicsearchSupportAvailable(self):
        """
        Returns True if the current connection supports DASL basic search.
        
        @return: DASL basic search support (True / False)
        @rtype: C{bool}
        """
        options = self.getSpecificOption(Constants.HTTP_HEADER_DASL)
        if not options or \
           not options.find(Constants.HTTP_HEADER_OPTION_DAV_BASIC_SEARCH) >= 0:
            return False
        else:
            return True
    
    daslBasicsearchSupportAvailable = property(_getDaslBasicsearchSupportAvailable)
    
    def isConnectedToCatacombServer(self):
        """
        Returns True if connected to a Catacomb WebDav server.
        
        @return: if connected to Catacomb Webdav server (True / False)
        @rtype: C{bool}
        """
        if not self.connection.serverTypeChecked:
            options = self.getSpecificOption(Constants.HTTP_HEADER_SERVER)
            if options.find(Constants.HTTP_HEADER_SERVER_TAMINO) >= 0:
                self.connection.isConnectedToCatacomb = False
            else:
                self.connection.isConnectedToCatacomb = True
            self.connection.serverTypeChecked = True
        return self.connection.isConnectedToCatacomb
        
    def getSpecificOption(self, option):
        """
        Returns specified WebDav options.
        @param option: name of the option
        
        @return: String containing the value of the option.
        @rtype: C{string}
        """
        options = ''
        try:
            options = self.options().get(option)
        except KeyError:
            return options
        return options  
        
    ### delegate some method invocations    
    def __getattr__(self, name):
        """
        Build-in method:
        Forwards unknow lookups (methods) to delegate object 'versionHandler'.
        
        @param name: name of unknown attribute
        """
        # delegate Delta-V methods
        return getattr(self.versionHandler, name)
    
    def copy(self, toUrl, infinity=True):
        """
        Copies this resource.
        
        @param toUrl: target URI path
        @param infinity: Flag that indicates that the complete content of collection is copied. (default)  
        @type depth: C{boolean}
        """
        self.connection.logger.debug("Copy to " + repr(toUrl));
        _checkUrl(toUrl)
        if infinity:
            response = self.connection.copy(self.path, toUrl)
        else:
            response = self.connection.copy(self.path, toUrl, 0)
        if  response.status == Constants.CODE_MULTISTATUS and response.msr.errorCount > 0:
            raise WebdavError("Request failed: " + response.msr.reason, response.msr.code)        
        
    def delete(self, lockToken=None):
        """
        Deletes this resource.
        
        @param lockToken: String returned by last lock operation or null.
        @type  lockToken: L{LockToken}
        """
        assert lockToken == None or isinstance(lockToken, LockToken), \
                "Invalid lockToken argument %s" % type(lockToken)
        header = {}
        if lockToken:
            header = lockToken.toHeader()
        response = self.connection.delete(self.path, header)
        if  response.status == Constants.CODE_MULTISTATUS and response.msr.errorCount > 0:
            raise WebdavError("Request failed: " + response.msr.reason, response.msr.code)        

    def move(self, toUrl):
        """
        Moves this resource to the given path or renames it.
        
        @param toUrl: new (URI) path
        """
        self.connection.logger.debug("Move to " + repr(toUrl));
        _checkUrl(toUrl)
        response = self.connection.move(self.path, toUrl)
        if  response.status == Constants.CODE_MULTISTATUS and response.msr.errorCount > 0:
            raise WebdavError("Request failed: " + response.msr.reason, response.msr.code)        

 
    def lock(self, owner):
        """
        Locks this resource for exclusive write access. This means that for succeeding
        write operations the returned lock token has to be passed.
        If the methode does not throw an exception the lock has been granted.
        
        @param owner: describes the lock holder
        @return: lock token string (automatically generated)
        @rtype: L{LockToken}
        """
        response = self.connection.lock(self.path, owner)
        if  response.status == Constants.CODE_MULTISTATUS and response.msr.errorCount > 0:
            raise WebdavError("Request failed: " + response.msr.reason, response.msr.code)        
        return LockToken(self.url, response.locktoken)

    def unlock(self, lockToken):
        """
        Removes the lock from this resource.
        
        @param lockToken: which has been return by the lock() methode
        @type  lockToken: L{LockToken}
        """
        self.connection.unlock(self.path, lockToken.token)


    def deleteContent(self, lockToken=None):
        """
        Delete binary data at permanent storage.
        
        @param lockToken: None or lock token from last lock request
        @type  lockToken: L{LockToken}
        """
        assert lockToken == None or isinstance(lockToken, LockToken), \
                "Invalid lockToken argument %s" % type(lockToken)
        header = {}
        if lockToken:
            header = lockToken.toHeader()
        self.connection.put(self.path, "", extra_hdrs=header)

    def uploadContent(self, content, lockToken=None):
        """
        Write binary data to permanent storage.
        
        @param content: containing binary data
        @param lockToken: None or lock token from last lock request
        @type  lockToken: L{LockToken}
        """
        assert not content or isinstance(content, types.UnicodeType) or\
                isinstance(content, types.StringType), "Content is not a string: " + content.__class__.__name__
        assert lockToken == None or isinstance(lockToken, LockToken), \
                "Invalid lockToken argument %s" % type(lockToken)
        header = {}
        if lockToken:
            header = lockToken.toHeader()
        response = None
        try: 
            response = self.connection.put(self.path, content, extra_hdrs=header)
        finally:
            if response:        
                self.connection.logger.debug(response.read())
                response.close()

    def uploadFile(self, newFile, lockToken=None):
        """
        Write binary data to permanent storage.
        
        @param newFile: File containing binary data.
        @param lockToken: None or lock token from last lock request
        @type  lockToken: L{LockToken}
        """
        assert isinstance(newFile, types.FileType), "Argument is no file: " + file.__class__.__name__
        assert lockToken == None or isinstance(lockToken, LockToken), \
                "Invalid lockToken argument %s" % type(lockToken)
        header = {}
        if lockToken:
            header = lockToken.toHeader()
        self.connection.putFile(self.path, newFile, header=header)

    def downloadContent(self):
        """
        Read binary data from permanent storage.
        """
        response = self.connection.get(self.path)
        # TODO: Other interface ? return self.connection.getfile()
        return response

    def downloadFile(self, localFileName):
        """
        Copy binary data from permanent storage to a local file.
        
        @param localFileName: file to write binary data to
        """
        localFile = open(localFileName, 'wb')
        remoteFile = self.downloadContent()
        _blockCopyFile(remoteFile, localFile, Connection.blockSize)
        remoteFile.close()
        localFile.close()

    def readProperties(self, *names):
        """
        Reads the given properties.
        
        @param names: a list of property names.
                      A property name is a (XmlNameSpace, propertyName) tuple.
        @return: a map from property names to DOM Element or String values.
        """
        assert names, "Property names are missing."
        body = createFindBody(names, self.defaultNamespace)
        response = self.connection.propfind(self.path, body, depth=0)
        properties = response.msr.values()[0]
        if  properties.errorCount > 0:
            raise WebdavError("Property is missing on '%s': %s" % (self.path, properties.reason), properties.code)
        return properties

    def readProperty(self, nameSpace, name):
        """
        Reads the given property.
        
        @param nameSpace: XML-namespace
        @type nameSpace: string
        @param name: A property name.
        @type name: string
        
        @return: a map from property names to DOM Element or String values.
        """
        results = self.readProperties((nameSpace, name))
        if  len(results) == 0:
            raise WebdavError("Property is missing: " + results.reason)
        return results.values()[0]

    def readAllProperties(self):
        """
        Reads all properties of this resource.
        
        @return: a map from property names to DOM Element or String values.
        """
        response = self.connection.allprops(self.path, depth=0)
        return response.msr.values()[0]

    def readAllPropertyNames(self):
        """
        Returns the names of all properties attached to this resource.
        
        @return: List of property names
        """
        response = self.connection.propnames(self.path, depth=0)
        return response.msr.values()[0]

    def readStandardProperties(self):
        """
        Read all WebDAV live properties.
        
        @return: A L{LiveProperties} instance which contains a getter method for each live property.
        """
        body = createFindBody(LiveProperties.NAMES, Constants.NS_DAV)
        response = self.connection.propfind(self.path, body, depth=0)
        properties = response.msr.values()[0]
        return LiveProperties(properties)

    def writeProperties(self, properties, lockToken=None):
        """
        Sets or updates the given properties.
        
        @param lockToken: if the resource has been locked this is the lock token.
        @type  lockToken: L{LockToken}
        @param properties: a map from property names to a String or
                           DOM element value for each property to add or update.
        """
        assert isinstance(properties, types.DictType)
        assert lockToken == None or isinstance(lockToken, LockToken), \
                "Invalid lockToken argument %s" % type(lockToken)
        header = {}
        if lockToken:
            header = lockToken.toHeader()
        body = createUpdateBody(properties, self.defaultNamespace)
        response = self.connection.proppatch(self.path, body, header)
        if  response.msr.errorCount > 0:
            raise WebdavError("Request failed: " + response.msr.reason, response.msr.code)

    def deleteProperties(self, lockToken=None, *names):
        """
        Removes the given properties from this resource.
        
        @param lockToken: if the resource has been locked this is the lock token.
        @type  lockToken: L{LockToken}
        @param names: a collection of property names.
               A property name is a (XmlNameSpace, propertyName) tuple.
        """
        assert lockToken == None or isinstance(lockToken, LockToken), \
                "Invalid lockToken argument %s" % type(lockToken)
        header = {}
        if lockToken:
            header = lockToken.toHeader()
        body = createDeleteBody(names, self.defaultNamespace)
        response = self.connection.proppatch(self.path, body, header)
        if  response.msr.errorCount > 0:
            raise WebdavError("Request failed: " + response.msr.reason, response.msr.code)

    # ACP extension
    def setAcl(self, acl, lockToken=None):
        """
        Sets ACEs in the non-inherited and non-protected ACL or the resource.
        This is the implementation of the ACL method of the WebDAV ACP.
        
        @param acl: ACL to be set on resource as ACL object.
        @param lockToken: If the resource has been locked this is the lock token (defaults to None).
        @type  lockToken: L{LockToken}
        """
        assert lockToken == None or isinstance(lockToken, LockToken), \
                "Invalid lockToken argument %s" % type(lockToken)
        headers = {}
        if lockToken:
            headers = lockToken.toHeader()
        headers['Content-Type'] = XML_CONTENT_TYPE
        body                    = acl.toXML()
        response = self.connection._request('ACL', self.path, body, headers)
        return response
        ## TODO: parse DAV:error response

    def getAcl(self):
        """
        Returns this resource's ACL in an ACL instance.
        
        @return: Access Control List.
        @rtype: L{ACL<webdav.acp.Acl.ACL>}
        """
        xmlAcl = self.readProperty(Constants.NS_DAV, Constants.TAG_ACL)
        return ACL(xmlAcl)

    def getCurrentUserPrivileges(self):
        """
        Returns a tuple of the current user privileges.
        
        @return: list of Privilege instances
        @rtype: list of L{Privilege<webdav.acp.Privilege.Privilege>}
        """
        privileges = self.readProperty(Constants.NS_DAV, Constants.PROP_CURRENT_USER_PRIVILEGE_SET)
        result = []
        for child in privileges.children:
            result.append(Privilege(domroot=child))
        return result
    
    def getPrincipalCollections(self):
        """
        Returns a list principal collection URLs.
        
        @return: list of principal collection URLs
        @rtype: C{list} of C{unicode} elements
        """
        webdavQueryResult = self.readProperty(Constants.NS_DAV, Constants.PROP_PRINCIPAL_COLLECTION_SET)
        principalCollectionList = []
        for child in webdavQueryResult.children:
            principalCollectionList.append(child.first_cdata)            
        return principalCollectionList
    
    def getOwnerUrl(self):
        """ Explicitly retireve the Url of the owner. """
        
        result = self.readProperty(Constants.NS_DAV, Constants.PROP_OWNER)
        if result and len(result.children):
            return result.children[0].textof()
        return None

class CollectionStorer(ResourceStorer):
    """
    This class provides client access to a WebDAV collection resource identified by an URI.
    This class does not cache resource data. This has to be performed by its clients.
    
    @author: Roland Betz
    """
        
    def __init__(self, url, connection=None, validateResourceNames=True):
        """
        Creates a CollectionStorer instance for a URL and an optional Connection object.
        User must invoke validate() after constuction to check the resource on the server.
        
        @see: L{webdav.WebdavClient.ResourceStorer.__init__}
        @param url: unique resource location for this storer
        @param connection: this optional parameter contains a Connection object for the host part
            of the given URL. Passing a connection saves memory by sharing this connection.    
        """
        if  url[-1] != '/':     # Collection URL must end with slash
            url += '/'
        ResourceStorer.__init__(self, url, connection, validateResourceNames)
    
    def getResourceStorer(self, name):
        """
        Return a ResourceStorer instance for a child resource (member) of this Collection.
        
        @param name: leaf name of child resource
        @return: L{ResourceStorer} instance
        """
        assert isinstance(name, types.StringType) or isinstance(name, types.UnicodeType)
        return ResourceStorer(self.url + name, self.connection, self.validateResourceNames)
             
    def validate(self):
        """
        Check whether this URL contains a WebDAV collection.
        Uses the WebDAV OPTION method.
        
        @raise WebdavError: L{WebdavError} if URL does not contain a WebDAV collection resource.
        """
        super(CollectionStorer, self).validate()
        isCollection = self.readProperty(Constants.NS_DAV, Constants.PROP_RESOURCE_TYPE)
        if not (isCollection and isCollection.children):
            raise WebdavError("Not a collection URL.", 0)        
        
    def addCollection(self, name, lockToken=None):
        """
        Make a new WebDAV collection resource within this collection.
        
        @param name: of the new collection
        @param lockToken: None or token returned by last lock operation
        @type  lockToken: L{LockToken}
        """
        assert isinstance(name, types.StringType) or isinstance(name, types.UnicodeType)
        assert lockToken == None or isinstance(lockToken, LockToken), \
                "Invalid lockToken argument %s" % type(lockToken)
        header = {}
        if lockToken:
            header = lockToken.toHeader()
        if self.validateResourceNames:
            validateResourceName(name)
        if  name[-1] != '/':     # Collection URL must end with slash
            name += '/'
        self.connection.mkcol(self.path + name, header)
        return CollectionStorer(self.url + name, self.connection, self.validateResourceNames) 
        
    def addResource(self, name, content=None, properties=None, lockToken=None):
        """
        Create a new empty WebDAV resource contained in this collection with the given
        properties.
        
        @param name: leaf name of the new resource    
        @param content: None or initial binary content of resource
        @param properties: name/value-map containing properties
        @param lockToken: None or token returned by last lock operation
        @type  lockToken: L{LockToken}
        """
        assert isinstance(name, types.StringType) or isinstance(name, types.UnicodeType)
        assert lockToken == None or isinstance(lockToken, LockToken), \
                "Invalid lockToken argument %s" % type(lockToken)
        if self.validateResourceNames:
            validateResourceName(name) ## check for invalid characters        
        resource_ = ResourceStorer(self.url + name, self.connection, self.validateResourceNames)
        resource_.uploadContent(content, lockToken)
        if properties:
            resource_.writeProperties(properties, lockToken)
        return resource_
        
    def deleteResource(self, name, lockToken=None):
        """
        Delete a collection which is contained within this collection
        
        @param name: leaf name of a contained collection resource
        @param lockToken: None or token returned by last lock operation
        @type  lockToken: L{LockToken}
        """
        assert isinstance(name, types.StringType) or isinstance(name, types.UnicodeType)
        assert lockToken == None or isinstance(lockToken, LockToken), \
                "Invalid lockToken argument %s" % type(lockToken)
        header = {}
        if lockToken:
            header = lockToken.toHeader()
        if self.validateResourceNames:
            validateResourceName(name)
        response = self.connection.delete(self.path + name, header)
        if  response.status == Constants.CODE_MULTISTATUS and response.msr.errorCount > 0:
            raise WebdavError("Request failed: %s" % response.msr.reason, response.msr.code)        
        
    def lockAll(self, owner):
        """
        Locks this collection resource for exclusive write access. This means that for 
        succeeding write operations the returned lock token has to be passed.
        The operation is applied recursively to all contained resources.
        If the methode does not throw an exception then the lock has been granted.
        
        @param owner: describes the lock holder
        @return: Lock token string (automatically generated).
        @rtype: L{LockToken}
        """
        assert isinstance(owner, types.StringType) or isinstance(owner, types.UnicodeType)
        response = self.connection.lock(self.path, owner, depth=Constants.HTTP_HEADER_DEPTH_INFINITY)
        return LockToken(self.url, response.locktoken)
        
    def listResources(self):
        """
        Describe all members within this collection.
        
        @return: map from URI to a L{LiveProperties} instance containing the WebDAV
                 live attributes of the contained resource
        """
        # *LiveProperties.NAMES denotes the list of all live properties as an
        # argument to the method call.
        response = self.connection.getprops(self.path,
                                            depth=1,
                                            ns=Constants.NS_DAV,
                                            *LiveProperties.NAMES)
        result = {}
        for path, properties in response.msr.items():
            if path == self.path:      # omit this collection resource
                continue
            ## some servers do not append a trailing slash to collection paths
            if self.path.endswith('/') and self.path[0:-1] == path:
                continue
            result[path] = LiveProperties(properties=properties)
        return result

    def getCollectionContents(self):
        """
        Return a list of the tuple (resources or collection) / properties)

        @return: a list of the tuple (resources or collection) / properties)
        @rtype: C{list} 
        """
        self.validate()
        collectionContents = []
        result = self.listResources()
        for url, properties_ in result.items():
            if not self.path == url:
                if properties_.getResourceType() == 'resource':
                    myWebDavStorer = ResourceStorer(url, self.connection, self.validateResourceNames)
                else:
                    myWebDavStorer = CollectionStorer(url, self.connection, self.validateResourceNames)
                collectionContents.append((myWebDavStorer, properties_))
        return collectionContents

    def findProperties(self, *names):
        """
        Retrieve given properties for this collection and all directly contained resources.
        
        @param names: a list of property names
        @return: a map from resource URI to a map from property name to value.
        """
        assert isinstance(names, types.ListType) or isinstance(names, types.TupleType), \
                "Argument name has type %s" % str(type(names))
        body = createFindBody(names, self.defaultNamespace)
        response = self.connection.propfind(self.path, body, depth=1)
        return response.msr
        
    def deepFindProperties(self, *names):
        """
        Retrieve given properties for this collection and all contained (nested) resources.
        
        Note:
        =====
          This operation can take a long time if used with recursive=true and is therefore
          disabled on some WebDAV servers.
        
        @param names: a list of property names
        @return: a map from resource URI to a map from property name to value.
        """
        assert isinstance(names, types.ListType.__class__) or isinstance(names, types.TupleType), \
                "Argument name has type %s" % str(type(names))
        body = createFindBody(names, self.defaultNamespace)
        response = self.connection.propfind(self.path, body, depth=Constants.HTTP_HEADER_DEPTH_INFINITY)
        return response.msr
        
    def findAllProperties(self):
        """
        Retrieve all properties for this collection and all directly contained resources.
        
        @return: a map from resource URI to a map from property name to value.
        """
        response = self.connection.allprops(self.path, depth=1)
        return response.msr
            
        
    # DASL extension
    def search(self, conditions, selects):
        """
        Search for contained resources which match the given search condition.
        
        @param conditions: tree of ConditionTerm instances representing a logical search term
        @param selects: list of property names to retrieve for the found resources
        """
        assert isinstance(conditions, ConditionTerm)
        headers = { 'Content-Type' : XML_CONTENT_TYPE, "depth": Constants.HTTP_HEADER_DEPTH_INFINITY}
        body = createSearchBody(selects, self.path, conditions)
        response = self.connection._request('SEARCH', self.path, body, headers)
        return response.msr
        

class LockToken(object):
    """
    This class provides help on handling WebDAV lock tokens.
    
    @author: Roland Betz
    """
    # restrict instance variables
    __slots__ = ('url', 'token')

    def __init__(self, url, token):
        assert isinstance(url, types.StringType) or isinstance(url, types.UnicodeType), \
            "Invalid url argument %s" % type(url)
        assert isinstance(token, types.StringType) or isinstance(token, types.UnicodeType), \
            "Invalid lockToken argument %s" % type(token)
        self.url = url
        self.token = token

    def value(self):
        """
        Descriptive string containing the lock token's URL and the token itself.
        
        @return: Descriptive lock token with URL.
        @rtype: C{string}
        """
        return "<" + self.url + "> (<" + self.token + ">)"

    def toHeader(self):
        """
        Header fragment for WebDAV request.
        
        @return: Dictionary containing an entry for the lock token query.
        @rtype: C{dictionary}
        """
        return {Constants.HTTP_HEADER_IF: self.value()}
    
    def __str__(self):
        return self.value()


def _blockCopyFile(source, dest, blockSize):
    """
    Copies a file in chunks of C{blockSize}.
    
    @param source: Source file.
    @type  source: FileIO buffer.
    @param dest: Destination file.
    @type  dest: FileIO buffer.
    @param blockSize: Size of block in bytes.
    @type  blockSize: C{int}
    """
    transferedBytes = 0
    block = source.read(blockSize)
    while len(block):
        dest.write(block)
        transferedBytes += len(block);
        block = source.read(blockSize)        

def _checkUrl(url):
    """
    Checks the given URL for validity.
    
    @param url: URL to check.
    @type  url: C{string}
    
    @raise ValueError: If the URL does not contain valid/usable content.
    """
    
    parts = urlsplit(url, allow_fragments=False)
    if len(parts[0]) == 0 or len(parts[1]) == 0 or len(parts[2]) == 0:
        raise ValueError("Invalid URL: " + repr(url))

# small test
# asks for WebDAV colection (and optionaly username and password) and lists the content of the collection.
if __name__ == "__main__":
    import sys
 
    authFailures = 0
    while authFailures < 2:
        try:
        
            for resource, properties in webdavConnection.getCollectionContents():
                try: 
                    print resource.path.encode(sys.getfilesystemencoding())
                    print unicode(properties).encode(sys.getfilesystemencoding())
                except UnicodeEncodeError:
                    print("Cannot encode resource path or properties.")
            
            break # break out of the authorization failure counter
        except AuthorizationError as e:
            username = raw_input("Username:").strip()
            password = raw_input("Password:").strip()
                        
            if e.authType == "Basic":
                webdavConnection.connection.addBasicAuthorization(username, password)
            elif e.authType == "Digest":
                info = parseDigestAuthInfo(e.authInfo)
                
                webdavConnection.connection.addDigestAuthorization(username, password, realm=info["realm"], qop=info["qop"], nonce=info["nonce"])
            else:
                raise
                        
            authFailures += 1
        print("\n")
