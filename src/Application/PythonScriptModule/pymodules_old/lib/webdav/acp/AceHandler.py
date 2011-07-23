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
Handling of WebDAV Access Protocol Extensions and ACL preparation for UI.
"""


from webdav import Constants
from webdav.WebdavClient import ResourceStorer
from webdav.Connection import WebdavError


__version__ = "$LastChangedRevision: 13 $"


def extractSupportedPrivilegeSet(userPrivileges):
    """
    Returns a dictionary of supported privileges.
    
    @param userPrivileges: A DOM tree.
    @type  userPrivileges: L{webdav.WebdavResponse.Element} object
    
    @raise WebdavError: When unknown elements appear in the 
        C{DAV:supported-privilege} appear a L{WebdavError} is raised.
    
    @return: A dictionary with privilege names as keys and privilege descriptions as values.
    @rtype:  C{dictionary}
    """
    result = {}
    for element in userPrivileges.children:
        if element.name == Constants.TAG_SUPPORTED_PRIVILEGE:
            privName        = ''
            privDescription = ''
            for privilege in element.children:
                if privilege.name == Constants.TAG_PRIVILEGE:
                    privName = privilege.children[0].name
                elif privilege.name == Constants.TAG_DESCRIPTION:
                    privDescription = privilege.textof()
                else:
                    raise WebdavError('Unknown element in DAV:supported-privilege: ' + privilege.name)
                
                if privName and privDescription:
                    result[privName] = privDescription
                    privName        = ''
                    privDescription = ''
        else:
            raise WebdavError('Invalid element tag in DAV:supported-privilege-set: ' + element.name)
    return result


def _insertAclDisplaynames(acl):
    """
    Modifies the ACL by adding the human readable names 
    (DAV:displayname property) of each principal found in an ACL.
    
    This should be done with the REPORT method, but it is not supported by 
    Jacarta Slide, yet. (As of Aug. 1, 2003 in CVS repository)
    
    So we are going to do it differently by foot the harder way ...
    
    @param acl: An ACL object for which the displaynames should be retrieved.
    @type  acl: L{ACL} object
    """
    ## This is redundant code to be still kept for the REPORT method way of doing it ...
    ## property = '''<D:prop><D:displayname/></D:prop>'''
    ## return self.getReport(REPORT_ACL_PRINCIPAL_PROP_SET, property)
    for ace in acl.aces:
        if not ace.principal.property:
            principalConnection = \
                ResourceStorer(ace.principal.principalURL)
            ace.principal.displayname = \
                principalConnection.readProperty(Constants.NS_DAV, Constants.PROP_DISPLAY_NAME)


def prepareAcls(acls):
    """
    Returns all ACLs describing the behaviour of the resource. The information 
    in the ACL is modified to contain all information needed to display in the UI.
    
    @param acls: ACL objects.
    @type  acls: C{list} of L{ACL} objects
    
    @return: (non-valid) ACLs that contain both grant and deny clauses in an ACE.
        Displaynames are added to the Principals where needed.
    @rtype:  C{list} of L{ACL} objects
    """
    for acl in acls.keys():
        acls[acl] = acls[acl].joinGrantDeny()
        _insertAclDisplaynames(acls[acl])
    return acls


def prepareAcl(acl):
    """
    Returns an ACL describing the behaviour of the resource. The information 
    in the ACL is modified to contain all information needed to display in the UI.
    
    @param acl: An ACL object.
    @type  acl: L{ACL} object
    
    @return: A (non-valid) ACL that contains both grant and deny clauses in an ACE.
        Displaynames are added to the Principals where needed.
    @rtype:  L{ACL} object
    """
    acl = acl.joinGrantDeny()
    _insertAclDisplaynames(acl)
    return acl


def refineAclForSet(acl):
    """
    Sets the ACL composed from the UI on the WebDAV server. For that purpose the 
    ACL object gets refined first to form a well accepted ACL to be set by the 
    ACL WebDAV method.

    @param acl: An ACL object to be refined.
    @type  acl: L{ACL} object
    
    @return: A valid ACL that contains only grant or deny clauses in an ACE.
        Inherited and protected ACEs are stripped out.
    @rtype:  L{ACL} object
    """
    acl = acl.splitGrantDeny()
    acl = acl.stripAces()
    return acl


##~ unsupported or unfinished methods:
##~ 
##~ def report(self, report, request=None, lockToken=None):
##~     """
##~     This method implements the WebDAV ACP method: REPORT for given report 
##~     types.
##~     
##~     Parameters:
##~     
##~       'report' -- Report type as a string.
##~       
##~       'request' -- XML content of the request for the report (defaults to None).
##~       
##~       'lockToken' -- Lock token to be set (defaults to None).
##~     """
##~     raise WebdavError('Reports are not supported by our Jacarta Slide, yet (as of Aug. 1, 2003 in CVS).')
##~     
##~     headers                 = createCondition(lockToken)
##~     headers['Content-Type'] = XML_CONTENT_TYPE
##~     body                    = '<D:%s xmlns:D="DAV:">%s</D:%s>' % (report, request, report)
##~     print "Body: ", body
##~     response = self.connection._request('REPORT', self.path, body, headers)
##~     return response
##~     ## TODO: parse DAV:error response
##~ 
##~ 
##~ def getAllAcls(self):
##~    """
##~     Returns a dictionary of ACL resources with respective ACL objects 
##~     that apply to the given resource.
##~     
##~     ### This method needs to be extended for inherited ACLs when Tamino
##~     support tells me (Guy) how to get to them.
##~     """
##~     acls = {self.path: self.getAcl()}
##~     for ace in acls[self.path].aces:
##~         if ace.inherited:
##~             if not ace.inherited in acls:
##~                 acls[ace.inherited] = self.getAcl()
##~    
##~     # append some more stuff here to acls for possible inherited ACLs
##~     return acls
