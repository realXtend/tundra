/**
    For conditions of distribution and use, see copyright notice in LICENSE

    @file   Transform.cpp
    @brief  Describes transformation of an object in 3D space. */

#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#include "Transform.h"

#include "Math/MathFunc.h"

#include <stdio.h>
#include <stdlib.h>

#include "MemoryLeakCheck.h"

Transform::operator QString() const
{
    return "Transform(Pos:(" + QString::number(pos.x) + "," + QString::number(pos.y) + "," + QString::number(pos.z) + ") Rot:(" +
        QString::number(rot.x) + "," + QString::number(rot.y) + "," + QString::number(rot.z) + " Scale:(" +
        QString::number(scale.x) + "," + QString::number(scale.y) + "," + QString::number(scale.z) + "))";
}

QString Transform::SerializeToString() const
{
    char str[256];
    sprintf(str, "%f,%f,%f,%f,%f,%f,%f,%f,%f", pos.x, pos.y, pos.z, rot.x, rot.y, rot.z, scale.x, scale.y, scale.z);
    return str;
}

Transform Transform::FromString(const char *str)
{
    assume(str);
    if (!str)
        return Transform();
    if (*str == '(')
        ++str;
    Transform t;
    t.pos.x = (float)strtod(str, const_cast<char**>(&str));
    if (*str == ',' || *str == ';')
        ++str;
    t.pos.y = (float)strtod(str, const_cast<char**>(&str));
    if (*str == ',' || *str == ';')
        ++str;
    t.pos.z = (float)strtod(str, const_cast<char**>(&str));
    if (*str == ',' || *str == ';')
        ++str;
    t.rot.x = (float)strtod(str, const_cast<char**>(&str));
    if (*str == ',' || *str == ';')
        ++str;
    t.rot.y = (float)strtod(str, const_cast<char**>(&str));
    if (*str == ',' || *str == ';')
        ++str;
    t.rot.z = (float)strtod(str, const_cast<char**>(&str));
    if (*str == ',' || *str == ';')
        ++str;
    t.scale.x = (float)strtod(str, const_cast<char**>(&str));
    if (*str == ',' || *str == ';')
        ++str;
    t.scale.y = (float)strtod(str, const_cast<char**>(&str));
    if (*str == ',' || *str == ';')
        ++str;
    t.scale.z = (float)strtod(str, const_cast<char**>(&str));
    if (*str == ',' || *str == ';')
        ++str;
    return t;
}
