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
ACE object handling according to WebDAV ACP specification.
"""


from webdav.acp.Principal import Principal
from webdav.acp.GrantDeny import GrantDeny
from webdav import Constants
from webdav.Connection import WebdavError


__version__ = "$LastChangedRevision: 13 $"


class ACE(object):
    """
    This class provides functionality for handling ACEs
    
    @ivar principal:   A principal (user or group)
    @type principal:   L{Principal} object
    @ivar invert:      Flag indicating whether ACE should invert the principal.
    @type invert:      C{bool}
    @ivar grantDenies: Grant or deny clauses for privileges
    @type grantDenies: C{list} of L{GrantDeny} objects
    @ivar protected:   Flag indicating whether ACE is protected.
    @type protected:   C{bool}
    @ivar inherited:   URL indicating the source from where the ACE is inherited.
    @type inherited:   C{string}
    """
    
    # restrict instance variables
    __slots__ = ('principal', 'invert', 'grantDenies', 'protected', 'inherited')
    
    def __init__(self, domroot=None, principal=None, grantDenies=None):
        """
        Constructor should be called with either no parameters (create blank ACE),
        one parameter (a DOM tree or principal), or two parameters (principal and 
        sequence of GrantDenies).
        
        @param domroot:     A DOM tree (default: None).
        @type  domroot:     L{webdav.WebdavResponse.Element} object
        @param principal:   A principal (user or group), (default: None).
        @type  principal:   L{Principal} object
        @param grantDenies: Grant and deny clauses for privileges (default: None).
        @type  grantDenies: sequence of L{GrantDeny} objects
        
        @raise WebdavError: When non-valid parameters are passed a L{WebdavError} is raised.
        """
        self.principal   = Principal()
        self.protected   = None
        self.inherited   = None
        self.invert      = None
        self.grantDenies = []

        if domroot:
            self.principal   = Principal(domroot=domroot.find(Constants.TAG_PRINCIPAL, Constants.NS_DAV))
            self.inherited   = domroot.find(Constants.TAG_INHERITED, Constants.NS_DAV)
            if self.inherited:
                self.inherited = self.inherited.children[0].textof()
            if domroot.find(Constants.TAG_PROTECTED, Constants.NS_DAV):
                self.protected = 1
            for child in domroot.children:
                if child.ns == Constants.NS_DAV \
                        and (child.name == Constants.TAG_GRANT or child.name == Constants.TAG_DENY):
                    self.grantDenies.append(GrantDeny(domroot=child))
        elif isinstance(principal, Principal):
            newPrincipal = Principal()
            newPrincipal.copy(principal)
            self.principal = newPrincipal
            if (isinstance(grantDenies, list) or isinstance(grantDenies, tuple)):
                self.addGrantDenies(grantDenies)
        elif domroot == None and grantDenies == None:
            # no param ==> blank ACE
            pass
        else:
            # This shouldn't happen, someone screwed up with the params ...
            raise WebdavError('non-valid parameters handed to ACE constructor')

    def __cmp__(self, other):
        if not isinstance(other, ACE):
            return 1
        if self.principal == other.principal \
                and self.invert == other.invert \
                and self.protected == other.protected \
                and self.inherited == other.inherited:
            equal = 1
            for grantDeny in self.grantDenies:
                inList = 0
                for otherGrantDeny in other.grantDenies:
                    if grantDeny == otherGrantDeny:
                        inList = 1
                if inList == 0:
                    equal = 0
            return not equal
        else:
            return 1

    def __repr__(self):
        repr = '<class ACE: '
        if self.invert:
            repr += 'inverted principal, ' % (self.invert)
        if self.principal:
            repr += 'principal: %s, ' % (self.principal)
        if self.protected:
            repr += 'protected, '
        if self.inherited:
            repr += 'inherited href: %s, ' % (self.inherited)
        first = 1
        repr += 'grantDenies: ['
        for grantDeny in self.grantDenies:
            if first:
                repr += '%s' % grantDeny
                first = 0
            else:
                repr += ', %s' % grantDeny
        return '%s]>' % (repr)

    def copy(self, other):
        '''Copy an ACE object.
        
        @param other: Another ACE to copy.
        @type  other: L{ACE} object
        
        @raise WebdavError: When an object that is not an L{ACE} is passed 
            a L{WebdavError} is raised.
        '''
        if not isinstance(other, ACE):
            raise WebdavError('Non-ACE object passed to copy method: %s.' % other.__class__)
        self.invert    = other.invert
        self.protected = other.protected
        self.inherited = other.inherited
        self.principal = Principal()
        if other.principal:
            self.principal.copy(other.principal)
        if other.grantDenies:
            self.addGrantDenies(other.grantDenies)

    def isValid(self):
        """
        Returns true/false (1/0) whether necessarry props 
        principal and grantDenies are set and whether the ACE contains one 
        grant or deny clauses.
        
        @return: Validity of ACE.
        @rtype:  C{bool}
        """
        return self.principal and len(self.grantDenies) == 1

    def isGrant(self):
        '''
        Returns true/false (1/0) if ACE contains only grant clauses.
        
        @return: Value whether the ACE is of grant type.
        @rtype:  C{bool}
        '''
        if self.isMixed() or len(self.grantDenies) < 1:
            return 0
        else:
            return self.grantDenies[0].isGrant()

    def isDeny(self):
        '''
        Returns true/false (1/0) if ACE contains only deny clauses.
        
        @return: Value whether the ACE is of deny type.
        @rtype:  C{bool}
        '''
        if self.isMixed() or len(self.grantDenies) < 1:
            return 0
        else:
            return self.grantDenies[0].isDeny()

    def isMixed(self):
        '''
        Returns true/false (1/0) if ACE contains both types (grant and deny) of clauses.
        
        @return: Value whether the ACE is of mixed (grant and deny) type.
        @rtype:  C{bool}
        '''
        mixed = 0
        if len(self.grantDenies):
            first = self.grantDenies[0].grantDeny
            for grantDeny in self.grantDenies:
                if grantDeny.grantDeny != first:
                    mixed = 1
        return mixed

    def toXML(self, defaultNameSpace=None):
        """
        Returns ACE content as a string of valid XML as described in WebDAV ACP.
        
        @param defaultNameSpace: Name space (default: None).
        @type  defaultNameSpace: C(string)
        """
        assert self.isValid(), "ACE is not initialized or does not contain valid content!"
        
        ACE = 'D:' + Constants.TAG_ACE
        res = self.principal.toXML(self.invert)
        for grantDeny in self.grantDenies:
            res += grantDeny.toXML()
        if self.protected:
            res += '<D:protected/>'
        if self.inherited:
            res += '<D:inherited><D:href>%s</D:href></D:inherited>' % (self.inherited)
        return '<%s>%s</%s>' % (ACE, res, ACE)

    def setPrincipal(self, principal):
        '''
        Sets the passed principal on the ACE.
        
        @param principal: A principal.
        @type  principal: L{Principal} object
        '''
        self.principal = Principal()
        self.principal.copy(principal)

    def setInherited(self, href):
        '''
        Sets the passed URL on the ACE to denote from where it is inherited.
        
        @param href: A URL.
        @type  href: C{string}
        '''
        self.inherited = href

    def addGrantDeny(self, grantDeny):
        '''
        Adds the passed GrantDeny object to list if it's not in it, yet.
        
        @param grantDeny: A grant or deny clause.
        @type  grantDeny: L{GrantDeny} object
        '''
        # only add it if it's not in the list, yet ...
        inList = 0
        for element in self.grantDenies:
            if element == grantDeny:
                inList = 1
        if not inList:
            newGrantDeny = GrantDeny()
            newGrantDeny.copy(grantDeny)
            self.grantDenies.append(newGrantDeny)

    def addGrantDenies(self, grantDenies):
        '''Adds the list of passed grant/deny objects to list.
        
        @param grantDenies: Grant or deny clauses.
        @type  grantDenies: sequence of L{GrantDeny} objects
        '''
        map(lambda grantDeny: self.addGrantDeny(grantDeny), grantDenies)

    def delGrantDeny(self, grantDeny):
        '''Deletes the passed GrantDeny object from list.
        
        @param grantDeny: A grant or deny clause.
        @type  grantDeny: L{GrantDeny} object
        
        @raise WebdavError: A L{WebdavError} is raised if the clause to be 
            deleted is not present.
        '''
        # only add it if it's not in the list, yet ...
        count = 0
        index = 0
        for element in self.grantDenies:
            count += 1
            if element == grantDeny:
                index = count
        if index:
            self.grantDenies.pop(index - 1)
        else:
            raise WebdavError('GrantDeny to be deleted not in list: %s.' % grantDeny)

    def delGrantDenies(self, grantDenies):
        '''Deletes the list of passed grant/deny objects from list.
        
        @param grantDenies: Grant or deny clauses.
        @type  grantDenies: sequence of L{GrantDeny} objects
        '''
        map(lambda grantDeny: self.delGrantDeny(grantDeny), grantDenies)
