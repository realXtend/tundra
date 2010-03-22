# pylint: disable-msg=W0622
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
ACL object handling according to WebDAV ACP specification.
"""


from webdav.acp.Ace import ACE
from webdav import Constants
from webdav.Connection import WebdavError
from davlib import XML_DOC_HEADER


__version__ = "$LastChangedRevision: 13 $"


class ACL(object):
    """
    This class provides access to Access Control List funcionality
    as specified in the WebDAV ACP.
    
    @ivar aces:          ACEs in ACL
    @type aces:          C{list} of L{ACE} objects
    @ivar withInherited: Flag indicating whether ACL contains inherited ACEs.
    @type withInherited: C{bool}
    """

    # restrict instance variables
    __slots__ = ('aces', 'withInherited')
    
    def __init__(self, domroot=None, aces=None):
        """
        Constructor should be called with either no parameters (create blank ACE),
        or one parameter (a DOM tree or ACE list).
        
        @param domroot: A DOM tree (default: None).
        @type  domroot: L{webdav.WebdavResponse.Element} object
        @param aces:    ACE objects (default: None)
        @type  aces:    C{list} of L{ACE} objects
        
        @raise WebdavError: When non-valid parameters are passed a L{WebdavError} is raised.
        """
        self.withInherited = None
        self.aces          = []
        
        if domroot:
            for child in domroot.children:
                if child.name == Constants.TAG_ACE and child.ns == Constants.NS_DAV:
                    self.addAce(ACE(child))
                else:
                    # This shouldn't happen, someone screwed up with the params ...
                    raise WebdavError('Non-ACE tag handed to ACL constructor: ' + child.ns + child.name)
        elif isinstance(aces, list) or isinstance(aces, tuple):
            self.addAces(aces)
        elif domroot == None and aces == None:
            # no param ==> blank object
            pass
        else:
            # This shouldn't happen, someone screwed up with the params ...
            raise WebdavError('non-valid parameters handed to ACL constructor')

    def __cmp__(self, other):
        if not isinstance(other, ACL):
            return 1
        if self.withInherited == other.withInherited:
            equal = 1
            for ace in self.aces:
                inList = 0
                for otherAce in other.aces:
                    if ace == otherAce:
                        inList = 1
                if inList == 0:
                    equal = 0
            return not equal
        else:
            return 1

    def __repr__(self):
        repr = '<class ACL: '
        if self.withInherited:
            repr += 'with inherited, '
        first = 1
        repr += 'aces: ['
        for ace in self.aces:
            if first:
                repr += '%s' % ace
                first = 0
            else:
                repr += ', %s' % ace
        return '%s]>' % (repr)

    def copy(self, other):
        '''Copy an ACL object.
        
        @param other: Another ACL to copy.
        @type  other: L{ACL} object
        
        @raise WebdavError: When an object that is not an L{ACL} is passed 
            a L{WebdavError} is raised.
        '''
        if not isinstance(other, ACL):
            raise WebdavError('Non-ACL object passed to copy method: %s' % other.__class__)
        self.withInherited = other.withInherited
        if other.aces:
            self.addAces(other.aces)

    def toXML(self):
        """
        Returns ACL content as a string of valid XML as described in WebDAV ACP.
        """
        aclTag = 'D:' + Constants.TAG_ACL
        return XML_DOC_HEADER +\
            '<' + aclTag + ' xmlns:D="DAV:">' + reduce(lambda xml, ace: xml + ace.toXML() + '\n', [''] + self.aces) +\
            '</' + aclTag + '>'

    def addAce(self, ace):
        '''
        Adds the passed ACE object to list if it's not in it, yet.
        
        @param ace: An ACE.
        @type  ace: L{ACE} object
        '''
        newAce = ACE()
        newAce.copy(ace)
        # only add it if it's not in the list, yet ...
        inList = 0
        for element in self.aces:
            if element == ace:
                inList = 1
        if not inList:
            self.aces.append(newAce)

    def addAces(self, aces):
        '''Adds the list of passed ACE objects to list.
        
        @param aces: ACEs
        @type  aces: sequence of L{ACE} objects
        '''
        for ace in aces:
            self.addAce(ace)

    def delAce(self, ace):
        '''Deletes the passed ACE object from list.
        
        @param ace: An ACE.
        @type  ace: L{ACE} object
        
        @raise WebdavError: When the ACE to be deleted is not within the ACL 
            a L{WebdavError} is raised.
        '''
        # find where it is and delete it ...
        count = 0
        index = 0
        for element in self.aces:
            count += 1
            if element == ace:
                index = count
        if index:
            self.aces.pop(index - 1)
        else:
            raise WebdavError('ACE to be deleted not in list: %s.' % ace)

    def delAces(self, aces):
        '''Deletes the list of passed ACE objects from list.
        
        @param aces: ACEs
        @type  aces: sequence of L{ACE} objects
        '''
        for ace in aces:
            self.delAce(ace)

    def delPrincipalsAces(self, principal):
        """
        Deletes all ACEs in ACL by given principal.
        
        @param principal: A principal.
        @type  principal: L{Principal} object
        """
        # find where it is and delete it ...
        index = 0
        while index < len(self.aces):
            if self.aces[index].principal.principalURL == principal.principalURL:
                self.aces.pop(index)
            else:
                index += 1

    def joinGrantDeny(self):
        """
        Returns a "refined" ACL of the ACL for ease of use in the UI. 
        The purpose is to post the user an ACE that can contain both, granted 
        and denied, privileges. So possible pairs of grant and deny ACEs are joined 
        to return them in one ACE. This resulting ACE then of course IS NOT valid 
        for setting ACLs anymore. They will have to be reconverted to yield valid 
        ACLs for the ACL method.
        
        @return: A (non-valid) ACL that contains both grant and deny clauses in an ACE.
        @rtype:  L{ACL} object
        """
        joinedAces = {}
        for ace in self.aces:
            if not ace.principal.principalURL is None:
                principalKey = ace.principal.principalURL
            elif not ace.principal.property is None:
                principalKey = ace.principal.property
            else:
                principalKey = None
            if ace.inherited:
                principalKey = ace.inherited + ":" + principalKey
            if principalKey in joinedAces:
                joinedAces[principalKey].addGrantDenies(ace.grantDenies)
            else:
                joinedAces[principalKey] = ACE()
                joinedAces[principalKey].copy(ace)
        newAcl = ACL()
        newAcl.addAces(joinedAces.values())
        return newAcl

    def splitGrantDeny(self):
        """
        Returns a "refined" ACL of the ACL for ease of use in the UI. 
        The purpose is to post the user an ACE that can contain both, granted 
        and denied, privileges. So possible joined grant and deny clauses in ACEs 
        splitted to return them in separate ACEs. This resulting ACE then is valid 
        for setting ACLs again. This method is to be seen in conjunction with the 
        method joinGrantDeny as it reverts its effect.
        
        @return: A valid ACL that contains only ACEs with either grant or deny clauses.
        @rtype:  L{ACL} object
        """
        acesGrant = {}
        acesDeny  = {}
        for ace in self.aces:
            for grantDeny in ace.grantDenies:
                if grantDeny.isGrant():
                    if ace.principal.principalURL in acesGrant:
                        ace.addGrantDeny(grantDeny)
                    else:
                        acesGrant[ace.principal.principalURL] = ACE()
                        acesGrant[ace.principal.principalURL].copy(ace)
                        acesGrant[ace.principal.principalURL].grantDenies = []
                        acesGrant[ace.principal.principalURL].addGrantDeny(grantDeny)
                else:
                    if ace.principal.principalURL in acesDeny:
                        ace.addGrantDeny(grantDeny)
                    else:
                        acesDeny[ace.principal.principalURL] = ACE()
                        acesDeny[ace.principal.principalURL].copy(ace)
                        acesDeny[ace.principal.principalURL].grantDenies = []
                        acesDeny[ace.principal.principalURL].addGrantDeny(grantDeny)
        newAcl = ACL()
        newAcl.addAces(acesGrant.values())
        newAcl.addAces(acesDeny.values())
        return newAcl

    def isValid(self):
        """
        Returns true (1) if all contained ACE objects are valid, 
        otherwise false (0) is returned.
        
        @return: Validity of ACL.
        @rtype:  C{bool}
        """
        valid = 1
        if len(self.aces):
            for ace in self.aces:
                if not ace.isValid():
                    valid = 0
        return valid

    def stripAces(self, inherited=True, protected=True):
        """
        Returns an ACL object with all ACEs stripped that are inherited 
        and/or protected.
        
        @param inherited: Flag to indicate whether inherited ACEs should 
            be stripped (default: True).
        @type  inherited: C{bool}
        @param protected: Flag to indicate whether protected ACEs should 
            be stripped (default: True).
        @type  protected: C{bool}
        
        @return: An ACL without the stripped ACEs.
        @rtype:  L{ACL} object
        """
        newAcl = ACL()
        if len(self.aces):
            for ace in self.aces:
                keep = 1
                if inherited and ace.inherited:
                    keep = 0
                elif protected and ace.protected:
                    keep = 0
                if keep:
                    newAcl.addAce(ace)
        return newAcl
