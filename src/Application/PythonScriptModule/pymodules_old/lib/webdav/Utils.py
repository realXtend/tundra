# pylint: disable-msg=W0141,R0912
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
The module contains functions to support use of the WebDav functionalities.
"""


import os
import sys

from webdav.WebdavClient import CollectionStorer, ResourceStorer
from webdav.Constants import NS_DAV, PROP_RESOURCE_TYPE, CODE_NOT_FOUND, PROP_RESOURCE_TYPE_RESOURCE
from webdav.Connection import WebdavError


__version__ = "$Revision: 13 $"[11:-2]


def resourceExists(node, name = None, resourceType = PROP_RESOURCE_TYPE_RESOURCE):
    """
    Check if resource exists.
    
    Usage:
      - resourceExists(ResourceStorer-object):
        check if resource exists
      - resourceExists(CollectionStorer-object, name):
        check if resource name exists in collection
    
    @param node: node that has to be checked or node of collection
    @type node: L{ResourceStorer<webdav.WebdavClient.ResourceStorer>}
    @param name: name of resource (in collection node) that has to be checked
    @type name: string
    
    @return: boolean
    
    @raise WebdavError: all WebDAV errors except WebDAV error 404 (not found)
    """
    
    exists = False
    if not node:
        return exists
    try:
        myResourceType = ""
        if name:
            # make sure it's unicode:
            if not isinstance(name, unicode):
                name = name.decode(sys.getfilesystemencoding())
            url = node.url
            if url.endswith("/"):
                url = url  + name
            else:
                url = url + "/" + name
            newNode = ResourceStorer(url, node.connection)
            element = newNode.readProperty(NS_DAV, PROP_RESOURCE_TYPE)
        else: # name is "None":
            element = node.readProperty(NS_DAV, PROP_RESOURCE_TYPE)
        
        if len(element.children) > 0:
            myResourceType = element.children[0].name
        if resourceType == myResourceType or resourceType == PROP_RESOURCE_TYPE_RESOURCE:
            exists = True
        else:
            exists = False
    except WebdavError, wderr:
        if wderr.code == CODE_NOT_FOUND:
            # node doesn't exist -> exists = False:
            exists = False
        else:
            # another exception occured -> "re-raise" it:
            raise
    return exists


def downloadCollectionContent(destinationPath, collectionToDownload):
    """
    Downloads the resources contained to the given directory.
    
    @param destinationPath: Path to download the files to, will be created if it not exists.
    @type destinationPath: C{String}
    @param collectionToDownload: Collection to download the content from.
    @type collectionToDownload: instance of L{CollectionStorer<webdav.WebdavClient.CollectionStorer>}
    
    @raise  WebdavError: If something goes wrong.
    """
    
    from time import mktime, gmtime

    downloadCount = 0

    listOfItems = collectionToDownload.getCollectionContents()
    
    if not os.path.exists(destinationPath):
        try:
            os.makedirs(destinationPath)
        except OSError:
            errorMessage = "Cannot create download destination directory '%s'." % destinationPath
            raise WebdavError(errorMessage)
        
    try:
        itemsInPath = os.listdir(destinationPath)
    except OSError:
        errorMessage = "Cannot read the content of download destination directory '%s'." % destinationPath
        raise WebdavError(errorMessage)
    
    for item in listOfItems:
        # skip collections
        if not isinstance(item[0], CollectionStorer):
            itemSavePath = os.path.join(destinationPath, item[0].name)
            existsItemSavePath = os.path.exists(itemSavePath)
            
            # update?
            if existsItemSavePath:
                try:
                    isUpdateNecessary = mktime(item[1].getLastModified()) > mktime(gmtime(os.path.getmtime(itemSavePath)))
                except (ValueError, OverflowError):
                    isUpdateNecessary = True
                # windows is not case sensitive
                for realItem in itemsInPath:
                    if realItem.lower() == item[0].name.lower():
                        itemsInPath.remove(realItem)
            else:
                isUpdateNecessary = True
            
            # download
            if not existsItemSavePath or (existsItemSavePath and isUpdateNecessary):
                item[0].downloadFile(itemSavePath)
                downloadCount = downloadCount + 1
    
    # delete old items
    try:
        for item in itemsInPath:
            os.remove(os.path.join(destinationPath, item))
    except OSError, e:
        if e.errno == 13:    # permission error
            sys.stderr.write("permission problem on '%s' in %s\n" % (e.filename, e.strerror))
        else:
            raise  
        
    return downloadCount
