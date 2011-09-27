"""
adds ec_material to texture refs that don't use a material (which was possible before 1.0, against opensim with lludp, as sl model has just texture assignments).
"""

import sys
import os
import xml.etree.ElementTree as ET
from xml.etree.ElementTree import ElementTree

sourcefile = sys.argv[1]
source = ElementTree()
source.parse(sourcefile)

def add_ecattr(comp, name, val):
    parm_el = ET.SubElement(comp, "attribute")
    parm_el.set("name", name)
    parm_el.set("value", val)

def create_texmaterial(ent, idx, mat):
    outname = mat.split('/')[-1] #filename from url - works for the Beneath the Waves web reffed txml now
    outname, _ = os.path.splitext(outname)
    outname += ".material"

    #EC_Material
    mat_el = ET.SubElement(ent, "component")
    mat_el.set("type", "EC_Material")
    mat_el.set("name", "material%d" % idx)
    mat_el.set("sync", "1")

    add_ecattr(mat_el, "Parameters", "texture = %s" % mat)
    add_ecattr(mat_el, "Input Material", "http://www.playsign.fi/rex/gallerywall/painting.material")
    add_ecattr(mat_el, "Output Material", outname)

    return outname
    

ents = source.getiterator("entity")
for ent in ents:
    for comp in ent:
        attrib = comp.attrib
        if attrib['type'] == 'EC_Mesh':
            mats_elem = comp.find(".//attribute[@name='%s']" % "Mesh materials")
            mats = mats_elem.get('value')
            #print attrib, mats

            if mats: #not an empty string
                mats = mats.split(';')
                outmats = []
                for idx, mat in enumerate(mats):
                    if not mat.endswith('.material'):
                        #print "texture ref found:", mat
                        outmat = create_texmaterial(ent, idx, mat)
                        outmats.append(outmat)

                if outmats:
                    mats_elem.set('value', ";".join(outmats))

            #else:
            #    print "Empty?", mats

ET.dump(source)
