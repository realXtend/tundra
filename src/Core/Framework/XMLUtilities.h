// For conditions of distribution and use, see copyright notice in LICENSE
#pragma once

#include "CoreDefines.h"
#include "Color.h"
#include "Math/float3.h"
#include "Math/Quat.h"

#include <string>

class QDomElement;

float ParseReal(const std::string& text, float default_value = 0.0f);
int ParseInt(const std::string& text, int default_value = 0);
Color ParseColor(const std::string& text);
//float3 ParseVector3(const std::string& text);

std::string WriteBool(bool value);
std::string WriteReal(float value);
std::string WriteInt(int value);
//std::string WriteVector3(const float3& vector);
std::string WriteColor(const Color& color);

void SetAttribute(QDomElement& elem, const std::string& name, const char* value);
void SetAttribute(QDomElement& elem, const std::string& name, const std::string& value);
void SetAttribute(QDomElement& elem, const std::string& name, float value);
void SetAttribute(QDomElement& elem, const std::string& name, bool value);
void SetAttribute(QDomElement& elem, const std::string& name, int value);


