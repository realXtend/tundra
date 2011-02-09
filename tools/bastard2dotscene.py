from xml.etree.ElementTree import ElementTree

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
