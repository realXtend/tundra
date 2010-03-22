import math
degrees2radians = 0.0174532925
radians2degrees = 57.2957795

def euler_to_quat(euler):
    #let's assume that the euler has the yaw,pitch and roll and they are in degrees, changing to radians
    yaw = euler[0] * degrees2radians
    pitch = euler[1] * degrees2radians
    roll = euler[2] * degrees2radians
    
    c1 = math.cos(yaw/2)
    c2 = math.cos(pitch/2)
    c3 = math.cos(roll/2)
    s1 = math.sin(yaw/2)
    s2 = math.sin(pitch/2)
    s3 = math.sin(roll/2)
    #print c1, c2, c3, s1, s2, s3
    c1c2 = c1*c2
    s1s2 = s1*s2
    w =c1c2*c3 - s1s2*s3
    x =c1c2*s3 + s1s2*c3
    y =s1*c2*c3 + c1*s2*s3
    z =c1*s2*c3 - s1*c2*s3
    
    return (x, y, z, w)

def quat_to_euler(quat):
    q1 = Quat(quat)
    sqw = q1.w*q1.w
    sqx = q1.x*q1.x
    sqy = q1.y*q1.y
    sqz = q1.z*q1.z
    unit = sqx + sqy + sqz + sqw # if normalised is one, otherwise is correction factor
    test = q1.x*q1.y + q1.z*q1.w
    if (test > 0.499*unit): # singularity at north pole
        yaw = 2 * math.atan2(q1.x,q1.w)
        pitch = math.pi/2
        roll = 0
        return convert_to_degrees(yaw, pitch, roll)
        
    if (test < -0.499*unit): # singularity at south pole
        yaw = -2 * math.atan2(q1.x,q1.w)
        pitch = -math.pi/2
        roll = 0
        return convert_to_degrees(yaw, pitch, roll)
        
    yaw = math.atan2(2*q1.y*q1.w-2*q1.x*q1.z , sqx - sqy - sqz + sqw)
    pitch = math.asin(2*test/unit)
    roll = math.atan2(2*q1.x*q1.w-2*q1.y*q1.z , -sqx + sqy - sqz + sqw)
    
    return convert_to_degrees(yaw, pitch, roll)

def convert_to_degrees(yaw, pitch, roll):
    yaw *= radians2degrees
    pitch *= radians2degrees
    roll *= radians2degrees
    
    return math.ceil(yaw), math.ceil(pitch), math.ceil(roll)
    
    
class Quat:
    def __init__(self, quat):
        self.x = quat[0]
        self.y = quat[1]
        self.z = quat[2]
        self.w = quat[3]
        
if __name__ == '__main__': 
    x = 0 #yaw / heading
    y = 0 #pitch / attitude 
    z = 90 #roll / bank
    euler = (x, y, z)
    quat = euler_to_quat(euler)
    print "Quat is:", quat
    
    euler = quat_to_euler(quat)
    print "Euler is:", euler