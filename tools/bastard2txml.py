"""
converts xml saved by 'opensim scene backup tool', in naali when used
with opensim/lludp, to tundra txml (pure naali EC attr style).

potentially useful for taiga->tundra migration, when the creators have
the original .mesh & .material files, but the scene is only in
opensim. 

USAGE: requires python 2.7 (which has elementtree 1.3 with good xpath support).
run with python, change SOURCEFILE below to your backup xml. e.g:
c:\python27\python.exe my.xml > my.txml
"""

import sys
import xml.etree.ElementTree as ET
from xml.etree.ElementTree import ElementTree
from math import atan2, asin, degrees #for quat2euler

sourcefile = sys.argv[1]
BASEURL = "http://www.realxtend.org/world/BeneathTheWaves/"

headxml = """<!DOCTYPE Scene>
<scene>
"""

meshxml = """
 <entity id="268">
  <component type="EC_Mesh" sync="1">
   <attribute value="0,0,0,90,0,180,1,1,1" name="Transform"/>
   <attribute value="%s" name="Mesh ref"/>
   <attribute value="%s" name="Mesh materials"/>
   <attribute value="%s" name="Skeleton ref"/>
   <attribute value="0" name="Draw distance"/>
   <attribute value="false" name="Cast shadows"/>
  </component>
  """
  
placeablexml = """
  <component type="EC_Placeable" sync="1">
   <attribute value="0 0 0" name="Position"/>
   <attribute value="1 1 1" name="Scale"/>
   <attribute value="%f,%f,%f,%f,%f,%f,%f,%f,%f" name="Transform"/>
   <attribute value="false" name="Show bounding box"/>
   <attribute value="true" name="Visible"/>
  </component>
 </entity>
 """
  
tailxml = """
</scene>
"""

#this is not really needed and is broken: doesn't deal with ; separated material lists right
#just convert urls with a normal string replace after this export
#def changeassetref(orgref):
#    if len(orgref) == 0:
#        return orgref #""
#
#    filename = s.split('/')[-1]
#    ref = BASEURL + filename
#    return ref

def entxml(mesh, material, skeleton, pos, ort, scale, scaletoprim):
    xml = meshxml % (mesh, material, skeleton)

    if scaletoprim: #not handled in viewer now, instead there's the 1/10 hack here, but this would allow handling in viewer
        xml += """<component type="EC_DynamicComponent" sync="1" name="ScaleToPrim">
  </component>"""

    rot = quat2euler(*ort)
    xml += placeablexml % (pos[0], pos[1], pos[2], rot[0], rot[1], rot[2], scale[0], scale[1], scale[2])
    return xml

source = ElementTree()
source.parse(sourcefile)

def attrval(el, attrname):
    attr = el.find(".//attribute[@name='%s']" % attrname)
    return attr.attrib['value']

def floatlist(el, attrname):
    return [float(v) for v in attrval(el, attrname).split(',')]

#ported from Quaternion::toEuler in Naali core
def clamp(value, minvalue, maxvalue):
    return max(minvalue, min(value, maxvalue))
def quat2euler(w, x, y, z):
    x = -x
    sqw = w*w;
    sqx = x*x;
    sqy = y*y;
    sqz = z*z;

    #heading = rotation about z-axis
    ez = atan2(2.0 * (x*y +z*w),(sqx - sqy - sqz + sqw))

    #bank = rotation about x-axis
    ex = atan2(2.0 * (y*z +x*w),(-sqx - sqy + sqz + sqw))

    #attitude = rotation about y-axis
    ey = asin(clamp(-2.0 * (x*z - y*w), -1.0, 1.0))

    return [degrees(v) for v in (ex, ey, ez)]

doc = headxml

ents = source.getiterator("entity")
for ent in ents:
    #digs these from the DC called RexPrimExportData
    mesh = None
    materials = skeleton = pos = ort = scale = ""
    scaletoprim = False
    for comp in ent:
        attrib = comp.attrib
        if attrib['type'] == 'EC_DynamicComponent' and attrib['name'] == 'RexPrimExportData':
            mesh = attrval(comp, 'MeshRef')
            materials = attrval(comp, 'Materials')
            skeleton = attrval(comp, 'SkeletonRef')
            
            #mesh, materials, skeleton = [changeassetref(s) for s in (mesh, materials, skeleton)]

            pos = floatlist(comp, 'Position')
            ort = floatlist(comp, 'Orientation')
            scale = floatlist(comp, 'Scale')
            #print mesh, ":", pos, ort, scale

            scaletoprimstr = attrval(comp, 'ScaleToPrim')
            if scaletoprimstr == "true":
                scaletoprim = True
                scale = [v/10 for v in scale] #XXX \todo can't handle here, would need to implement in viewer (is in primitive.cpp in rexlogic but not in anything in tundra, could be scripted

        else:
            doc += ET.tostring(comp)

    if mesh is not None: #the templates are now set so that these xml snippets must come last
        doc += entxml(mesh, materials, skeleton, pos, ort, scale, scaletoprim)

    else:
        #oh noes, how can bastartxml have entities without this component? perhaps non-placeable?
        doc += "</entity>"

doc += tailxml

print doc
