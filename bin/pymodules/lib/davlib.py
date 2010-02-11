# pylint: disable-msg=W0402,W0231,W0141,R0903,C0321,W0701,R0904,C0103,W0201,W0102,R0913,W0622,E1101,C0111,C0121,R0901
# DAV client library
#
# Copyright (C) 1998-2000 Guido van Rossum. All Rights Reserved.
# Written by Greg Stein. Given to Guido. Licensed using the Python license.
#
# This module is maintained by Greg and is available at:
#    http://www.lyra.org/greg/python/davlib.py
#
# Since this isn't in the Python distribution yet, we'll use the CVS ID
# for tracking:
#   $Id: davlib.py 3182 2008-02-22 15:57:55 +0000 (Fr, 22 Feb 2008) schlauch $
#

import httplib
import urllib
import string
import types
import mimetypes
import qp_xml


INFINITY = 'infinity'
XML_DOC_HEADER = '<?xml version="1.0" encoding="utf-8"?>'
XML_CONTENT_TYPE = 'text/xml; charset="utf-8"'

# block size for copying files up to the server
BLOCKSIZE = 16384


class HTTPProtocolChooser(httplib.HTTPSConnection):
    def __init__(self, *args, **kw):
        self.protocol = kw.pop('protocol')
        if self.protocol == "https":
            self.default_port = 443
        else:
            self.default_port = 80
            
        apply(httplib.HTTPSConnection.__init__, (self,) + args, kw)

    def connect(self):
        if self.protocol == "https":
            httplib.HTTPSConnection.connect(self)
        else:
            httplib.HTTPConnection.connect(self)


class HTTPConnectionAuth(HTTPProtocolChooser):
    def __init__(self, *args, **kw):
        apply(HTTPProtocolChooser.__init__, (self,) + args, kw)

        self.__username = None
        self.__password = None
        self.__nonce = None
        self.__opaque = None

    def setauth(self, username, password):
        self.__username = username
        self.__password = password


def _parse_status(elem):
    text = elem.textof()
    idx1 = string.find(text, ' ')
    idx2 = string.find(text, ' ', idx1+1)
    return int(text[idx1:idx2]), text[idx2+1:]

class _blank:
    def __init__(self, **kw):
        self.__dict__.update(kw)
class _propstat(_blank): pass
class _response(_blank): pass
class _multistatus(_blank): pass

def _extract_propstat(elem):
    ps = _propstat(prop={}, status=None, responsedescription=None)
    for child in elem.children:
        if child.ns != 'DAV:':
            continue
        if child.name == 'prop':
            for prop in child.children:
                ps.prop[(prop.ns, prop.name)] = prop
        elif child.name == 'status':
            ps.status = _parse_status(child)
        elif child.name == 'responsedescription':
            ps.responsedescription = child.textof()
        ### unknown element name

    return ps

def _extract_response(elem):
    resp = _response(href=[], status=None, responsedescription=None, propstat=[])
    for child in elem.children:
        if child.ns != 'DAV:':
            continue
        if child.name == 'href':
            resp.href.append(child.textof())
        elif child.name == 'status':
            resp.status = _parse_status(child)
        elif child.name == 'responsedescription':
            resp.responsedescription = child.textof()
        elif child.name == 'propstat':
            resp.propstat.append(_extract_propstat(child))
        ### unknown child element

    return resp

def _extract_msr(root):
    if root.ns != 'DAV:' or root.name != 'multistatus':
        raise 'invalid response: <DAV:multistatus> expected'

    msr = _multistatus(responses=[ ], responsedescription=None)

    for child in root.children:
        if child.ns != 'DAV:':
            continue
        if child.name == 'responsedescription':
            msr.responsedescription = child.textof()
        elif child.name == 'response':
            msr.responses.append(_extract_response(child))
        ### unknown child element

    return msr

def _extract_locktoken(root):
    if root.ns != 'DAV:' or root.name != 'prop':
        raise 'invalid response: <DAV:prop> expected'
    elem = root.find('lockdiscovery', 'DAV:')
    if not elem:
        raise 'invalid response: <DAV:lockdiscovery> expected'
    elem = elem.find('activelock', 'DAV:')
    if not elem:
        raise 'invalid response: <DAV:activelock> expected'
    elem = elem.find('locktoken', 'DAV:')
    if not elem:
        raise 'invalid response: <DAV:locktoken> expected'
    elem = elem.find('href', 'DAV:')
    if not elem:
        raise 'invalid response: <DAV:href> expected'
    return elem.textof()


class DAVResponse(httplib.HTTPResponse):
    def parse_multistatus(self):
        self.root = qp_xml.Parser().parse(self)
        self.msr = _extract_msr(self.root)

    def parse_lock_response(self):
        self.root = qp_xml.Parser().parse(self)
        self.locktoken = _extract_locktoken(self.root)


class DAV(HTTPConnectionAuth):

    response_class = DAVResponse

    def get(self, url, extra_hdrs={ }):
        return self._request('GET', url, extra_hdrs=extra_hdrs)

    def head(self, url, extra_hdrs={ }):
        return self._request('HEAD', url, extra_hdrs=extra_hdrs)

    def post(self, url, data={ }, body=None, extra_hdrs={ }):
        headers = extra_hdrs.copy()

        assert body or data, "body or data must be supplied"
        assert not (body and data), "cannot supply both body and data"
        if data:
            body = ''
            for key, value in data.items():
                if isinstance(value, types.ListType):
                    for item in value:
                        body = body + '&' + key + '=' + urllib.quote(str(item))
                else:
                    body = body + '&' + key + '=' + urllib.quote(str(value))
            body = body[1:]
            headers['Content-Type'] = 'application/x-www-form-urlencoded'

        return self._request('POST', url, body, headers)

    def options(self, url='*', extra_hdrs={ }):
        return self._request('OPTIONS', url, extra_hdrs=extra_hdrs)

    def trace(self, url, extra_hdrs={ }):
        return self._request('TRACE', url, extra_hdrs=extra_hdrs)

    def put(self, url, contents,
            content_type=None, content_enc=None, extra_hdrs={ }):

        if not content_type:
            content_type, content_enc = mimetypes.guess_type(url)

        headers = extra_hdrs.copy()
        if content_type:
            headers['Content-Type'] = content_type
        if content_enc:
            headers['Content-Encoding'] = content_enc
        return self._request('PUT', url, contents, headers)

    def delete(self, url, extra_hdrs={ }):
        return self._request('DELETE', url, extra_hdrs=extra_hdrs)

    def propfind(self, url, body=None, depth=None, extra_hdrs={ }):
        headers = extra_hdrs.copy()
        headers['Content-Type'] = XML_CONTENT_TYPE
        if depth is not None:
            headers['Depth'] = str(depth)
        return self._request('PROPFIND', url, body, headers)

    def proppatch(self, url, body, extra_hdrs={ }):
        headers = extra_hdrs.copy()
        headers['Content-Type'] = XML_CONTENT_TYPE
        return self._request('PROPPATCH', url, body, headers)

    def mkcol(self, url, extra_hdrs={ }):
        return self._request('MKCOL', url, extra_hdrs=extra_hdrs)

    def move(self, src, dst, extra_hdrs={ }):
        headers = extra_hdrs.copy()
        headers['Destination'] = dst
        return self._request('MOVE', src, extra_hdrs=headers)

    def copy(self, src, dst, depth=None, extra_hdrs={ }):
        headers = extra_hdrs.copy()
        headers['Destination'] = dst
        if depth is not None:
            headers['Depth'] = str(depth)
        return self._request('COPY', src, extra_hdrs=headers)

    def lock(self, url, owner='', timeout=None, depth=None,
             scope='exclusive', type='write', extra_hdrs={ }):
        headers = extra_hdrs.copy()
        headers['Content-Type'] = XML_CONTENT_TYPE
        if depth is not None:
            headers['Depth'] = str(depth)
        if timeout is not None:
            headers['Timeout'] = timeout
        body = XML_DOC_HEADER + \
               '<DAV:lockinfo xmlns:DAV="DAV:">' + \
               '<DAV:lockscope><DAV:%s/></DAV:lockscope>' % scope + \
               '<DAV:locktype><DAV:%s/></DAV:locktype>' % type + \
               '<DAV:owner>' + owner + '</DAV:owner>' + \
               '</DAV:lockinfo>'
        return self._request('LOCK', url, body, extra_hdrs=headers)

    def unlock(self, url, locktoken, extra_hdrs={ }):
        headers = extra_hdrs.copy()
        if locktoken[0] != '<':
            locktoken = '<' + locktoken + '>'
        headers['Lock-Token'] = locktoken
        return self._request('UNLOCK', url, extra_hdrs=headers)

    def _request(self, method, url, body=None, extra_hdrs={}):
        "Internal method for sending a request."

        self.request(method, url, body, extra_hdrs)
        return self.getresponse()


    #
    # Higher-level methods for typical client use
    #

    def allprops(self, url, depth=None):
        body = XML_DOC_HEADER + \
               '<DAV:propfind xmlns:DAV="DAV:"><DAV:allprop/></DAV:propfind>'
        return self.propfind(url, body, depth=depth)

    def propnames(self, url, depth=None):
        body = XML_DOC_HEADER + \
               '<DAV:propfind xmlns:DAV="DAV:"><DAV:propname/></DAV:propfind>'
        return self.propfind(url, body, depth)

    def getprops(self, url, *names, **kw):
        assert names, 'at least one property name must be provided'
        if kw.has_key('ns'):
            xmlns = ' xmlns:NS="' + kw['ns'] + '"'
            ns = 'NS:'
            del kw['ns']
        else:
            xmlns = ns = ''
        if kw.has_key('depth'):
            depth = kw['depth']
            del kw['depth']
        else:
            depth = 0
        assert not kw, 'unknown arguments'
        body = XML_DOC_HEADER + \
               '<DAV:propfind xmlns:DAV="DAV:"' + xmlns + '><DAV:prop><' + ns + \
               string.joinfields(names, '/><' + ns) + \
               '/></DAV:prop></DAV:propfind>'
        return self.propfind(url, body, depth)

    def delprops(self, url, *names, **kw):
        assert names, 'at least one property name must be provided'
        if kw.has_key('ns'):
            xmlns = ' xmlns:NS="' + kw['ns'] + '"'
            ns = 'NS:'
            del kw['ns']
        else:
            xmlns = ns = ''
        assert not kw, 'unknown arguments'
        body = XML_DOC_HEADER + \
               '<DAV:propertyupdate xmlns:DAV="DAV:"' + xmlns + \
               '><DAV:remove><DAV:prop><' + ns + \
               string.joinfields(names, '/><' + ns) + \
               '/></DAV:prop></DAV:remove></DAV:propertyupdate>'
        return self.proppatch(url, body)

    def setprops(self, url, *xmlprops, **props):
        assert xmlprops or props, 'at least one property must be provided'
        xmlprops = list(xmlprops)
        if props.has_key('ns'):
            xmlns = ' xmlns:NS="' + props['ns'] + '"'
            ns = 'NS:'
            del props['ns']
        else:
            xmlns = ns = ''
        for key, value in props.items():
            if value:
                xmlprops.append('<%s%s>%s</%s%s>' % (ns, key, value, ns, key))
            else:
                xmlprops.append('<%s%s/>' % (ns, key))
        elems = string.joinfields(xmlprops, '')
        body = XML_DOC_HEADER + \
               '<DAV:propertyupdate xmlns:DAV="DAV:"' + xmlns + \
               '><DAV:set><DAV:prop>' + \
               elems + \
               '</DAV:prop></DAV:set></DAV:propertyupdate>'
        return self.proppatch(url, body)

    def get_lock(self, url, owner='', timeout=None, depth=None):
        response = self.lock(url, owner, timeout, depth)
        response.parse_lock_response()
        return response.locktoken
    