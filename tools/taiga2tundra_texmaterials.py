"""
adds ec_material to texture refs that don't use a material (which was possible before 1.0, against opensim with lludp, as sl model has just texture assignments).
"""

import sys
import xml.etree.ElementTree as ET
from xml.etree.ElementTree import ElementTree

sourcefile = sys.argv[1]
source = ElementTree()
source.parse(sourcefile)

ents = source.getiterator("entity")
for ent in ents:
    for comp in ent:
        attrib = comp.attrib
        if attrib['type'] == 'EC_Mesh':
            mats_elem = comp.find(".//attribute[@name='%s']" % "Mesh materials")
            mats = mats_elem.attrib['value']
            print attrib, mats

            if not mats.endswith('.material'):
                print "texture ref found:",
                print mats

            else:
                print "MATERIAL:", mats
