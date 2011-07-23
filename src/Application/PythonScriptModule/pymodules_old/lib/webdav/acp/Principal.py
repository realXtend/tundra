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
Handling of principals for ACEs according to WebDAV ACP specification.
"""


from webdav import Constants
from webdav.Connection import WebdavError


__version__ = "$LastChangedRevision: 13 $"


class Principal(object):
    """
    This class provides functionality for handling
    principals according to the WebDAV ACP.
     
    @ivar displayname:  Name of the principal for output
    @type displayname:  C{string}
    @ivar principalURL: URL under which the principal can be referenced on the server.
    @type principalURL: C{string}
    @ivar property:     Information on type of a pseudo/jproperty principal, e. g. 
        DAV:owner, DAV:authenticated, etc.
    @type property:     C{string}
    
    @cvar _TAG_LIST_PRINCIPALS: List of allowed XML tags within a principal declaration.
    @type _TAG_LIST_PRINCIPALS: C{tuple} of C{string}s
    @cvar _TAG_LIST_STATUS:     List of XML tags for the status of a pseudo principal.
    @type _TAG_LIST_STATUS:     C{tuple} of C{string}s
    """
    
    # some local constants for this class to make things easier/more readable:
    _TAG_LIST_PRINCIPALS = (Constants.TAG_HREF,     # directly by URL
                            Constants.TAG_ALL, Constants.TAG_AUTHENTICATED, Constants.TAG_UNAUTHENTICATED, 
                                                    # by log-in status
                            Constants.TAG_PROPERTY, # for property info, e. g. 'owner'
                            Constants.TAG_SELF,     # only if the resource is the principal itself
                            Constants.TAG_PROP)     # contains property info like 'displayname'
    _TAG_LIST_STATUS     = (Constants.TAG_ALL, Constants.TAG_AUTHENTICATED, Constants.TAG_UNAUTHENTICATED)
    
    # restrict instance variables
    __slots__ = ('displayname', 'principalURL', 'property')
    
    def __init__(self, domroot=None, displayname=None, principalURL=None):
        """
        Constructor should be called with either no parameters (create blank Principal),
        one parameter (a DOM tree), or two parameters (displayname and URL or property tag).
        
        @param domroot:      A DOM tree (default: None).
        @type  domroot:      L{webdav.WebdavResponse.Element} object
        @param displayname:  The display name of a principal (default: None).
        @type  displayname:  C{string}
        @param principalURL: The URL representing a principal (default: None).
        @type  principalURL: C{string}
        
        @raise WebdavError: When non-valid parameters or sets of parameters are 
            passed a L{WebdavError} is raised.
        """
        self.displayname  = None
        self.principalURL = None
        self.property     = None

        if domroot:
            for child in domroot.children:
                if child.ns == Constants.NS_DAV and (child.name in self._TAG_LIST_PRINCIPALS):
                    if child.name == Constants.TAG_PROP:
                        self.displayname = \
                            child.find(Constants.PROP_DISPLAY_NAME, Constants.NS_DAV)
                    elif child.name == Constants.TAG_HREF:
                        self.principalURL = child.textof()
                        if self.principalURL and self.property in self._TAG_LIST_STATUS:
                            raise WebdavError('Principal cannot contain a URL and "%s"' % (self.property))
                    elif child.name == Constants.TAG_PROPERTY:
                        if child.count() == 1:
                            if self.property:
                                raise WebdavError('Property for principal has already been set: old "%s", new "%s"' \
                                    % (self.property, child.pop().name))
                            elif self.principalURL:
                                raise WebdavError('Principal cannot contain a URL and "%s"' % (self.property))
                            else:
                                self.property = child.pop().name
                        else:
                            raise WebdavError("There should be only one value in the property for a principal, we have: %s" \
                                % child.name)
                    else:
                        if self.property:
                            raise WebdavError('Property for principal has already been set: old "%s", new "%s"' \
                                % (self.property, child.name))
                        else:
                            self.property = child.name
                        if self.principalURL and self.property in self._TAG_LIST_STATUS:
                            raise WebdavError('Principal cannot contain a URL and "%s"' % (self.property))
                else: # This shouldn't happen, something's wrong with the DOM tree
                    raise WebdavError('Non-valid tag in principal DOM tree for constructor: %s' % child.name)
        elif displayname == None or principalURL == None:
            if displayname:
                self.displayname  = displayname
            if principalURL:
                self.principalURL = principalURL
        else:
            # This shouldn't happen, someone screwed up with the params ...
            raise WebdavError('Non-valid parameters handed to Principal constructor.')

    def __cmp__(self, other):
        if not isinstance(other, Principal):
            return 1
        if self.displayname == other.displayname \
                and self.principalURL == other.principalURL \
                and self.property == other.property:
            return 0
        else:
            return 1

    def __repr__(self):
        return '<class Principal: displayname: "%s", principalURL: "%s", property: "%s">' \
            % (self.displayname, self.principalURL, self.property)

    def copy(self, other):
        """Copy Principal object.
        
        @param other: Another principal to copy.
        @type  other: L{Principal} object
        
        @raise WebdavError: When an object that is not a L{Principal} is passed 
            a L{WebdavError} is raised.
        """
        if not isinstance(other, Principal):
            raise WebdavError('Non-Principal object passed to copy method: ' % other.__class__)
        self.displayname  = other.displayname
        self.principalURL = other.principalURL
        self.property     = other.property

    def isValid(self):
        """
        Checks whether necessarry props for principal are set.
        
        @return: Validity of principal.
        @rtype:  C{bool}
        """
        return (self.displayname and
                (self.principalURL or self.property) and
                not (self.principalURL and self.property))
        
    def toXML(self, invert=False, displayname=False, defaultNameSpace=None):
        """Returns string of Principal content in valid XML as described in WebDAV ACP.
        
        @param defaultNameSpace: Name space (default: None).
        @type  defaultNameSpace: C(string)
        @param invert:           True if principal should be inverted (default: False).
        @type  invert:           C{bool}
        @param displayname:      True if displayname should be in output (default: False).
        @type  displayname:      C{bool}
        """
        # this check is needed for setting principals only:
        # assert self.isValid(), "principal is not initialized or does not contain valid content!"
        
        PRINCIPAL = 'D:' + Constants.TAG_PRINCIPAL
        res = ''
        if self.principalURL:
            res += '<D:%s>%s</D:%s>' % (Constants.TAG_HREF, self.principalURL, Constants.TAG_HREF)
        elif self.property in self._TAG_LIST_STATUS \
                or self.property == Constants.TAG_SELF:
            res += '<D:%s/>' % (self.property)
        elif self.property:
            res += '<D:%s><D:%s/></D:%s>' \
                % (Constants.TAG_PROPERTY, self.property, Constants.TAG_PROPERTY)
        if self.displayname and displayname:
            res += '<D:%s><D:%s>%s</D:%s></D:%s>' \
                % (Constants.TAG_PROP, Constants.PROP_DISPLAY_NAME,
                    self.displayname,
                    Constants.PROP_DISPLAY_NAME, Constants.TAG_PROP)
        if invert:
            res = '<D:invert>%s</D:invert>' % (res)
        return '<%s>%s</%s>' % (PRINCIPAL, res, PRINCIPAL)
