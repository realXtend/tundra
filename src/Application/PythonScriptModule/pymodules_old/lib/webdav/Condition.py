# pylint: disable-msg=R0921,W0704,R0901,W0511,R0201
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
This module contains classes for creating a search condition according to the DASL draft.
The classes will output the WHERE part of a search request to a WebDAV server.

Instances of the classes defined in this module form a tree data structure which represents
a search condition. This tree is made up of AND-nodes, OR-nodes, Operator- and comparison-
nodes and from property (i.e. variable) and constant leaf nodes.
"""


import types
from time import strftime
from calendar import timegm
from rfc822 import formatdate

from webdav.Constants import NS_DAV, PROP_LAST_MODIFIED, DATE_FORMAT_ISO8601


__version__ = "$Revision: 7 $"[11:-2]


class ConditionTerm(object):
    """
    This is the abstact base class for all condition terms.
    """
    def __init__(self):
        pass
    
    def toXML(self):
        """
        Abstact method which return a XML string which can be passed to a WebDAV server
        for a search condition.
        """
        raise NotImplementedError
    
    # start Tamino workaround for missing like-op:
    def postFilter(self, resultSet):
        """
        Abstact method for temporary workaround for Tamino's absense of the like-operator.
        This method shall filter the given result set for those resources which match
        all Contains-trems.
        """
        return resultSet
    # end of workaround


class IsCollectionTerm(ConditionTerm):
    """ Leaf condition. Checks if the matching resources are collections. """
    
    def __init__(self):
        """ Constructor. """
        
        ConditionTerm.__init__(self)

    def toXML(self):
        """
        Returns XML encoding.
        """
        
        return "<D:is-collection/>"


class Literal(ConditionTerm):
    """
    A leaf class for condition expressions representing a constant value.
    """
    def __init__(self, literal):
        ConditionTerm.__init__(self)
        self.literal = literal
        
    def toXML(self):
        '''
        Returns XML encoding.
        '''
        return "<D:literal>" + self.literal + "</D:literal>"
    

class UnaryTerm(ConditionTerm):
    """
    Base class of all nodes with a single child node.
    """
    def __init__(self, child):
        ConditionTerm.__init__(self)
        self.child = child
    
    def toXML(self):
        '''
        Returns XML encoding.
        '''
        return self.child.toXML()
    
    
class BinaryTerm(ConditionTerm):
    """
    Base class of all nodes with two child nodes
    """
    def __init__(self, left, right):
        ConditionTerm.__init__(self)
        self.left = left
        self.right = right
    
    def toXML(self):
        '''
        Returns XML encoding.
        '''
        return self.left.toXML() + self.right.toXML()

class TupleTerm(ConditionTerm):
    """
    Base class of all nodes with multiple single child nodes.
    """
    def __init__(self, terms):
        ConditionTerm.__init__(self)
        self.terms = terms
    
    def addTerm(self, term):
        '''
        Removes a term.
        
        @param term: term to add
        '''
        self.terms.append(term)
    
    def removeTerm(self, term):
        '''
        Adds a term.
        
        @param term: term to remove
        '''   
        try:
            self.terms.remove(term)
        except ValueError:
            pass
                
    def toXML(self):
        '''
        Returns XML encoding.
        '''
        result = ""
        for term in self.terms:
            result += term.toXML()
        return result


class AndTerm(TupleTerm):
    """
    This class represents and logical AND-condition with multiple sub terms.
    """
    def toXML(self):
        '''
        Returns XML encoding.
        '''
        return "<D:and>" + TupleTerm.toXML(self) + "</D:and>"

    # start Tamino workaround for missing like-op:
    def postFilter(self, resultSet):
        '''
        Filters the given result set. This is a TAMINO WebDav server workaround
        for the missing 'like' tag.
        
        @param resultSet: the result set that needs to be filtered.
        '''
        for term in self.terms:
            filtered = term.postFilter(resultSet)
            resultSet = filtered
        return resultSet
    # end of workaround

class OrTerm(TupleTerm):
    """
    This class represents and logical OR-condition with multiple sub terms.
    """
    def toXML(self):
        '''
        Returns XML encoding.
        '''
        return "<D:or>" + TupleTerm.toXML(self) + "</D:or>"
    
    # start Tamino workaround for missing like-op:
    def postFilter(self, resultSet):
        '''
        Filters the given result set. This is a TAMINO WebDav server workaround
        for the missing 'like' tag.
        
        @param resultSet: the result set that needs to be filtered.
        '''
        raise NotImplementedError

   
class NotTerm(UnaryTerm):
    """
    This class represents a negation term for the contained sub term.
    """
    def toXML(self):
        '''
        Returns XML encoding.
        '''
        # start Tamino workaround for missing like-op:
        if  isinstance(self.child, ContainsTerm):
            return ""
        # end of workaround        
        return "<D:not>" + UnaryTerm.toXML(self) + "</D:not>"

    # start Tamino workaround for missing like-op:
    def postFilter(self, resultSet):
        '''
        Filters the given result set. This is a TAMINO WebDav server workaround
        for the missing 'like' tag.
        
        @param resultSet: the result set that needs to be filtered.
        '''
        if  isinstance(self.child, ContainsTerm):
            self.child.negate = 1
            # TODO: pass on filter
        return self.child.postFilter(resultSet)
        

class ExistsTerm(UnaryTerm):
    """
    Nodes of this class must have a single child with tuple type (of len 2) representing a 
    WebDAV property.
    This leaf term evaluates to true if the (child) property exists.
    """
    def toXML(self):
        '''
        Returns XML encoding.
        '''
        return '<D:is-defined><D:prop xmlns="%s"><%s' % self.child + ' /></D:prop></D:is-defined>'

class ContentContainsTerm(UnaryTerm):
    """
    This class can be used to search for a given phrase in resources' contents.
    """
    def toXML(self):
        '''
        Returns XML encoding.
        '''
        return "<D:contains>" + self.child + "</D:contains>"



class BinaryRelationTerm(BinaryTerm):
    """
    This is the abstact base class for the following relation operands.
    """
    def __init__(self, left, right):
        BinaryTerm.__init__(self, left, right)
        if isinstance(self.left, types.StringType):     # Must be namespace + name pair
            self.left = ('DAV:', self.left)
        if not isinstance(self.right, Literal):
            self.right = Literal(self.right)             # Must be Literal instance

    def toXML(self):
        '''
        Returns XML encoding.
        '''
        ## TODO: extract name space and create shortcut for left element
        return '<D:prop xmlns="%s"><%s /></D:prop>' % self.left + self.right.toXML()

        
class StringRelationTerm(BinaryRelationTerm):
    """
    This is the abstact base class for the following string relation classes.
    """
    def __init__(self, left, right, caseless=None):
        """
        @param left: webdav property (namespace, name)
        @param right: string/unicode literal
        qparam caseless: 1 for case sensitive comparison
        """
        BinaryRelationTerm.__init__(self, left, Literal(right))
        self.caseless = caseless
        if self.caseless:
            self.attrCaseless = "yes"
        else:
            self.attrCaseless = "no"
        
class NumberRelationTerm(BinaryRelationTerm):
    """
    This is the abstact base class for the following number comparison classes.
    """
    def __init__(self, left, right):
        """
        @param left: webdav property (namespace, name)
        @param right: constant number
        """
        ## TODO: implemet typed literal
        BinaryRelationTerm.__init__(self, left, Literal(str(right)))

class DateRelationTerm(BinaryRelationTerm):
    """
    This is the abstact base class for the following date comparison classes.
    """
    def __init__(self, left, right):
        """
        @param left: webdav property (namespace, name)
        @param right: string literal containing a date in ISO8601 format
        """
        ## TODO: implemet typed literal
        assert len(right) == 9, "No time is specified for literal: " + str(right)
        BinaryRelationTerm.__init__(self, left, right)        
        if self.left == (NS_DAV, PROP_LAST_MODIFIED):
            rfc822Time = formatdate(timegm(right))      # must not use locale setting                  
            self.right = Literal(rfc822Time)
        else:               
            self.right = Literal(strftime(DATE_FORMAT_ISO8601, right))


class MatchesTerm(StringRelationTerm):
    """
    Nodes of this class evaluate to true if the (child) property's value matches the (child) string.
    """
    def toXML(self):
        '''
        Returns XML encoding.
        '''
        return '<D:eq caseless="%s">' % self.attrCaseless + StringRelationTerm.toXML(self) + "</D:eq>"

class ContainsTerm(StringRelationTerm):        
    """
    Nodes of this class evaluate to true if the (left child) property's value contains the
    (right child) string.
    """
    def __init__(self, left, right, isTaminoWorkaround=False):
        right = unicode(right)
        StringRelationTerm.__init__(self, left, "%" + right + "%")
        # Tamino workaround: operator like is not yet implemented:
        self.negate = 0
        self.isTaminoWorkaround = isTaminoWorkaround 
            
    def toXML(self):
        '''
        Returns XML encoding.
        '''
        # Tamino workaround: operator like is not yet implemented:
        # Produce a is-defined-condition instead
        if self.isTaminoWorkaround:
            return "<D:isdefined><D:prop xmlns='%s'><%s" % self.left + " /></D:prop></D:isdefined>"
        else:
            return '<D:like caseless="%s">' % self.attrCaseless + StringRelationTerm.toXML(self) + "</D:like>"
        
    # start Tamino workaround for missing like-op:
    def postFilter(self, resultSet):
        '''
        Filters the given result set. This is a TAMINO WebDav server workaround
        for the missing 'like' tag.
        
        @param resultSet: the result set that needs to be filtered.
        '''
        newResult = {}
        word = self.right.literal[1:-1]       # remove leading and trailing '%' characters (see __init__())
        for url, properties in resultSet.items():
            value = properties.get(self.left)
            if self.negate:
                if not value or value.textof().find(word) < 0:
                    newResult[url] = properties                
            else:
                if value and value.textof().find(word) >= 0:
                    newResult[url] = properties
        return newResult
    # end of workaround

class IsEqualTerm(NumberRelationTerm):
    """
    Nodes of this class evaluate to true if the (left child) numerical property's value is equal
    to the (right child) number.
    """
    def toXML(self):
        '''
        Returns XML encoding.
        '''
        return "<D:eq>" + NumberRelationTerm.toXML(self) + "</D:eq>"

class IsGreaterTerm(NumberRelationTerm):
    """
    Nodes of this class evaluate to true if the (left child) numerical property's value is greater
    than the (right child) number.
    """
    def toXML(self):
        '''
        Returns XML encoding.
        '''
        return "<D:gt>" + NumberRelationTerm.toXML(self) + "</D:gt>"

class IsGreaterOrEqualTerm(NumberRelationTerm):
    """
    Nodes of this class evaluate to true if the (left child) numerical property's value is greater
    than or equal to the (right child) number.
    """
    def toXML(self):
        '''
        Returns XML encoding.
        '''
        return "<D:gte>" + NumberRelationTerm.toXML(self) + "</D:gte>"

class IsSmallerTerm(NumberRelationTerm):
    """
    Nodes of this class evaluate to true if the (left child) numerical property's value is less
    than the (right child) number.
    """
    def toXML(self):
        '''
        Returns XML encoding.
        '''
        return "<D:lt>" + NumberRelationTerm.toXML(self) + "</D:lt>"

class IsSmallerOrEqualTerm(NumberRelationTerm):
    """
    Nodes of this class evaluate to true if the (left child) numerical property's value is less
    than or equal to the (right child) number.
    """
    def toXML(self):
        '''
        Returns XML encoding.
        '''
        return "<D:lte>" + NumberRelationTerm.toXML(self) + "</D:lte>"


class OnTerm(DateRelationTerm):
    """
    Nodes of this class evaluate to true if the (left child) property's value is a date
    equal to the (right child) date.
    """
    def toXML(self):
        '''
        Returns XML encoding.
        '''
        return "<D:eq>" + DateRelationTerm.toXML(self) + "</D:eq>"

class AfterTerm(DateRelationTerm):
    """
    Nodes of this class evaluate to true if the (left child) property's value is a date
    succeeding the (right child) date.
    """
    def toXML(self):
        '''
        Returns XML encoding.
        '''
        return "<D:gt>" + DateRelationTerm.toXML(self) + "</D:gt>"

class BeforeTerm(DateRelationTerm):
    """
    Nodes of this class evaluate to true if the (left child) property's value is a date
    preceeding the (right child) date.
    """
    def toXML(self):
        '''
        Returns XML encoding.
        '''
        return "<D:lt>" + DateRelationTerm.toXML(self) + "</D:lt>"


    
# Simple module test
if  __name__ == '__main__':
    # use the example from the webdav specification
    condition = AndTerm( (MatchesTerm('getcontenttype', 'image/gif'), \
                IsGreaterTerm('getcontentlength', 4096)) )
    print "Where: " + condition.toXML()
