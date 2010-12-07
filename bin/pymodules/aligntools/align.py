# functions to align selected objects with
#
#

import random
import rexviewer
from PythonQt.QtGui import QVector3D

# entities: list of selected entities to align
# axis: 'x', 'y', 'z'. Which axis to use for the transform
# first: True, False. True when first object position to use, False for last
#        object position. Irrelevant when spaced is True
# spaced: True, False. If True space on the specified axis between first and last
#         object.
def align_position(entities, axis, first, spaced):
    if len(entities) <= 1:
        return

    if not spaced:
        if first:
            orig_pos = entities[0].placeable.Position
        else:
            orig_pos = entities[-1].placeable.Position

        for entity in entities:
            try:
                oldpos = pos = entity.placeable.Position
                if axis=='x':
                    pos.setX(orig_pos.x())
                elif axis=='y':
                    pos.setY(orig_pos.y())
                elif axis=='z':
                    pos.setZ(orig_pos.z())
                entity.placeable.Position = pos
                try:
                    rexviewer.networkUpdate(entity.Id)
                except:
                    rexviewer.logDebug("align_position non spaced: couldn't update entity %d" % entity.Id)
            except: # entity didn't have EC_Placeable
                    rexviewer.logDebug("align_position non spaced: entity %d has no EC_Placeable" % entity.Id)
    else:
        count = len(entities)

        # no use to space with two or less.
        if count <= 2:
            return

        first = entities[0].placeable.Position
        last = entities[-1].placeable.Position
        # we make copies of the QVector3Ds. Otherwise they get directly changed
        if first < last:
            tmp = QVector3D(first)
            first = QVector3D(last)
            last = QVector3D(tmp)
        diff = QVector3D(first)
        diff = diff-last

        # determine the distribution between nodes
        if axis=='x':
            step = diff.x()/(count-1)
        elif axis=='y':
            step = diff.y()/(count-1)
        elif axis=='z':
            step = diff.z()/(count-1)

        # set the nodes (entities) to their respective position
        i=0
        for entity in entities[1:-1]:
            i+=1
            try:
                pos = entity.placeable.Position
                if axis=='x':
                    pos.setX(last.x()+step*i)
                if axis=='y':
                    pos.setY(last.y()+step*i)
                if axis=='z':
                    pos.setZ(last.z()+step*i)
                print entity.placeable.Position, pos, step*i
                entity.placeable.Position = pos
                try:
                    rexviewer.networkUpdate(entity.Id)
                except:
                    rexviewer.logDebug("align_position spaced: couldn't update entity %d" % entity.Id)
            except: # entity didn't have EC_Placeable
                    rexviewer.logDebug("align_position spaced: entity %d has no EC_Placeable" % entity.Id)

###############################################################################
##
## Tools
##
##

### X-axis ###

# aling selected objects on the x axis, according to first selected object
def align_on_x_first(entities):
    align_position(entities, 'x', True, False)

# aling selected objects on the x axis, according to last selected object
def align_on_x_last(entities):
    align_position(entities, 'x', False, False)

# distribute selected objects evenly on the x axis between the first and last selected objects
def align_on_x_spaced(entities):
    align_position(entities, 'x', True, True)

### Y-axis ###

# aling selected objects on the y axis, according to first selected object
def align_on_y_first(entities):
    align_position(entities, 'y', True, False)

# aling selected objects on the y axis, according to last selected object
def align_on_y_last(entities):
    align_position(entities, 'y', False, False)

# distribute selected objects evenly on the y axis between the first and last selected objects
def align_on_y_spaced(entities):
    align_position(entities, 'y', True, True)

### Z-axis ###

# aling selected objects on the z axis, according to first selected object
def align_on_z_first(entities):
    align_position(entities, 'z', True, False)

# aling selected objects on the z axis, according to last selected object
def align_on_z_last(entities):
    align_position(entities, 'z', False, False)

# distribute selected objects evenly on the z axis between the first and last selected objects
def align_on_z_spaced(entities):
    align_position(entities, 'z', True, True)

# get bounding box of selection, and the median of the positions
def _get_selection_bb(entities):
    xs = [e.placeable.Position.x() for e in entities]
    ys = [e.placeable.Position.y() for e in entities]
    zs = [e.placeable.Position.z() for e in entities]
            
    minpos = QVector3D(min(xs), min(ys), min(zs))
    maxpos = QVector3D(max(xs), max(ys), max(zs))
    median = (QVector3D(minpos) + maxpos) / 2 
    return minpos, maxpos, median

def align_random(entities):
    min, max, med = _get_selection_bb(entities)

    for entity in entities:
        x = random.uniform(min.x(),max.x())
        y = random.uniform(min.y(),max.y())
        z = random.uniform(min.z(),max.z())
        entity.placeable.Position = QVector3D(x, y, z)
        try:
            rexviewer.networkUpdate(entity.Id)
        except:
            rexviewer.logDebug("align_position random: couldn't update entity %d" % entity.Id)

def align_random_x(entities):
    min, max, med = _get_selection_bb(entities)

    for entity in entities:
        x = random.uniform(min.x(),max.x())
        y = entity.placeable.Position.y()
        z = entity.placeable.Position.z()
        entity.placeable.Position = QVector3D(x, y, z)
        try:
            rexviewer.networkUpdate(entity.Id)
        except:
            rexviewer.logDebug("align_position random z: couldn't update entity %d" % entity.Id)

def align_random_y(entities):
    min, max, med = _get_selection_bb(entities)

    for entity in entities:
        x = entity.placeable.Position.x()
        y = random.uniform(min.y(),max.y())
        z = entity.placeable.Position.z()
        entity.placeable.Position = QVector3D(x, y, z)
        try:
            rexviewer.networkUpdate(entity.Id)
        except:
            rexviewer.logDebug("align_position random y: couldn't update entity %d" % entity.Id)

def align_random_z(entities):
    min, max, med = _get_selection_bb(entities)

    for entity in entities:
        x = entity.placeable.Position.x()
        y = entity.placeable.Position.y()
        z = random.uniform(min.z(),max.z())
        entity.placeable.Position = QVector3D(x, y, z)
        try:
            rexviewer.networkUpdate(entity.Id)
        except:
            rexviewer.logDebug("align_position random z: couldn't update entity %d" % entity.Id)
