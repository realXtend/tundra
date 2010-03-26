# pylint: disable-msg=W0612,W0142
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
The WebDAV client module forwards Delta-V related method invocations to
the following VersionHandler class.
"""

__version__ = '$Revision: 13 $'[11:-2]


import types

from webdav import Constants
from davlib import XML_CONTENT_TYPE, XML_DOC_HEADER


class VersionHandler(object):
    """
    Implements a client interface for WebDAV Delta-V methods
    For the Delta-V see RFC 3253 at http://www.ietf.org/rfc/rfc3253.txt
    """
    
    # restrict instance variables
    __slots__ = ('path', 'connection')
 
    
    def __init__(self, connection, path):
        """
        Construct a VersionHandler with a URL path and a WebDAV connection.
        This constructor must not be called outside class ResourceStorer.
        
        @param connection: L{webdav.Connection} instance
        @param path: resource's path part of URL
        """
        #assert isinstance(connection, Connection), \
        #    "Class of connection is %s." % connection.__class__.__name__
        self.connection = connection
        self.path = path
 
 
    def activateVersionControl(self):
        """
        Turns version control on for this resource.
        The resource becomes a version controlled resource (VCR)
        """
        response = self.connection._request(Constants.METHOD_VERSION_CONTROL, self.path, None, {})
        # set auto-versioning to DAV:locked-checkout
        ## parse response body in case of an error

    def uncheckout(self, lockToken=None):
        """
        Undos a previous check-out operation on this VCR.
        The VCR is reverted to the state before the checkout/lock operation.
        Beware: Property or content changes will be lost !
        A (optional) lock has to be removed seperatedly.
        
        @param lockToken: returned by a preceeding lock() method invocation or None
        """
        headers = {}
        if lockToken:
            headers = lockToken.toHeader() 
        response = self.connection._request(Constants.METHOD_UNCHECKOUT, self.path, None, headers)
        ## parse response body in case of an error
    
    def listAllVersions(self):
        """
        List version history.
        
        @return: List of versions for this VCR. Each version entry is a tuple adhering
            to the format (URL-path, name, creator, tuple of successor URL-paths).
            If there are no branches then there is at most one successor within the tuple. 
        """
        # implementation is similar to the propfind method
        headers                 = {}
        headers['Content-Type'] = XML_CONTENT_TYPE
        body = _createReportVersionTreeBody()
        response = self.connection._request(Constants.METHOD_REPORT, self.path, body, headers)
        # response is multi-status
        result = []
        for path, properties in response.msr.items():           
            # parse the successor-set value from XML into alist
            result.append( (path, str(properties[Constants.PROP_VERSION_NAME]), \
                            str(properties[Constants.PROP_CREATOR]), \
                            _extractSuccessorList(properties[Constants.PROP_SUCCESSOR_SET])) )
        ## TODO: sort for path and produce list
        result.sort()
        return result

    # warning: not tested yet
    def readVersionProperties(self):
        """
        Provide version related information on this VCR.
        This include a reference to the latest version resource,
        check-out state information and a comment.
        
        @return: map of version properties with values.
        """
        versionProperties = (Constants.PROP_CHECKEDIN, Constants.PROP_CHECKEDOUT, Constants.PROP_COMMENT)
        return self.connection.readProperties(*versionProperties)
    

    def revertToVersion(self, oldVersion):
        """
        Revert this VCR to the given version.
        Beware: All versions succeeding the given version are made unavailable.
        
        @param oldVersion: URL-path of a previous version of this VCR.
        """
        ## send an update request
        assert isinstance(oldVersion, types.StringType) or isinstance(oldVersion, types.UnicodeType)
        response = self.connection._request(Constants.METHOD_UPDATE, self.path,
                        _createUpdateBody(oldVersion), {})
        return response


    # the following is not needed when using auto-versioning
    
    # warning: not tested yet
    def checkout(self):
        """
        Marks resource as checked-out
        This is usually followed by a GET (download) operation.
        """
        response = self.connection._request(Constants.METHOD_CHECKOUT, self.path, None, {})
        ## parse response body in case of an error

    # warning: not tested yet
    def checkin(self):
        """
        Creates a new version from the VCR's content.
        This opeartion is usually preceeded by one or more write operations.
        """
        response = self.connection._request(Constants.METHOD_CHECKIN, self.path, None, {})
        versionUrl = response.getheader('Location')
        return versionUrl
        ## parse response body in case of an error




# Helper functions
def _createReportVersionTreeBody():
    """
    TBD
    
    @return: ...
    @rtype: string
    """
    versions = 'D:' + Constants.TAG_VERSION_TREE
    prop = 'D:' + Constants.TAG_PROP
    nameList = [Constants.PROP_SUCCESSOR_SET, Constants.PROP_VERSION_NAME, Constants.PROP_CREATOR]
    return XML_DOC_HEADER + \
        '<%s xmlns:D="DAV:"><%s>' % (versions, prop) + \
        reduce(lambda xml, name: xml + "<D:%s/>" % name[1], [''] + nameList) + \
        '</%s></%s>' % (prop, versions)

def _createUpdateBody(path):
    """
    TBD
    
    @return: ...
    @rtype: string
    """
    update = 'D:' + Constants.TAG_UPDATE
    version = 'D:' + Constants.TAG_VERSION
    href = 'D:' + Constants.TAG_HREF
    #PROP = 'D:' + TAG_PROP
    return XML_DOC_HEADER + \
        '<%s xmlns:D="DAV:"><%s><%s>' % (update, version, href) + \
        path + \
        '</%s></%s></%s>' % (href, version, update)

def _extractSuccessorList(element):
    """
    TBD
    
    @return: ...
    @rtype: tuple of strings
    """
    result = []
    for href in element.children:
        result.append(href.textof())
    return tuple(result)
