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
Handling of grant and deny clauses in ACEs according to WebDAV ACP specification.
"""


from webdav.acp.Privilege import Privilege
from webdav import Constants
from webdav.Connection import WebdavError


__version__ = "$LastChangedRevision: 13 $"


class GrantDeny(object):
    """
    This class provides functionality for handling
    grant and deny clauses in ACEs.
    
    @ivar grantDeny:  Flag indicating whether clause grants or denies.
    @type grantDeny:  C{bool}
    @ivar privileges: Privileges to be granted or denied.
    @type privileges: C{list} of L{Privilege} objects
    """
    
    def __init__(self, domroot=None):
        """
        Constructor should be called with either no parameters 
        (create blank GrantDeny), or one parameter (a DOM tree).
        
        @param domroot:     A DOM tree (default: None).
        @type  domroot:     L{webdav.WebdavResponse.Element} object
        
        @raise WebdavError: When non-valid parameters are passed a L{WebdavError} is raised.
        """
        self.grantDeny  = 0   # 0: deny, 1: grant
        self.privileges = []
        
        if domroot:
            self.grantDeny = (domroot.name == Constants.TAG_GRANT)
            for child in domroot.children:
                if child.name == Constants.TAG_PRIVILEGE and child.ns == Constants.NS_DAV:
                    self.privileges.append(Privilege(domroot=child))
                else:
                    # This shouldn't happen, someone screwed up with the params ...
                    raise WebdavError('Non-privilege tag handed to GrantDeny constructor: %s' \
                        % child.name)
        elif domroot == None:
            # no param ==> blank object
            pass
        else:
            # This shouldn't happen, someone screwed up with the params ...
            raise WebdavError('Non-valid parameters handed to GrantDeny constructor.')

    def __cmp__(self, other):
        """ Compares two GrantDeny instances. """
        if not isinstance(other, GrantDeny):
            return 1
        if self.grantDeny == other.grantDeny:
            equal = 1
            for priv in self.privileges:
                inList = 0
                for otherPriv in other.privileges:
                    if priv == otherPriv:
                        inList = 1
                if inList == 0:
                    equal = 0
            return not equal
        else:
            return 1

    def __repr__(self):
        """ Returns the representation of an instance. """
        representation = '<class GrantDeny: '
        if self.grantDeny:
            representation += 'grant privileges: ['
        else:
            representation += 'deny privileges: ['
        first = 1
        for priv in self.privileges:
            if first:
                representation += '%s' % priv
                first = 0
            else:
                representation += ', %s' % priv
        return '%s]>' % (representation)

    def copy(self, other):
        """
        Copy a GrantDeny object.
        
        @param other: Another grant or deny clause to copy.
        @type  other: L{GrantDeny} object
        
        @raise WebdavError: When an object that is not an L{GrantDeny} is passed 
            a L{WebdavError} is raised.
        """
        if not isinstance(other, GrantDeny):
            raise WebdavError('Non-GrantDeny object passed to copy method: %s' \
                % other)
        self.grantDeny = other.grantDeny
        if other.privileges:
            self.addPrivileges(other.privileges)

    def isGrant(self):
        """
        Returns whether the set of privileges is of type "grant"
        indicating true or false.
        
        @return: Value whether the clause is of grant type.
        @rtype:  C{bool}
        """
        return self.grantDeny

    def isDeny(self):
        """
        Returns whether the set of privileges is of type "deny"
        indicating true or false.
        
        @return: Value whether the clause is of deny type.
        @rtype:  C{bool}
        """
        return not self.grantDeny

    def setGrantDeny(self, grantDeny):
        """
        Sets the set of privileges to given value for grantDeny.
        
        @param grantDeny: Grant/deny value for clause (grant: True/1, deny: False/0).
        @type  grantDeny: C{bool}
        """
        if grantDeny == 0 or grantDeny == 1:
            self.grantDeny = grantDeny

    def setGrant(self):
        """ Sets the set of privileges to type "grant". """
        self.grantDeny = 1

    def setDeny(self):
        """ Sets the set of privileges to type "deny". """
        self.grantDeny = 0

    def isAll(self):
        """
        Checks whether the privileges contained are equal
        to aggregate DAV:all privilege.
        
        @return: Value whether all un-aggregated privileges are present.
        @rtype:  C{bool}
        """
        if len(self.privileges) == len(Constants.TAMINO_PRIVILEGES):
            return 1
        elif len(self.privileges) == 1 and self.privileges[0].name == Constants.TAG_ALL:
            return 1
        return 0

    def addPrivilege(self, privilege):
        """
        Adds the passed privilege to list if it's not in it, yet.
        
        @param privilege: A privilege.
        @type  privilege: L{Privilege} object
        """
        inList = False
        for priv in self.privileges:
            if priv == privilege:
                inList = True
        if not inList:
            newPrivilege = Privilege()
            newPrivilege.copy(privilege)
            self.privileges.append(newPrivilege)

    def addPrivileges(self, privileges):
        """
        Adds the list of passed privileges to list.
        
        @param privileges: Several privileges.
        @type  privileges: sequence of L{Privilege} objects
        """
        for priv in privileges:
            self.addPrivilege(priv)

    def delPrivilege(self, privilege):
        """
        Deletes the passed privilege from list if it's in it.
        
        @param privilege: A privilege.
        @type  privilege: L{Privilege} object
        
        @raise WebdavError: A L{WebdavError} is raised if the privilege to be 
            deleted is not present.
        """
        count = 0
        index = 0
        for priv in self.privileges:
            count += 1
            if priv == privilege:
                index = count
        if index:
            self.privileges.pop(index - 1)
        else:
            raise WebdavError('Privilege to be deleted not in list: %s' % privilege)

    def delPrivileges(self, privileges):
        """
        Deletes the list of passed privileges from list.
        
        @param privileges: Several privileges.
        @type  privileges: sequence of L{Privilege} objects
        """
        for priv in privileges:
            self.delPrivilege(priv)

    def toXML(self):
        """
        Returns string of GrantDeny content to valid XML as described in WebDAV ACP.
        """
        assert self.privileges, "GrantDeny object is not initialized or does not contain content!"
        
        if self.isGrant():
            tag = 'D:' + Constants.TAG_GRANT
        else:
            tag = 'D:' + Constants.TAG_DENY
            
        res = ''
        for privilege in self.privileges:
            res += privilege.toXML()
        return '<%s>%s</%s>' % (tag, res, tag)
