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
    return quat_mult_vec(entity.placeable.orientation, v)
    
# replacement for r.GetCameraRight()
def get_right(entity):
    v = QVector3D(1.0, 0.0, 0.0)
    return quat_mult_vec(entity.placeable.orientation, v)

def invert_3x3_matrix(m):
    """
    for matrix m:
    (a,b,c)
    (d,e,f)
    (g,h,k)    
    inverse m is given by:                
            (A,B,C)
    (1/Z) * (D,E,F)
            (G,H,K)
    A=(ek-fh) B=(ch-bk) C=(bf-ce)
    D=(fg-dk) E=(ak-cg) F=(cd-af)
    G=(dh-eg) H=(bg-ah) K=(ae-bd)
    Z = a(ek - fh) + b(fg - kd) + c(dh - eg)
    """
    a=m[0][0]
    b=m[0][1]
    c=m[0][2]
    d=m[1][0]
    e=m[1][1]
    f=m[1][2]
    g=m[2][0]
    h=m[2][1]
    k=m[2][2]
    A=e*k-f*h
    B=c*h-b*k
    C=b*f-c*e
    D=f*g-d*k
    E=a*k-c*g
    F=c*d-a*f
    G=d*h-e*g
    H=b*g-a*h
    K=a*e-b*d
    Z=a*(e*k - f*h) + b*(f*g - k*d) + c*(d*h - e*g)
    #print "Z %s"%str(Z)
    z=1.0/Z
    #print "z %s"%str(z)
    A*=z
    B*=z
    C*=z
    D*=z
    E*=z
    F*=z
    G*=z
    H*=z
    K*=z
    ret = ((A,B,C),(D,E,F),(G,H,K))
    return ret
