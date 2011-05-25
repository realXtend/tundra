/** @file
    @author Jukka Jylänki

    This work is copyrighted material and may NOT be used for any kind of commercial or 
    personal advantage and may NOT be copied or redistributed without prior consent
    of the author(s). 
*/
#include "StableHeaders.h"
#include "Math/TransformOps.h"
#include "Math/MathFunc.h"
#include "Math/float4.h"
#include "Math/float3x4.h"
#include "Math/float4x4.h"

TranslateOp::TranslateOp(float tx, float ty, float tz)
:x(tx), y(ty), z(tz)
{
}

TranslateOp::TranslateOp(const float3 &trans)
:x(trans.x), y(trans.y), z(trans.z)
{
}

TranslateOp::operator float3x4() const
{
    float3x4 m;
    m.SetRow(0, 1, 0, 0, x);
    m.SetRow(1, 0, 1, 0, y);
    m.SetRow(2, 0, 0, 1, z);
    return m;
}

TranslateOp::operator float4x4() const
{
    float4x4 m;
    m.SetRow(0, 1, 0, 0, x);
    m.SetRow(1, 0, 1, 0, y);
    m.SetRow(2, 0, 0, 1, z);
    m.SetRow(3, 0, 0, 0, 1.f);
    return m;
}

float3 TranslateOp::Offset() const
{
    return float3(x, y, z);
}

float3x4 operator *(const TranslateOp &lhs, const float3x4 &rhs)
{
    float3x4 r = rhs;
    r.SetTranslatePart(r.TranslatePart() + lhs.Offset());
    // Our optimized form of multiplication must be the same as this.
    assert(r.Equals((float3x4)lhs * rhs));
    return r;
}

float3x4 operator *(const float3x4 &lhs, const TranslateOp &rhs)
{
    float3x4 r = lhs;
    r.SetTranslatePart(lhs.TransformPoint(rhs.Offset()));

    // Our optimized form of multiplication must be the same as this.
    assert(r.Equals(lhs * (float3x4)rhs));
    return r;
}

float4x4 operator *(const TranslateOp &lhs, const float4x4 &rhs)
{
    float4x4 r = rhs;
    r.SetTranslatePart(r.TranslatePart() + lhs.Offset());

    // Our optimized form of multiplication must be the same as this.
    assert(r.Equals((float4x4)lhs * rhs));
    return r;
}

float4x4 operator *(const float4x4 &lhs, const TranslateOp &rhs)
{
    float4x4 r = lhs;
    r.SetTranslatePart(lhs.TransformPoint(rhs.Offset()));

    // Our optimized form of multiplication must be the same as this.
    assert(r.Equals(lhs * (float4x4)rhs));
    return r;
}

ScaleOp::ScaleOp(float sx, float sy, float sz)
:x(sx), y(sy), z(sz)
{
}

ScaleOp::ScaleOp(const float3 &scale)
:x(scale.x), y(scale.y), z(scale.z)
{
}

ScaleOp::operator float3x3() const
{
    float3x3 m;
    m.SetRow(0, x, 0, 0);
    m.SetRow(1, 0, y, 0);
    m.SetRow(2, 0, 0, z);
    return m;
}

ScaleOp::operator float3x4() const
{
    float3x4 m;
    m.SetRow(0, x, 0, 0, 0);
    m.SetRow(1, 0, y, 0, 0);
    m.SetRow(2, 0, 0, z, 0);
    return m;
}

ScaleOp::operator float4x4() const
{
    float4x4 m;
    m.SetRow(0, x, 0, 0, 0);
    m.SetRow(1, 0, y, 0, 0);
    m.SetRow(2, 0, 0, z, 0);
    m.SetRow(3, 0, 0, 0, 1.f);
    return m;
}

float3x3 operator *(const ScaleOp &lhs, const float3x3 &rhs)
{
    float3x3 ret = rhs;
    ret.ScaleRow(0, lhs.x);
    ret.ScaleRow(1, lhs.y);
    ret.ScaleRow(2, lhs.z);

    // Our optimized form of multiplication must be the same as this.
    assert(ret.Equals((float3x3)lhs * rhs));
    return ret;
}

float3x3 operator *(const float3x3 &lhs, const ScaleOp &rhs)
{
    float3x3 ret = lhs;
    ret.ScaleCol(0, rhs.x);
    ret.ScaleCol(1, rhs.y);
    ret.ScaleCol(2, rhs.z);

    // Our optimized form of multiplication must be the same as this.
    assert(ret.Equals(lhs * (float3x3)rhs));
    return ret;
}

float3x4 operator *(const ScaleOp &lhs, const float3x4 &rhs)
{
    ///\todo Optimize.
    return (float3x4)lhs * rhs;
}

float3x4 operator *(const float3x4 &lhs, const ScaleOp &rhs)
{
    ///\todo Optimize.
    return lhs * (float3x4)rhs;
}

float4x4 operator *(const ScaleOp &lhs, const float4x4 &rhs)
{
    ///\todo Optimize.
    return (float4x4)lhs * rhs;
}

float4x4 operator *(const float4x4 &lhs, const ScaleOp &rhs)
{
    ///\todo Optimize.
    return lhs * (float4x4)rhs;
}

float3 ScaleOp::Offset() const
{
    return float3(x, y, z);
}
