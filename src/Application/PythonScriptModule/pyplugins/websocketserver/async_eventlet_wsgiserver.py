import errno
import os
import sys
import time
import traceback
import warnings
import socket

import eventlet.wsgi as wsgiserver
import eventlet.greenpool as greenpool
from eventlet.support import get_errno

def server(sock, site,
           log=None,
           environ=None,
           max_size=None,
           max_http_version=wsgiserver.DEFAULT_MAX_HTTP_VERSION,
           protocol=wsgiserver.HttpProtocol,
           server_event=None,
           minimum_chunk_size=None,
           log_x_forwarded_for=True,
           custom_pool=None,
           keepalive=True,
           log_format=wsgiserver.DEFAULT_LOG_FORMAT):

    sock.settimeout(0.001)
    #sock.settimeout(None)
    
    serv = wsgiserver.Server(sock, sock.getsockname(),
                  site, log,
                  environ=environ,
                  max_http_version=max_http_version,
                  protocol=protocol,
                  minimum_chunk_size=minimum_chunk_size,
                  log_x_forwarded_for=log_x_forwarded_for,
                  keepalive=keepalive,
                  log_format=log_format)
    if server_event is not None:
        server_event.send(serv)
    if max_size is None:
        max_size = wsgiserver.DEFAULT_MAX_SIMULTANEOUS_REQUESTS
    if custom_pool is not None:
        pool = custom_pool
    else:
        pool = greenpool.GreenPool(max_size)
    try:
        host, port = sock.getsockname()
        port = ':%s' % (port, )
        if hasattr(sock, 'do_handshake'):
            scheme = 'https'
            if port == ':443':
                port = ''
        else:
            scheme = 'http'
            if port == ':80':
                port = ''

        serv.log.write("(%s) wsgi starting up on %s://%s%s/\n" % (
            os.getpid(), scheme, host, port))
        while True:
            yield
            try:
                #select([sock], [], [], 
                try:
                    client_socket = sock.accept()
                except socket.timeout:
                    continue
                try:
                    pool.spawn_n(serv.process_request, client_socket)
                except AttributeError:
                    warnings.warn("wsgi's pool should be an instance of " \
                        "eventlet.greenpool.GreenPool, is %s. Please convert your"\
                        " call site to use GreenPool instead" % type(pool),
                        DeprecationWarning, stacklevel=2)
                    pool.execute_async(serv.process_request, client_socket)

            except wsgiserver.ACCEPT_EXCEPTIONS, e:
                if get_errno(e) not in ACCEPT_ERRNO:
                    raise
            except (KeyboardInterrupt, SystemExit):
                serv.log.write("wsgi exiting\n")
                break


    finally:
        try:
            # NOTE: It's not clear whether we want this to leave the
            # socket open or close it.  Use cases like Spawning want
            # the underlying fd to remain open, but if we're going
            # that far we might as well not bother closing sock at
            # all.
            sock.close()
        except socket.error, e:
            if get_errno(e) not in wsgiserver.BROKEN_SOCK:
                traceback.print_exc()
