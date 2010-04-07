// For conditions of distribution and use, see copyright notice in license.txt
#include "XMLUtilities.h"

namespace RexTypes
{
    Real ParseReal(const std::string& text, Real default_value)
    {
        Real ret_value = default_value;
        try
        {
            ret_value = ParseString<Real>(text);
        }
        catch (boost::bad_lexical_cast)
        {
        }        
        return ret_value;
    }
    
    int ParseInt(const std::string& text, int default_value)
    {
        int ret_value = default_value;
        try
        {
            ret_value = ParseString<int>(text);
        }
        catch (boost::bad_lexical_cast)
        {
        }        
        return ret_value;
    }    
    
    Vector3df ParseVector3(const std::string& text)
    {
        Vector3df vec(0.0f, 0.0f, 0.0f);
        
        StringVector components = SplitString(text, ' ');
        if (components.size() == 3)
        {
            try
            {
                vec.x = ParseString<Real>(components[0]);
                vec.y = ParseString<Real>(components[1]);
                vec.z = ParseString<Real>(components[2]);
            }
            catch (boost::bad_lexical_cast)
            {
            }
        }
        return vec;
    }
    
    Quaternion ParseQuaternion(const std::string& text)
    {
        Quaternion quat;
        
        StringVector components = SplitString(text, ' ');
        if (components.size() == 4)
        {
            try
            {
                quat.w = ParseString<Real>(components[0]);
                quat.x = ParseString<Real>(components[1]);
                quat.y = ParseString<Real>(components[2]);
                quat.z = ParseString<Real>(components[3]);
            }
            catch (boost::bad_lexical_cast)
            {
            }
        }
        return quat;
    }
    
    Quaternion ParseEulerAngles(const std::string& text)
    {
        Quaternion quat;
        
        StringVector components = SplitString(text, ' ');
        if (components.size() == 3)
        {
            try
            {
                Real xrad = degToRad(ParseString<Real>(components[0]));
                Real yrad = degToRad(ParseString<Real>(components[1]));
                Real zrad = degToRad(ParseString<Real>(components[2]));
                
                Real angle = yrad * 0.5;
                double cx = cos(angle);
                double sx = sin(angle);

                angle = zrad * 0.5;
                double cy = cos(angle);
                double sy = sin(angle);

                angle = xrad * 0.5;
                double cz = cos(angle);
                double sz = sin(angle);

                quat.x = sx * sy * cz + cx * cy * sz;
                quat.y = sx * cy * cz + cx * sy * sz;
                quat.z = cx * sy * cz - sx * cy * sz;
                quat.w = cx * cy * cz - sx * sy * sz;
                
                quat.normalize();
            }
            catch (boost::bad_lexical_cast)
            {
            }
        }
        
        return quat;
    }
    
    std::string WriteBool(bool value)
    {
        if (value)
            return "true";
        else
            return "false";
    }
    
    std::string WriteVector3(const Vector3df& vector)
    {
        return ToString<Real>(vector.x) + " " +
            ToString<Real>(vector.y) + " " +
            ToString<Real>(vector.z);
    }
    
    std::string WriteQuaternion(const Quaternion& quat)
    {
        return ToString<Real>(quat.w) + " " +
            ToString<Real>(quat.x) + " " +
            ToString<Real>(quat.y) + " " +
            ToString<Real>(quat.z);
    }
    
    std::string WriteEulerAngles(const Quaternion& quat)
    {
        Vector3df radians;
        quat.toEuler(radians);
        
        return ToString<Real>(radians.x * RADTODEG) + " " +
            ToString<Real>(radians.y * RADTODEG) + " " + 
            ToString<Real>(radians.z * RADTODEG);
        
        //Ogre::Matrix3 rotMatrix;
        //Ogre::Quaternion value = OgreRenderer::ToOgreQuaternion(quat);
        //value.ToRotationMatrix(rotMatrix);
        //Ogre::Radian anglex;
        //Ogre::Radian angley;
        //Ogre::Radian anglez;
        //rotMatrix.ToEulerAnglesXYZ(anglex, angley, anglez);

        //Real angles[3];
        //angles[0] = anglex.valueDegrees();
        //angles[1] = angley.valueDegrees();
        //angles[2] = anglez.valueDegrees();
        //
        //return ToString<Real>(angles[0]) + " " +
        //    ToString<Real>(angles[1]) + " " + 
        //    ToString<Real>(angles[2]);
    }

    void SetAttribute(QDomElement& elem, const std::string& name, const char* value)
    {
        elem.setAttribute(QString::fromStdString(name), value);
    }
        
    void SetAttribute(QDomElement& elem, const std::string& name, const std::string& value)
    {
        elem.setAttribute(QString::fromStdString(name), QString::fromStdString(value));
    }
     
    void SetAttribute(QDomElement& elem, const std::string& name, Real value)
    {
        elem.setAttribute(QString::fromStdString(name), QString::fromStdString(ToString<Real>(value)));
    }
    
    void SetAttribute(QDomElement& elem, const std::string& name, bool value)
    {
        elem.setAttribute(QString::fromStdString(name), QString::fromStdString(WriteBool(value)));
    }
    
    void SetAttribute(QDomElement& elem, const std::string& name, int value)
    {
        elem.setAttribute(QString::fromStdString(name), QString::fromStdString(ToString<int>(value)));
    }
}

