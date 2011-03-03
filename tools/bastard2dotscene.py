"""
converts xml saved by 'opensim scene backup tool', in naali when used
with opensim/lludp, to tundra txml (pure naali EC attr style).

potentially useful for taiga->tundra migration, when the creators have
the original .mesh & .material files, but the scene is only in
opensim. 

USAGE: requires python 2.7 (which has elementtree 1.3 with good xpath support).
run with python, change SOURCEFILE below to your backup xml. e.g:
c:\python27\python.exe my.xml > my.scene

directly to txml would be more involved 'cause would need to deal with material & texture UUID refs etc.
"""

from xml.etree.ElementTree import ElementTree
#import md5 #could add asset ref -> cache id conversion here trially
SOURCEFILE = "cecil.bastardxml"

head = """
<scene formatVersion="">
    <nodes>
"""

node = """
        <node name="%s" id="%d">
            <position x="%f" y="%f" z="%f" />
            <rotation qx="%f" qy="%f" qz="%f" qw="%f" />
            <scale x="%f" y="%f" z="%f" />
            <entity name="%s" meshFile="%s" static="false" />
        </node>
"""

tail = """
    </nodes>
</scene>
"""

#data = ("Jack", 1, 
#        0.0, 0.0, 0.0,
#        0.0, 0.0, 0.0, 0.0,
#        1.0, 1.0, 1.0,
#        "Jack", "Jack.mesh")

doc = head

source = ElementTree()
source.parse(SOURCEFILE)

def attrval(el, attrname):
    attr = el.find(".//attribute[@name='%s']" % attrname)
    return attr.attrib['value']

def floatlist(el, attrname):
    return [float(v) for v in attrval(el, attrname).split(',')]

ents = source.getiterator("entity")
for idx, ent in enumerate(ents):
    c = ent.find('component')
    #print c.attrib
    if c.attrib['type'] == 'EC_DynamicComponent':
        if c.attrib['name'] == 'RexPrimExportData':
            mesh = attrval(c, 'MeshRef')

            pos = floatlist(c, 'Position')
            ort = floatlist(c, 'Orientation')
            scale = floatlist(c, 'Scale')
            #print mesh, ":", pos, ort, scale

            doc += node % (mesh, idx,
                           pos[0], pos[1], pos[2],
                           ort[0], ort[1], ort[2], ort[3],
                           scale[0], scale[1], scale[2],
                           mesh, mesh + ".Mesh")
    
doc += tail

print doc
