// For conditions of distribution and use, see copyright notice in license.txt
#ifndef incl_XMLUtilities_h
#define incl_XMLUtilities_h

#include "Core.h"

class QDomElement;

namespace RexTypes
{
    float ParseReal(const std::string& text, float default_value = 0.0f);
    int ParseInt(const std::string& text, int default_value = 0);
    Color ParseColor(const std::string& text);
    Vector3df ParseVector3(const std::string& text);
    Quaternion ParseQuaternion(const std::string& text);
    Quaternion ParseEulerAngles(const std::string& text);
    
    std::string WriteBool(bool value);
    std::string WriteReal(float value);
    std::string WriteInt(int value);
    std::string WriteVector3(const Vector3df& vector);
    std::string WriteColor(const Color& color);
    std::string WriteQuaternion(const Quaternion& quat);
    std::string WriteEulerAngles(const Quaternion& quat);

    void SetAttribute(QDomElement& elem, const std::string& name, const char* value);
    void SetAttribute(QDomElement& elem, const std::string& name, const std::string& value);
    void SetAttribute(QDomElement& elem, const std::string& name, float value);
    void SetAttribute(QDomElement& elem, const std::string& name, bool value);
    void SetAttribute(QDomElement& elem, const std::string& name, int value);
}

#endif

