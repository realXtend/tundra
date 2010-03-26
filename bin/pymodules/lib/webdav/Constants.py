# pylint: disable-msg=C0103
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
Contains XML tag names for the WebDAV protocol (RFC 2815)
and further WebDAV related constants.
"""


__version__ = "$Revision: 7 $"[11:-2]


QUOTED_SHARP = "%23"
SHARP = "#"

# Date formats
DATE_FORMAT_ISO8601 = r"%Y-%m-%dT%H:%M:%SZ"
DATE_FORMAT_HTTP = r"%a, %d %b %Y %H:%M:%S GMT"  # not used, substituted by rfc822 function

NS_DAV = 'DAV:'
NS_TAMINO = 'http://namespaces.softwareag.com/tamino/response2'

TAG_PROPERTY_FIND   = 'propfind'
TAG_PROPERTY_NAME   = 'propname'
TAG_PROPERTY_UPDATE = 'propertyupdate'
TAG_PROPERTY_SET    = 'set'
TAG_PROPERTY_REMOVE = 'remove'
TAG_ALL_PROPERTY    = 'allprop'
TAG_PROP            = 'prop'

TAG_MULTISTATUS         = 'multistatus'
TAG_RESPONSE            = 'response'
TAG_HREF                = 'href'
TAG_PROPERTY_STATUS     = 'propstat'
TAG_STATUS              = 'status'
TAG_RESPONSEDESCRIPTION = 'responsdescription'

PROP_CREATION_DATE    = 'creationdate'
PROP_DISPLAY_NAME     = 'displayname'
PROP_CONTENT_LANGUAGE = 'getcontentlanguage'
PROP_CONTENT_LENGTH   = 'getcontentlength'
PROP_CONTENT_TYPE     = 'getcontenttype'
PROP_ETAG             = 'getetag'
PROP_MODIFICATION_DATE = 'modificationdate' # this property is supported by
# Tamino 4.4 but not by Catacomb; the date format is ISO8601
PROP_LAST_MODIFIED    = 'getlastmodified'
PROP_LOCK_DISCOVERY   = 'lockdiscovery'
PROP_RESOURCE_TYPE    = 'resourcetype'
PROP_SOURCE           = 'source'
PROP_SUPPORTED_LOCK   = 'supportedlock'
PROP_OWNER            = 'owner'

PROP_RESOURCE_TYPE_RESOURCE    = 'resource'
PROP_RESOURCE_TYPE_COLLECTION  = 'collection'

TAG_LINK             = 'link'
TAG_LINK_SOURCE      = 'src'
TAG_LINK_DESTINATION = 'dst'

TAG_LOCK_ENTRY     = 'lockentry'
TAG_LOCK_SCOPE     = 'lockscope'
TAG_LOCK_TYPE      = 'locktype'
TAG_LOCK_INFO      = 'lockinfo'
TAG_ACTIVE_LOCK    = 'activelock'
TAG_LOCK_DEPTH     = 'depth'
TAG_LOCK_TOKEN     = 'locktoken'
TAG_LOCK_TIMEOUT   = 'timeout'
TAG_LOCK_EXCLUSIVE = 'exclusive'
TAG_LOCK_SHARED    = 'shared'
TAG_LOCK_OWNER     = 'owner'

# HTTP error code constants
CODE_MULTISTATUS = 207
CODE_SUCCEEDED = 200
CODE_CREATED = 201
CODE_NOCONTENT = 204

CODE_LOWEST_ERROR = 300

CODE_UNAUTHORIZED = 401
CODE_FORBIDDEN = 403
CODE_NOT_FOUND = 404
CODE_CONFLICT = 409
CODE_PRECONDITION_FAILED = 412
CODE_LOCKED = 423   # no permission
CODE_FAILED_DEPENDENCY = 424

CODE_OUTOFMEM = 507

# ?
CONFIG_UNICODE_URL = 1

# constants for WebDAV DASL according to draft

TAG_SEARCH_REQUEST = 'searchrequest'
TAG_SEARCH_BASIC   = 'basicsearch'
TAG_SEARCH_SELECT  = 'select'
TAG_SEARCH_FROM    = 'from'
TAG_SEARCH_SCOPE   = 'scope'
TAG_SEARCH_WHERE   = 'where'

# constants for WebDAV ACP (according to draft-ietf-webdav-acl-09) below ...

TAG_ACL                 = 'acl'
TAG_ACE                 = 'ace'
TAG_GRANT               = 'grant'
TAG_DENY                = 'deny'
TAG_PRIVILEGE           = 'privilege'
TAG_PRINCIPAL           = 'principal'
TAG_ALL                 = 'all'
TAG_AUTHENTICATED       = 'authenticated'
TAG_UNAUTHENTICATED     = 'unauthenticated'
TAG_OWNER               = 'owner'
TAG_PROPERTY            = 'property'
TAG_SELF                = 'self'
TAG_INHERITED           = 'inherited'
TAG_PROTECTED           = 'protected'
TAG_SUPPORTED_PRIVILEGE = 'supported-privilege'
TAG_DESCRIPTION         = 'description'

# privileges for WebDAV ACP:
TAG_READ = 'read'
TAG_WRITE = 'write'
TAG_WRITE_PROPERTIES = 'write-properties'
TAG_WRITE_CONTENT = 'write-content'
TAG_UNLOCK = 'unlock'
TAG_READ_ACL = 'read-acl'
TAG_READ_CURRENT_USER_PRIVILEGE_SET = 'read-current-user-privilege-set'
TAG_WRITE_ACL = 'write-acl'
TAG_ALL = 'all'
TAG_BIND = 'bind'
TAG_UNBIND = 'unbind'

# tamino specific security option
TAG_TAMINO_SECURITY = 'security'

# maybe this shouldn't be hard coded in here, but for now we'll just have to
# live with it this way ...
TAMINO_PRIVILEGES     = (TAG_READ, TAG_WRITE, TAG_READ_ACL, TAG_WRITE_ACL)

# properties for WebDAV ACP:
PROP_CURRENT_USER_PRIVILEGE_SET = 'current-user-privilege-set'
PROP_SUPPORTED_PRIVILEGE_SET    = 'supported-privilege-set'
PROP_PRINCIPAL_COLLECTION_SET = 'principal-collection-set' 

# reports for WebDAV ACP
REPORT_ACL_PRINCIPAL_PROP_SET   = 'acl-principal-prop-set'



# constants for WebDAV Delta-V

#   WebDAV Delta-V method names
METHOD_REPORT = 'REPORT'
METHOD_VERSION_CONTROL = 'VERSION-CONTROL'
METHOD_UNCHECKOUT = 'UNCHECKOUT'
METHOD_CHECKOUT = 'CHECKOUT'
METHOD_CHECKIN = 'CHECKIN'
METHOD_UPDATE = 'UPDATE'

#   Special properties
PROP_SUCCESSOR_SET = (NS_DAV, 'successor-set')
PROP_PREDECESSOR_SET = (NS_DAV, 'predecessor-set')
PROP_VERSION_HISTORY = (NS_DAV, 'version-history')
PROP_CREATOR = (NS_DAV, 'creator-displayname')
PROP_VERSION_NAME = (NS_DAV, 'version-name')
PROP_CHECKEDIN = (NS_DAV, 'checked-in')
PROP_CHECKEDOUT = (NS_DAV, 'checked-out')
PROP_COMMENT = (NS_DAV, 'comment')

#   XML tags for request body
TAG_VERSION_TREE = 'version-tree'
TAG_LOCATE_BY_HISTORY = 'locate-by-history'
TAG_UPDATE = 'update'
TAG_VERSION = 'version'

# HTTP header constants
HTTP_HEADER_DEPTH_INFINITY  = 'infinity'
HTTP_HEADER_IF  = 'if'
HTTP_HEADER_DAV = 'dav'
HTTP_HEADER_DASL = 'dasl'
HTTP_HEADER_OPTION_ACL = 'access-control'
HTTP_HEADER_OPTION_DAV_BASIC_SEARCH = 'DAV:basicsearch'
HTTP_HEADER_SERVER = 'server'
HTTP_HEADER_SERVER_TAMINO = 'Apache/2.0.54 (Win32)'
