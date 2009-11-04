# Module:	__init__
# Date:		3rd October 2008
# Author:	James Mills, prologic at shortcircuit dot net dot au

"""Circuits Library - Web

circuits.web contains the circuits full stack web server that is HTTP
and WSGI compliant.
"""

from utils import url
from loggers import Logger

try:
    from sessions import Sessions
except ImportError:
    print "couldn't load circuits.web.sessions, missing libs probably (uuid?), continuing without"

from core import expose, Controller
from events import Request, Response
from servers import BaseServer, Server
from errors import HTTPError, Forbidden, NotFound, Redirect
from dispatchers import Static, Dispatcher, VirtualHosts, XMLRPC

try:
    from dispatchers import JSONRPC
except ImportError:
    pass
