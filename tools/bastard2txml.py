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

sourcefile = sys.argv[1]

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
   <attribute value="%f,%f,%f,0,0,0,1,1,1" name="Transform"/>
   <attribute value="false" name="Show bounding box"/>
   <attribute value="true" name="Visible"/>
  </component>
 </entity>
 """
  
tailxml = """
</scene>
"""

def entxml(mesh, material, skeleton, pos, ort, scale):
    xml = meshxml % (mesh, material, skeleton) 
    xml += placeablexml % (pos[0], pos[1], pos[2]) 
    return xml

source = ElementTree()
source.parse(sourcefile)

def attrval(el, attrname):
    attr = el.find(".//attribute[@name='%s']" % attrname)
    return attr.attrib['value']

def floatlist(el, attrname):
    return [float(v) for v in attrval(el, attrname).split(',')]

doc = headxml

ents = source.getiterator("entity")
for ent in ents:
    #digs these from the DC called RexPrimExportData
    mesh = None
    materials = skeleton = pos = ort = scale = ""
    for comp in ent:
        attrib = comp.attrib
        if attrib['type'] == 'EC_DynamicComponent' and attrib['name'] == 'RexPrimExportData':
            mesh = attrval(comp, 'MeshRef')
            #materials
            #skeleton

            pos = floatlist(comp, 'Position')
            ort = floatlist(comp, 'Orientation')
            scale = floatlist(comp, 'Scale')
            #print mesh, ":", pos, ort, scale

        else:
            doc += ET.tostring(comp)

    if mesh is not None: #the templates are now set so that these xml snippets must come last
        doc += entxml(mesh, materials, skeleton, pos, ort, scale)

    else:
        #oh noes, how can bastartxml have entities without this component? perhaps non-placeable?
        doc += "</entity>"

doc += tailxml

print doc
