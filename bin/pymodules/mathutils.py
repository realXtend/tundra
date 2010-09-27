# Math utils for Naali
import math
import PythonQt.QtGui
from PythonQt.QtGui import QQuaternion
from PythonQt.QtGui import QVector3D

# some handy shortcut rotations for quats
ninty_around_x = QQuaternion(math.sqrt(0.5), math.sqrt(0.5), 0, 0)
ninty_around_y = QQuaternion(math.sqrt(0.5), 0, math.sqrt(0.5), 0)
ninty_around_z = QQuaternion(math.sqrt(0.5), 0, 0, math.sqrt(0.5))

# To be used with Qt QQuaternion and QVector3D
# according Quaternion*Vector from irrlicht (see Core\Quaternion.h)
def quat_mult_vec(quat, v):
    qvec = quat.vector()
    uv = QVector3D.crossProduct(qvec, v)
    uuv = QVector3D.crossProduct(qvec, uv)
    uv = uv * 2.0 * quat.scalar()
    uuv = uuv * 2.0

    return v + uv + uuv

# QQuaternion to euler [x,y,z]
def quat_to_euler(quat):
    euler = [0, 0, 0]
    sqw = quat.scalar() * quat.scalar()
    sqx = quat.x() * quat.x()
    sqy = quat.y() * quat.y()
    sqz = quat.z() * quat.z()
    
    euler[2] = math.atan2(2.0 * (quat.x()*quat.y() +quat.z()*quat.scalar()),(sqx - sqy - sqz + sqw))
    euler[0] = math.atan2(2.0 * (quat.y()*quat.z() +quat.x()*quat.scalar()),(-sqx - sqy + sqz + sqw))
    yval = math.asin(-2.0 * (quat.x()*quat.z() - quat.y()*quat.scalar()))
    if yval < -1.0:
        yval = -1.0
    elif yval > 1.0:
        yval = 1.0
    euler[1] = yval
    
    return euler

# euler [x,y,z] to QQuaternion
def euler_to_quat(euler):
    ang = euler[0] * 0.5
    sr = math.sin(ang)
    cr = math.cos(ang)

    ang = euler[1] * 0.5
    sp = math.sin(ang)
    cp = math.cos(ang)

    ang = euler[2] * 0.5
    sy = math.sin(ang)
    cy = math.cos(ang)

    cpcy = cp * cy
    spcy = sp * cy
    cpsy = cp * sy
    spsy = sp * sy

    quat = QQuaternion(cr*cpcy + sr*spsy, sr * cpcy - cr*spsy, cr*spcy + sr * cpsy, cr * cpsy - sr * spcy)
    quat.normalize()
    return quat

# replacement for r.GetCameraUp()
def get_up(entity):
    v = QVector3D(0.0, 1.0, 0.0)
    return quat_mult_vec(entity.placeable.Orientation, v)
    
# replacement for r.GetCameraRight()
def get_right(entity):
    v = QVector3D(1.0, 0.0, 0.0)
    return quat_mult_vec(entity.placeable.Orientation, v)
