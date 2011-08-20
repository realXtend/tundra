This directory has a few different examples demonstrating how the Javascript sandboxing for untrusted code works in Naali.

* tooltip-httpscript.txml 
  modified ../Tooltip/ToolTipBox.txml which loads the js source with http from tooltip-httpscript.txml .
  works identically with the original, as input and scene manipulation work in the sandbox.

* systemaccess.js
  an example of how a malicious script with system access could e.g. read or delete your files.
  this is harmless, as it only prints your home directory and the files there, for you to see only.

* systemaccess-localscript.txml
  points to systemaccess.js with a local:// asset reference, 
  for which loading qt extensions like qt.core with system access is allowed, so systemaccess.js runs

* systemaccess-httpscript.txml
  points to the same systemaccess.js on a remote server with http.
  this way the code is *not* executed, but a warning is printed to the console instead.
