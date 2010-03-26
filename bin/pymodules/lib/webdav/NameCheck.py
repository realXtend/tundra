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
Check name of new collections/resources for "illegal" characters.
"""


import re
import unicodedata


__version__ = "$LastChangedRevision: 28 $"


_unicodeUmlaut = [unicodedata.lookup("LATIN CAPITAL LETTER A WITH DIAERESIS"),
                  unicodedata.lookup("LATIN SMALL LETTER A WITH DIAERESIS"),
                  unicodedata.lookup("LATIN CAPITAL LETTER O WITH DIAERESIS"),
                  unicodedata.lookup("LATIN SMALL LETTER O WITH DIAERESIS"),
                  unicodedata.lookup("LATIN CAPITAL LETTER U WITH DIAERESIS"),
                  unicodedata.lookup("LATIN SMALL LETTER U WITH DIAERESIS"),
                  unicodedata.lookup("LATIN SMALL LETTER SHARP S")]
 
# Define characters and character base sets
_german  = u"".join(_unicodeUmlaut)
_alpha = "A-Za-z"
_num = "0-9"
_alphaNum = _alpha + _num
_space = " "
_under = "_"
_dash = "\-"
_dot = "\."
_exclam = "\!"
_tilde   = "\~"
_dollar  = "\$"
_plus = "+"
_equal = "="
_sharp = "#"

# Define character groups
_letterNum = _alphaNum + _german
_letter = _alpha + _german

# Define character sets for names
firstPropertyChar = _letter + _under
propertyChar = firstPropertyChar + _num + _dash + _dot
firstResourceChar = firstPropertyChar + _num + _tilde + _exclam + _dollar + \
                     _dot + _dash + _plus + _equal + _sharp
resourceChar = firstResourceChar + _space

# Define regular expressions for name validation
_propertyFirstRe = re.compile(u"^["+ firstPropertyChar +"]")

_propertyRe = re.compile(u"[^"+ propertyChar +"]")
_resourceFirstRe = re.compile(u"^["+ firstResourceChar +"]")
_resourceRe = re.compile(u"[^"+ resourceChar +"]")
                    
                    
def isValidPropertyName(name):
    """
    Check if the given property name is valid.
    
    @param name: Property name.
    @type name: C{unicode}
    
    @return: Boolean indicating whether the given property name is valid or not. 
    @rtype: C{bool}
    """

    illegalChar = _propertyRe.search(name)
    return illegalChar == None  and  _propertyFirstRe.match(name) != None
    
    
def isValidResourceName(name):
    """
    Check if the given resource name is valid.
    
    @param name: Resource name.
    @type name: C{unicode}
    
    @return: Boolean indicating whether the given resource name is valid or not. 
    @rtype: C{bool}
    """
    
    illegalChar = _resourceRe.search(name)
    return illegalChar == None  and  _resourceFirstRe.match(name) != None


def validatePropertyName(name):
    """
    Check if the given property name is valid.
    
    @param name: Property name.
    @type name: C{unicode}
    @raise WrongNameError: if validation fails (see L{datafinder.common.NameCheck.WrongNameError})
    """
    
    illegalChar = _propertyRe.search(name)
    if illegalChar:
        raise WrongNameError(illegalChar.start(), name[illegalChar.start()])
    if not _propertyFirstRe.match(name):
        if len(name) > 0:
            raise WrongNameError(0, name[0])
        else:
            raise WrongNameError(0, 0)
       
    
def validateResourceName(name):
    """
    Check if the given resource name is valid.
    
    @param name: name of resource/collection
    @type name: C{unicode}
    @raise WrongNameError: if validation fails (@see L{datafinder.common.NameCheck.WrongNameError})
    """

    illegalChar = _resourceRe.search(name)
    if illegalChar:
        raise WrongNameError(illegalChar.start(), name[illegalChar.start()])
    if not _resourceFirstRe.match(name):
        if len(name) > 0:
            raise WrongNameError(0, name[0])
        else:
            raise WrongNameError(0, 0)


def getResourceNameErrorPosition(name):
    """
    Get position of illegal character (and the error-message).
    This method can be used to get this information if L{isValidPropertyName}
    or L{isValidResourceName} failed.
    
    @param name: Resource name.
    @type name: C{unicode}
    
    @return: Tuple of error position and message.
    @rtype: C{tuple} of C{int} and C{unicode}
    """

    result = (-1, None)
    illegalChar = _resourceRe.search(name)
    if illegalChar:
        result = (illegalChar.start(), \
                  u"Illegal character '%s' at index %d." % \
                      (name[illegalChar.start()], illegalChar.start()))
    elif not _resourceFirstRe.match(name):
        result = (0, u"Illegal character '%s' at index %d." % (name[0], 0))
    return result

    
class WrongNameError(ValueError):
    """
    Exception raised if an "illegal" character was found.
    
    @ivar character: character that caused the exception
    @type character: C{unicode}
    @ivar position: position of C{character}
    @type position: C{int}
    """
    
    def __init__(self, position, character):
        """
        Constructor.
        
        @param character: Character that caused the exception.
        @type character: C{unicode}
        @param position: Position of C{character}
        @type position: C{int}
        """
        
        ValueError.__init__(self)        
        self.character = character
        self.position = position
    
    def __str__(self):
        """ Returns string representation. """
        
        return ValueError.__str__(self) + \
            "Character '%s' at index %d." % (self.character, self.position)
