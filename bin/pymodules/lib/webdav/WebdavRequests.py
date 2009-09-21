# pylint: disable-msg=W0511,W0212,E1111
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
This module handles WebDav server requests.
"""


import types
from webdav import Constants
import qp_xml
from tempfile import TemporaryFile

from davlib import XML_DOC_HEADER

from webdav.NameCheck import validatePropertyName


__version__ = "$LastChangedRevision: 2 $"


## TODO: create a property list class
    
class XmlNameSpaceMangler(object):
    '''
    Handles WebDav requests.
    '''
    
    # restrict instance variables
    __slots__ = ('shortcuts', 'defaultNameSpace')
    
    def __init__(self, nameList, defaultNameSpace = None):
        '''
        
        @param nameList:
        @param defaultNameSpace:
        '''
        
        assert isinstance(nameList, types.ListType) or isinstance(nameList, types.TupleType), \
            "1. argument has wrong type %s" % type(nameList)
        self.shortcuts = {}
        self.defaultNameSpace = defaultNameSpace
        for name in nameList:
            if  not isinstance(name, types.TupleType):
                name = (defaultNameSpace, name)            
            assert isinstance(name, types.TupleType) and len(name) == 2, \
                         "Name is not a namespace, name tuple: %s" % type(name)
            validatePropertyName(name[1])
            if  name[0] and not self.shortcuts.has_key(name[0]):
                self.shortcuts[name[0]] = 'ns%d' % len(self.shortcuts)
    
    def getNameSpaces(self):
        '''
        Returns the namespace.
        '''
        
        result = ""        
        for namespace, short in self.shortcuts.items():
            result += ' xmlns:%s="%s"' % (short, namespace)
        return result
    
    def getUpdateElements(self, valueMap):
        '''
        
        @param valueMap:
        '''
        
        elements = ""
        for name in valueMap.keys():
            fullname = name
            if  isinstance(name, types.StringType):
                fullname = (self.defaultNameSpace, name)        
            if  not fullname[0]:
                tag = fullname[1]        
            else:
                tag = self.shortcuts[fullname[0]] + ':' + fullname[1]
            value = valueMap[name]
            if value:
                if isinstance(value, qp_xml._element):
                    tmpFile = TemporaryFile('w+')
                    value = qp_xml.dump(tmpFile, value)
                    tmpFile.flush()
                    tmpFile.seek(0)
                    tmpFile.readline()
                    value = tmpFile.read()
                else:
                    value = "<![CDATA[%s]]>" % value
            else:
                value = ""
            elements += "<%s>%s</%s>" % (tag, value, tag)
        return elements
    
    def getNameElements(self, nameList):
        '''
        
        @param nameList:
        '''
        
        elements = ""
        for name in nameList:
            if  isinstance(name, types.StringType):
                name = (self.defaultNameSpace, name)        
            if  not name[0]:
                tag = name[1]        
            else:
                tag = self.shortcuts[name[0]] + ':' + name[1]
            elements += "<%s />" % tag
        return elements
                         


def createUpdateBody(propertyDict, defaultNameSpace = None):
    '''
    
    @param propertyDict:
    @param defaultNameSpace:
    '''
    
    updateTag = 'D:' + Constants.TAG_PROPERTY_UPDATE
    setTag = 'D:' + Constants.TAG_PROPERTY_SET
    propTag = 'D:' + Constants.TAG_PROP
    mangler = XmlNameSpaceMangler(propertyDict.keys(), defaultNameSpace)
    return XML_DOC_HEADER + \
        '<%s xmlns:D="DAV:"><%s><%s %s>' % (updateTag, setTag, propTag, mangler.getNameSpaces()) + \
        mangler.getUpdateElements(propertyDict) + \
        '</%s></%s></%s>' % (propTag, setTag, updateTag)

        
def createDeleteBody(nameList, defaultNameSpace = None):
    '''
    
    @param nameList:
    @param defaultNameSpace:
    '''
    
    updateTag = 'D:' + Constants.TAG_PROPERTY_UPDATE
    removeTag = 'D:' + Constants.TAG_PROPERTY_REMOVE
    propTag = 'D:' + Constants.TAG_PROP
    mangler = XmlNameSpaceMangler(nameList, defaultNameSpace)
    return XML_DOC_HEADER + \
        '<%s xmlns:D="DAV:"><%s><%s %s>' % (updateTag, removeTag, propTag, mangler.getNameSpaces()) + \
        mangler.getNameElements(nameList) + \
        '</%s></%s></%s>' % (propTag, removeTag, updateTag)
        
        
def createFindBody(nameList, defaultNameSpace = None):
    '''
    
    @param nameList:
    @param defaultNameSpace:
    '''
    
    findTag = 'D:' + Constants.TAG_PROPERTY_FIND
    propTag = 'D:' + Constants.TAG_PROP
    mangler = XmlNameSpaceMangler(nameList, defaultNameSpace)
    return XML_DOC_HEADER + \
        '<%s xmlns:D="DAV:"><%s %s>' % (findTag, propTag, mangler.getNameSpaces()) + \
        mangler.getNameElements(nameList) + \
        '</%s></%s>' % (propTag, findTag)
        
        
def createSearchBody(selects, path, conditions, defaultNameSpace = None):
    '''
    Creates DASL XML body.
    
    @param selects: list of property names to retrieve for the found resources
    @param path: list of conditions
    @param conditions: tree of ConditionTerm instances representing a logical search term
    @param defaultNameSpace: default namespace
    '''
    
    searchTag = 'D:' + Constants.TAG_SEARCH_REQUEST
    basicTag = 'D:' + Constants.TAG_SEARCH_BASIC
    selectTag = 'D:' + Constants.TAG_SEARCH_SELECT
    fromTag = 'D:' + Constants.TAG_SEARCH_FROM
    scopeTag = 'D:' + Constants.TAG_SEARCH_SCOPE
    whereTag = 'D:' + Constants.TAG_SEARCH_WHERE
    propTag = 'D:' + Constants.TAG_PROP
    hrefTag = 'D:' + Constants.TAG_HREF
    depthTag = 'D:' + Constants.TAG_LOCK_DEPTH
    depthValue = Constants.HTTP_HEADER_DEPTH_INFINITY
    mangler = XmlNameSpaceMangler(selects, defaultNameSpace)
    return XML_DOC_HEADER + \
        '<%s xmlns:D="DAV:"><%s>' % (searchTag, basicTag) + \
        '<%s><%s %s>%s</%s></%s>' % (selectTag, propTag, mangler.getNameSpaces(), 
                                     mangler.getNameElements(selects), propTag, selectTag) + \
        '<%s><%s><%s>%s</%s><%s>%s</%s></%s></%s>' % (fromTag, scopeTag, hrefTag, path, hrefTag, 
                                                      depthTag, depthValue, depthTag, scopeTag, fromTag) + \
        '<%s>%s</%s>' % (whereTag, conditions.toXML(),whereTag) + \
        '</%s></%s>' % (basicTag, searchTag)
        