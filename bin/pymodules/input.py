import rexviewer as r

#some prim
idnum = 720001
#idnum = 0

#av ent
av_entid = 8880005

idnum = av_entid

print "Getting entity id", idnum,
e = r.getEntity(idnum)
print "got:", e
#print dir(r)

if 1: #test avatar tracking, works :)
    a = r.getEntity(av_entid)
    print "Avatar pos:", a.place
    """
    perhaps some local script could track movement?
    make a sound of a nearby object of interest, 
    like when a pet or a friend moves?
    """