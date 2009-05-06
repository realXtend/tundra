import rexviewer as r

idnum = 720001
#idnum = 0

print "Getting entity id", idnum,
e = r.getEntity(idnum)
print "got:", e
#print dir(r)