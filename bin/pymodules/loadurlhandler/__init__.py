try:
    loadurlhandler #err, this embedded context is weird?
except: #first run
    import loadurlhandler
else:
    loadurlhandler = reload(loadurlhandler)

from loadurlhandler import loadurl, LoadURLHandler
