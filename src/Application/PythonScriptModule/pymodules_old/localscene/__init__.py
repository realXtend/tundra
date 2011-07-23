# imitated loadurlhandler here, for direct access from C++ side

try:
    localscene
except:
    import localscene
else:
    localscene = reload(localscene)

from localscene import LocalScene, getLocalScene


