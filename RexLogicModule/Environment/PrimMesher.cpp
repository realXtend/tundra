// For conditions of distribution and use, see copyright notice in license.txt
// Based on PrimMesher project, converted from C# to C++

/*
 * Copyright (c) Contributors
 * See CONTRIBUTORS.TXT for a full list of copyright holders.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the OpenSim Project nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE DEVELOPERS ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*
   CONTRIBUTORS.TXT

   The primary developer of PrimMesher is Dahlia Trimble.

   Some portions of PrimMesher are from the following projects:
   * OpenSimulator (original extrusion concept)
   * LibOpenMetaverse (quaternion multiplication routine)
 */

#include "StableHeaders.h"

#include "Environment/PrimMesher.h"
#include "CoreMath.h"
#include "CoreException.h"

namespace PrimMesher
{

    Angle angles3[] =
    {
        Angle(0.0f, 1.0f, 0.0f),
        Angle(0.33333333333333333f, -0.5f, 0.86602540378443871f),
        Angle(0.66666666666666667f, -0.5f, -0.86602540378443837f),
        Angle(1.0f, 1.0f, 0.0f)
    };
    
    Coord normals3[] =
    {
        Coord(0.25f, 0.4330127019f, 0.0f).Normalize(),
        Coord(-0.5f, 0.0f, 0.0f).Normalize(),
        Coord(0.25f, -0.4330127019f, 0.0f).Normalize(),
        Coord(0.25f, 0.4330127019f, 0.0f).Normalize()
    };

    Angle angles4[] =
    {
        Angle(0.0f, 1.0f, 0.0f),
        Angle(0.25f, 0.0f, 1.0f),
        Angle(0.5f, -1.0f, 0.0f),
        Angle(0.75f, 0.0f, -1.0f),
        Angle(1.0f, 1.0f, 0.0f)
    };

    Coord normals4[] = 
    {
        Coord(0.5f, 0.5f, 0.0f).Normalize(),
        Coord(-0.5f, 0.5f, 0.0f).Normalize(),
        Coord(-0.5f, -0.5f, 0.0f).Normalize(),
        Coord(0.5f, -0.5f, 0.0f).Normalize(),
        Coord(0.5f, 0.5f, 0.0f).Normalize()
    };

    Angle angles24[] =
    {
        Angle(0.0f, 1.0f, 0.0f),
        Angle(0.041666666666666664f, 0.96592582628906831f, 0.25881904510252074f),
        Angle(0.083333333333333329f, 0.86602540378443871f, 0.5f),
        Angle(0.125f, 0.70710678118654757f, 0.70710678118654746f),
        Angle(0.16666666666666667f, 0.5f, 0.8660254037844386f),
        Angle(0.20833333333333331f, 0.25881904510252096f, 0.9659258262890682f),
        Angle(0.25f, 0.0f, 1.0f),
        Angle(0.29166666666666663f, -0.25881904510252063f, 0.96592582628906831f),
        Angle(0.33333333333333333f, -0.5f, 0.86602540378443871f),
        Angle(0.375f, -0.70710678118654746f, 0.70710678118654757f),
        Angle(0.41666666666666663f, -0.86602540378443849f, 0.5f),
        Angle(0.45833333333333331f, -0.9659258262890682f, 0.25881904510252102f),
        Angle(0.5f, -1.0f, 0.0f),
        Angle(0.54166666666666663f, -0.96592582628906842f, -0.25881904510252035f),
        Angle(0.58333333333333326f, -0.86602540378443882f, -0.5f),
        Angle(0.62499999999999989f, -0.70710678118654791f, -0.70710678118654713f),
        Angle(0.66666666666666667f, -0.5f, -0.86602540378443837f),
        Angle(0.70833333333333326f, -0.25881904510252152f, -0.96592582628906809f),
        Angle(0.75f, 0.0f, -1.0f),
        Angle(0.79166666666666663f, 0.2588190451025203f, -0.96592582628906842f),
        Angle(0.83333333333333326f, 0.5f, -0.86602540378443904f),
        Angle(0.875f, 0.70710678118654735f, -0.70710678118654768f),
        Angle(0.91666666666666663f, 0.86602540378443837f, -0.5f),
        Angle(0.95833333333333326f, 0.96592582628906809f, -0.25881904510252157f),
        Angle(1.0f, 1.0f, 0.0f)
    };

    static const float twoPi = 2.0f * (float)PI;

    Quat::Quat()
    {
        X = 0.0f;
        Y = 0.0f;
        Z = 0.0f;
        W = 1.0f;
    }

    Quat::Quat(float x, float y, float z, float w)
    {
        X = x;
        Y = y;
        Z = z;
        W = w;
    }

    Quat::Quat(const Coord& axis, float angle)
    {
        Coord axisNorm = axis;
        axisNorm.Normalize();

        angle *= 0.5f;
        float c = (float)cos(angle);
        float s = (float)sin(angle);

        X = axisNorm.X * s;
        Y = axisNorm.Y * s;
        Z = axisNorm.Z * s;
        W = c;

        Normalize();
    }

    float Quat::Length()
    {
        return (float)sqrt(X * X + Y * Y + Z * Z + W * W);
    }

    Quat& Quat::Normalize()
    {
        const float MAG_THRESHOLD = 0.0000001f;
        float mag = Length();

        // Catch very small rounding errors when normalizing
        if (mag > MAG_THRESHOLD)
        {
            float oomag = 1.0f / mag;
            X *= oomag;
            Y *= oomag;
            Z *= oomag;
            W *= oomag;
        }
        else
        {
            X = 0.0f;
            Y = 0.0f;
            Z = 0.0f;
            W = 1.0f;
        }

        return *this;
    }

    Coord::Coord()
    {
        X = 0.0f;
        Y = 0.0f;
        Z = 0.0f;
    }

    Coord::Coord(float x, float y, float z)
    {
        X = x;
        Y = y;
        Z = z;
    }

    float Coord::Length()
    {
        return (float)sqrt(X * X + Y * Y + Z * Z);
    }

    Coord& Coord::Invert()
    {
        X = -X;
        Y = -Y;
        Z = -Z;

        return *this;
    }

    Coord& Coord::Normalize()
    {
        const float MAG_THRESHOLD = 0.0000001f;
        float mag = Length();

        // Catch very small rounding errors when normalizing
        if (mag > MAG_THRESHOLD)
        {
            float oomag = 1.0f / mag;
            X *= oomag;
            Y *= oomag;
            Z *= oomag;
        }
        else
        {
            X = 0.0f;
            Y = 0.0f;
            Z = 0.0f;
        }

        return *this;
    }

    Coord Coord::Cross(const Coord& c1, const Coord& c2)
    {
        return Coord(
            c1.Y * c2.Z - c2.Y * c1.Z,
            c1.Z * c2.X - c2.Z * c1.X,
            c1.X * c2.Y - c2.X * c1.Y
            );
    }

    UVCoord::UVCoord()
    {
        U = 0.0f;
        V = 0.0f;
    }

    UVCoord::UVCoord(float u, float v)
    {
        U = u;
        V = v;
    }
    
    UVCoord UVCoord::Flip()
    {
        return UVCoord(1.0f - U, 1.0f - V);
    }

    Face::Face()
    {
        primFace = 0;

        v1 = 0;
        v2 = 0;
        v3 = 0;

        n1 = 0;
        n2 = 0;
        n3 = 0;

        uv1 = 0;
        uv2 = 0;
        uv3 = 0;

    }

    Face::Face(int pv1, int pv2, int pv3)
    {
        primFace = 0;

        v1 = pv1;
        v2 = pv2;
        v3 = pv3;

        n1 = 0;
        n2 = 0;
        n3 = 0;

        uv1 = 0;
        uv2 = 0;
        uv3 = 0;

    }

    Face::Face(int pv1, int pv2, int pv3, int pn1, int pn2, int pn3)
    {
        primFace = 0;

        v1 = pv1;
        v2 = pv2;
        v3 = pv3;

        n1 = pn1;
        n2 = pn2;
        n3 = pn3;

        uv1 = 0;
        uv2 = 0;
        uv3 = 0;
    }

    Coord Face::SurfaceNormal(const std::vector<Coord>& coordList)
    {
        const Coord& c1 = coordList[v1];
        const Coord& c2 = coordList[v2];
        const Coord& c3 = coordList[v3];

        Coord edge1 = Coord(c2.X - c1.X, c2.Y - c1.Y, c2.Z - c1.Z);
        Coord edge2 = Coord(c3.X - c1.X, c3.Y - c1.Y, c3.Z - c1.Z);

        return Coord::Cross(edge1, edge2).Normalize();
    }

    ViewerFace::ViewerFace()
    {
        primFaceNumber = 0;
    }

    ViewerFace::ViewerFace(int pprimFaceNumber)
    {
        primFaceNumber = pprimFaceNumber;
    }

    void ViewerFace::Scale(float x, float y, float z)
    {
        v1.X *= x;
        v1.Y *= y;
        v1.Z *= z;

        v2.X *= x;
        v2.Y *= y;
        v2.Z *= z;

        v3.X *= x;
        v3.Y *= y;
        v3.Z *= z;
    }

    void ViewerFace::AddRot(const Quat& q)
    {
        v1 *= q;
        v2 *= q;
        v3 *= q;

        n1 *= q;
        n2 *= q;
        n3 *= q;
    }

    void ViewerFace::CalcSurfaceNormal()
    {
        Coord edge1(v2.X - v1.X, v2.Y - v1.Y, v2.Z - v1.Z);
        Coord edge2(v3.X - v1.X, v3.Y - v1.Y, v3.Z - v1.Z);

        n1 = n2 = n3 = Coord::Cross(edge1, edge2).Normalize();
    }

    Angle::Angle()
    {
        angle = 0;
        X = 0;
        Y = 0;
    }

    Angle::Angle(float pangle, float px, float py)
    {
        angle = pangle;
        X = px;
        Y = py;
    }

    Angle AngleList::interpolatePoints(float newPoint, const Angle& p1, const Angle& p2)
    {
        float m = (newPoint - p1.angle) / (p2.angle - p1.angle);
        return Angle(newPoint, p1.X + m * (p2.X - p1.X), p1.Y + m * (p2.Y - p1.Y));
    }

    void AngleList::intersection(double x1, double y1, double x2, double y2, double x3, double y3, double x4, double y4)
    { // ref: http://local.wasp.uwa.edu.au/~pbourke/geometry/lineline2d/
        double denom = (y4 - y3) * (x2 - x1) - (x4 - x3) * (y2 - y1);
        double uaNumerator = (x4 - x3) * (y1 - y3) - (y4 - y3) * (x1 - x3);

        if (denom != 0.0)
        {
            double ua = uaNumerator / denom;
            iX = (float)(x1 + ua * (x2 - x1));
            iY = (float)(y1 + ua * (y2 - y1));
        }
    }

    void AngleList::makeAngles(int sides, float startAngle, float stopAngle)
    {
        angles.clear();
        normals.clear();

        double twoPi = PI * 2.0;
        float twoPiInv = 1.0f / (float)twoPi;

        if (sides < 1)
            throw Exception("number of sides not greater than zero");
        if (stopAngle <= startAngle)
            throw Exception("stopAngle not greater than startAngle");

        if ((sides == 3 || sides == 4 || sides == 24))
        {
            startAngle *= twoPiInv;
            stopAngle *= twoPiInv;

            Angle* sourceAngles;
            int endAngleIndex;
            if (sides == 3)
            {
                sourceAngles = angles3;
                endAngleIndex = 3;
            }
            else if (sides == 4)
            {
                sourceAngles = angles4;
                endAngleIndex = 4;
            }
            else
            {
                sourceAngles = angles24;
                endAngleIndex = 24;
            }

            int startAngleIndex = (int)(startAngle * sides);

            if (stopAngle < 1.0f)
                endAngleIndex = (int)(stopAngle * sides) + 1;
            if (endAngleIndex == startAngleIndex)
                endAngleIndex++;

            for (int angleIndex = startAngleIndex; angleIndex < endAngleIndex + 1; angleIndex++)
            {
                angles.push_back(sourceAngles[angleIndex]);
                if (sides == 3)
                    normals.push_back(normals3[angleIndex]);
                else if (sides == 4)
                    normals.push_back(normals4[angleIndex]);
            }

            if (startAngle > 0.0f)
                angles[0] = interpolatePoints(startAngle, angles[0], angles[1]);

            if (stopAngle < 1.0f)
            {
                int lastAngleIndex = angles.size() - 1;
                angles[lastAngleIndex] = interpolatePoints(stopAngle, angles[lastAngleIndex - 1], angles[lastAngleIndex]);
            }
        }
        else
        {
            double stepSize = twoPi / sides;

            int startStep = (int)(startAngle / stepSize);
            double angle = stepSize * startStep;
            int step = startStep;
            double stopAngleTest = stopAngle;
            if (stopAngle < twoPi)
            {
                stopAngleTest = stepSize * ((int)(stopAngle / stepSize) + 1);
                if (stopAngleTest < stopAngle)
                    stopAngleTest += stepSize;
                if (stopAngleTest > twoPi)
                    stopAngleTest = twoPi;
            }

            while (angle <= stopAngleTest)
            {
                Angle newAngle;
                newAngle.angle = (float)angle;
                newAngle.X = (float)cos(angle);
                newAngle.Y = (float)sin(angle);
                angles.push_back(newAngle);
                step += 1;
                angle = stepSize * step;
            }

            if (startAngle > angles[0].angle)
            {
                Angle newAngle;
                intersection(angles[0].X, angles[0].Y, angles[1].X, angles[1].Y, 0.0f, 0.0f, (float)cos(startAngle), (float)sin(startAngle));
                newAngle.angle = startAngle;
                newAngle.X = iX;
                newAngle.Y = iY;
                angles[0] = newAngle;
            }

            int index = angles.size() - 1;
            if (stopAngle < angles[index].angle)
            {
                Angle newAngle;
                intersection(angles[index - 1].X, angles[index - 1].Y, angles[index].X, angles[index].Y, 0.0f, 0.0f, (float)cos(stopAngle), (float)sin(stopAngle));
                newAngle.angle = stopAngle;
                newAngle.X = iX;
                newAngle.Y = iY;
                angles[index] = newAngle;
            }
        }
    }

    Profile::Profile()
    {
        faceNormal = Coord(0.0f, 0.0f, 1.0f);
        numOuterVerts = 0;
        numHollowVerts = 0;
        bottomFaceNumber = 0;
        numPrimFaces = 0;
    }

    Profile::Profile(int psides, float pprofileStart, float pprofileEnd, float phollow, int phollowSides, bool pcreateFaces)
    {
        faceNormal = Coord(0.0f, 0.0f, 1.0f);
        numOuterVerts = 0;
        numHollowVerts = 0;
        bottomFaceNumber = 0;
        numPrimFaces = 0;

        Coord center(0.0f, 0.0f, 0.0f);

        std::vector<Coord>hollowCoords;
        std::vector<Coord>hollowNormals;
        std::vector<float>hollowUs;

        bool hasHollow = (phollow > 0.0f);

        bool hasProfileCut = (pprofileStart > 0.0f || pprofileEnd < 1.0f);

        AngleList angles;
        AngleList hollowAngles;

        float xScale = 0.5f;
        float yScale = 0.5f;
        if (psides == 4)  // corners of a square are sqrt(2) from center
        {
            xScale = 0.707f;
            yScale = 0.707f;
        }

        float startAngle = pprofileStart * twoPi;
        float stopAngle = pprofileEnd * twoPi;

        angles.makeAngles(psides, startAngle, stopAngle); 

        numOuterVerts = angles.angles.size();

        // flag to create as few triangles as possible for 3 or 4 side profile
        //bool simpleFace = (psides < 5 && !(hasHollow || hasProfileCut));
        bool simpleFace = (psides < 5 && !hasHollow && !hasProfileCut);

        if (hasHollow)
        {
            if (psides == phollowSides)
                hollowAngles = angles;
            else
            {
                hollowAngles.makeAngles(phollowSides, startAngle, stopAngle);
            }
            numHollowVerts = hollowAngles.angles.size();
        }
        else if (!simpleFace)
        {
            coords.push_back(center);
            vertexNormals.push_back(Coord(0.0f, 0.0f, 1.0f));
            us.push_back(0.0f);
        }

        float z = 0.0f;

        Angle angle;
        Coord newVert;
        if (hasHollow && phollowSides != psides)
        {
            int numHollowAngles = hollowAngles.angles.size();
            for (int i = 0; i < numHollowAngles; i++)
            {
                angle = hollowAngles.angles[i];
                newVert.X = phollow * xScale * angle.X;
                newVert.Y = phollow * yScale * angle.Y;
                newVert.Z = z;

                hollowCoords.push_back(newVert);

                if (phollowSides < 5)
                    hollowNormals.push_back(hollowAngles.normals[i].Invert());
                else
                    hollowNormals.push_back(Coord(-angle.X, -angle.Y, 0.0f));

                hollowUs.push_back(angle.angle * phollow);
            }
        }

        int index = 0;
        int numAngles = angles.angles.size();

        for (int i = 0; i < numAngles; i++)
        {
            //int iNext = i == numAngles ? i + 1 : 0;
            angle = angles.angles[i];
            newVert.X = angle.X * xScale;
            newVert.Y = angle.Y * yScale;
            newVert.Z = z;
            coords.push_back(newVert);

            if (psides < 5)
            {
                vertexNormals.push_back(angles.normals[i]);
                float u = angle.angle;
                us.push_back(u);
            }
            else
            {
                vertexNormals.push_back(Coord(angle.X, angle.Y, 0.0f));
                us.push_back(angle.angle);
            }

            if (hasHollow)
            {
                if (phollowSides == psides)
                {
                    newVert.X *= phollow;
                    newVert.Y *= phollow;
                    newVert.Z = z;
                    hollowCoords.push_back(newVert);
                    if (psides < 5)
                    {
                        hollowNormals.push_back(angles.normals[i].Invert());
                    }

                    else
                        hollowNormals.push_back(Coord(-angle.X, -angle.Y, 0.0f));

                    hollowUs.push_back(angle.angle * phollow);
                }
            }
            else if (!simpleFace && pcreateFaces && angle.angle > 0.0001f)
            {
                Face newFace;
                newFace.v1 = 0;
                newFace.v2 = index;
                newFace.v3 = index + 1;

                faces.push_back(newFace);
            }
            index += 1;
        }

        if (hasHollow)
        {
            std::reverse(hollowCoords.begin(), hollowCoords.end());
            std::reverse(hollowNormals.begin(), hollowNormals.end());
            std::reverse(hollowUs.begin(), hollowUs.end());

            if (pcreateFaces)
            {
                int numOuterVerts = coords.size();
                int numHollowVerts = hollowCoords.size();
                int numTotalVerts = numOuterVerts + numHollowVerts;

                if (numOuterVerts == numHollowVerts)
                {
                    Face newFace;

                    for (int coordIndex = 0; coordIndex < numOuterVerts - 1; coordIndex++)
                    {
                        newFace.v1 = coordIndex;
                        newFace.v2 = coordIndex + 1;
                        newFace.v3 = numTotalVerts - coordIndex - 1;
                        faces.push_back(newFace);

                        newFace.v1 = coordIndex + 1;
                        newFace.v2 = numTotalVerts - coordIndex - 2;
                        newFace.v3 = numTotalVerts - coordIndex - 1;
                        faces.push_back(newFace);
                    }
                }
                else
                {
                    if (numOuterVerts < numHollowVerts)
                    {
                        Face newFace;
                        int j = 0; // j is the index for outer vertices
                        int maxJ = numOuterVerts - 1;
                        for (int i = 0; i < numHollowVerts; i++) // i is the index for inner vertices
                        {
                            if (j < maxJ)
                                if (angles.angles[j + 1].angle - hollowAngles.angles[i].angle < hollowAngles.angles[i].angle - angles.angles[j].angle + 0.000001f)
                                {
                                    newFace.v1 = numTotalVerts - i - 1;
                                    newFace.v2 = j;
                                    newFace.v3 = j + 1;

                                    faces.push_back(newFace);
                                    j += 1;
                                }

                            newFace.v1 = j;
                            newFace.v2 = numTotalVerts - i - 2;
                            newFace.v3 = numTotalVerts - i - 1;

                            faces.push_back(newFace);
                        }
                    }
                    else // numHollowVerts < numOuterVerts
                    {
                        Face newFace;
                        int j = 0; // j is the index for inner vertices
                        int maxJ = numHollowVerts - 1;
                        for (int i = 0; i < numOuterVerts; i++)
                        {
                            if (j < maxJ)
                                if (hollowAngles.angles[j + 1].angle - angles.angles[i].angle < angles.angles[i].angle - hollowAngles.angles[j].angle + 0.000001f)
                                {
                                    newFace.v1 = i;
                                    newFace.v2 = numTotalVerts - j - 2;
                                    newFace.v3 = numTotalVerts - j - 1;

                                    faces.push_back(newFace);
                                    j += 1;
                                }

                            newFace.v1 = numTotalVerts - j - 1;
                            newFace.v2 = i;
                            newFace.v3 = i + 1;

                            faces.push_back(newFace);
                        }
                    }
                }
            }

            coords.insert(coords.end(), hollowCoords.begin(), hollowCoords.end());
            vertexNormals.insert(vertexNormals.end(), hollowNormals.begin(), hollowNormals.end());
            us.insert(us.end(), hollowUs.begin(), hollowUs.end());
        }

        if (simpleFace && pcreateFaces)
        {
            if (psides == 3)
                faces.push_back(Face(0, 1, 2));
            else if (psides == 4)
            {
                faces.push_back(Face(0, 1, 2));
                faces.push_back(Face(0, 2, 3));
            }
        }

        if (hasProfileCut)
        {
            if (hasHollow)
            {
                int lastOuterVertIndex = numOuterVerts - 1;

                cutNormal1.X = coords[0].Y - coords[coords.size() - 1].Y;
                cutNormal1.Y = -(coords[0].X - coords[coords.size() - 1].X);

                cutNormal2.X = coords[lastOuterVertIndex + 1].Y - coords[lastOuterVertIndex].Y;
                cutNormal2.Y = -(coords[lastOuterVertIndex + 1].X - coords[lastOuterVertIndex].X);
            }

            else
            {
                cutNormal1.X = vertexNormals[1].Y;
                cutNormal1.Y = -vertexNormals[1].X;

                cutNormal2.X = -vertexNormals[vertexNormals.size() - 2].Y;
                cutNormal2.Y = vertexNormals[vertexNormals.size() - 2].X;

            }
            cutNormal1.Normalize();
            cutNormal2.Normalize();
        }

        MakeFaceUVs();

        hollowCoords.clear();
        hollowNormals.clear();
        hollowUs.clear();

        // calculate prim face numbers
        // I know it's ugly but so is the whole concept of prim face numbers
        int faceNum = 1;
        int startVert = hasProfileCut && !hasHollow ? 1 : 0;
        if (startVert > 0)
            faceNumbers.push_back(0);
        for (int i = 0; i < numOuterVerts; i++)
            faceNumbers.push_back(psides < 5 ? faceNum++ : faceNum);
        if (psides > 4)
            faceNum++;
        if (hasProfileCut)
            faceNumbers.push_back(0);
        for (int i = 0; i < numHollowVerts; i++)
            //faceNumbers.push_back(faceNum++);
            faceNumbers.push_back(phollowSides < 5 ? faceNum++ : faceNum);
        bottomFaceNumber = faceNum++;
        if (hasHollow && hasProfileCut)
            faceNumbers.push_back(faceNum++);
        for (int i = 0; i < faceNumbers.size(); i++)
            if (faceNumbers[i] == 0)
                faceNumbers[i] = faceNum++;

        numPrimFaces = faceNum;
    }

    void Profile::MakeFaceUVs()
    {
        faceUVs.clear();
        for (unsigned i = 0; i < coords.size(); ++i)
        {
            const Coord& c = coords[i];
            faceUVs.push_back(UVCoord(1.0f - (0.5f + c.X), 1.0f - (0.5f - c.Y)));
        }
    }

    void Profile::AddPos(Coord v)
    {
        AddPos(v.X, v.Y, v.Z);
    }

    void Profile::AddPos(float x, float y, float z)
    {
        int i;
        int numVerts = coords.size();
        Coord vert;

        for (i = 0; i < numVerts; i++)
        {
            vert = coords[i];
            vert.X += x;
            vert.Y += y;
            vert.Z += z;
            coords[i] = vert;
        }
    }

    void Profile::AddRot(Quat q)
    {
        int i;
        int numVerts = coords.size();

        for (i = 0; i < numVerts; i++)
            coords[i] *= q;

        int numNormals = vertexNormals.size();
        for (i = 0; i < numNormals; i++)
            vertexNormals[i] *= q;

        faceNormal *= q;
        cutNormal1 *= q;
        cutNormal2 *= q;

    }

    void Profile::Scale(float x, float y)
    {
        int i;
        int numVerts = coords.size();
        Coord vert;

        for (i = 0; i < numVerts; i++)
        {
            vert = coords[i];
            vert.X *= x;
            vert.Y *= y;
            coords[i] = vert;
        }
    }

    /// <summary>
    /// Changes order of the vertex indices and negates the center vertex normal. Does not alter vertex normals of radial vertices
    /// </summary>
    void Profile::FlipNormals()
    {
        int i;
        int numFaces = faces.size();
        Face tmpFace;
        int tmp;

        for (i = 0; i < numFaces; i++)
        {
            tmpFace = faces[i];
            tmp = tmpFace.v3;
            tmpFace.v3 = tmpFace.v1;
            tmpFace.v1 = tmp;
            faces[i] = tmpFace;
        }

        int normalCount = vertexNormals.size();
        if (normalCount > 0)
        {
            Coord n = vertexNormals[normalCount - 1];
            n.Z = -n.Z;
            vertexNormals[normalCount - 1] = n;
        }

        faceNormal.X = -faceNormal.X;
        faceNormal.Y = -faceNormal.Y;
        faceNormal.Z = -faceNormal.Z;

        int numfaceUVs = faceUVs.size();
        for (i = 0; i < numfaceUVs; i++)
        {
            UVCoord uv = faceUVs[i];
            uv.V = 1.0f - uv.V;
            faceUVs[i] = uv;
        }
    }

    void Profile::AddValue2FaceVertexIndices(int num)
    {
        int numFaces = faces.size();
        Face tmpFace;
        for (int i = 0; i < numFaces; i++)
        {
            tmpFace = faces[i];
            tmpFace.v1 += num;
            tmpFace.v2 += num;
            tmpFace.v3 += num;

            faces[i] = tmpFace;
        }
    }

    void Profile::AddValue2FaceNormalIndices(int num)
    {
        int numFaces = faces.size();
        Face tmpFace;
        for (int i = 0; i < numFaces; i++)
        {
            tmpFace = faces[i];
            tmpFace.n1 += num;
            tmpFace.n2 += num;
            tmpFace.n3 += num;

            faces[i] = tmpFace;
        }
    }

    PrimMesh::PrimMesh()
    {
        sides = 4;
        hollowSides = 4;
        profileStart = 0.0f;
        profileEnd = 1.0f;
        hollow = 0.0f;
        twistBegin = 0;
        twistEnd = 0;
        topShearX = 0.0f;
        topShearY = 0.0f;
        pathCutBegin = 0.0f;
        pathCutEnd = 1.0f;
        dimpleBegin = 0.0f;
        dimpleEnd = 1.0f;
        skew = 0.0f;
        holeSizeX = 1.0f; // called pathScaleX in pbs
        holeSizeY = 0.25f;
        taperX = 0.0f;
        taperY = 0.0f;
        radius = 0.0f;
        revolutions = 1.0f;
        stepsPerRevolution = 24;

        hasProfileCut = false;
        hasHollow = false;
        normalsProcessed = false;
        viewerMode = true;

        numPrimFaces = 0;
    }

    /// <summary>
    /// Constructs a PrimMesh object and creates the profile for extrusion.
    /// </summary>
    /// <param name="psides"></param>
    /// <param name="pprofileStart"></param>
    /// <param name="pprofileEnd"></param>
    /// <param name="phollow"></param>
    /// <param name="phollowSides"></param>
    PrimMesh::PrimMesh(int psides, float pprofileStart, float pprofileEnd, float phollow, int phollowSides)
    {
        sides = psides;
        hollowSides = phollowSides;
        profileStart = pprofileStart;
        profileEnd = pprofileEnd;
        hollow = phollow;
        twistBegin = 0;
        twistEnd = 0;
        topShearX = 0.0f;
        topShearY = 0.0f;
        pathCutBegin = 0.0f;
        pathCutEnd = 1.0f;
        dimpleBegin = 0.0f;
        dimpleEnd = 1.0f;
        skew = 0.0f;
        holeSizeX = 1.0f; // called pathScaleX in pbs
        holeSizeY = 0.25f;
        taperX = 0.0f;
        taperY = 0.0f;
        radius = 0.0f;
        revolutions = 1.0f;
        stepsPerRevolution = 24;

        hasProfileCut = false;
        hasHollow = false;
        normalsProcessed = false;
        viewerMode = true;

        numPrimFaces = 0;

        if (sides < 3)
            sides = 3;
        if (hollowSides < 3)
            hollowSides = 3;
        if (profileStart < 0.0f)
            profileStart = 0.0f;
        if (profileEnd > 1.0f)
            profileEnd = 1.0f;
        if (profileEnd < 0.02f)
            profileEnd = 0.02f;
        if (profileStart >= profileEnd)
            profileStart = profileEnd - 0.02f;
        if (hollow > 0.99f)
            hollow = 0.99f;
        if (hollow < 0.0f)
            hollow = 0.0f;

        hasProfileCut = (profileStart > 0.0f || profileEnd < 1.0f);
        hasHollow = (hollow > 0.001f);
    }

    /// <summary>
    /// Extrudes a profile along a straight line path. Used for prim types box, cylinder, and prism.
    /// </summary>
    void PrimMesh::ExtrudeLinear()
    {
        coords.clear();
        faces.clear();
        normals.clear();
        viewerFaces.clear();

        int step = 0;
        int steps = 1;

        float length = pathCutEnd - pathCutBegin;
        
        normalsProcessed = false;

        if (viewerMode && sides == 3)
        {
            // prisms don't taper well so add some vertical resolution
            // other prims may benefit from this but just do prisms for now
            if (abs(taperX) > 0.01 || abs(taperY) > 0.01)
                steps = (int)(steps * 4.5 * length);
        }

        float twistBegin = this->twistBegin / 360.0f * twoPi;
        float twistEnd = this->twistEnd / 360.0f * twoPi;
        float twistTotal = twistEnd - twistBegin;
        float twistTotalAbs = abs(twistTotal);
        if (twistTotalAbs > 0.01f)
            steps += (int)(twistTotalAbs * 3.66); //  dahlia's magic number

        float start = -0.5f;
        float stepSize = length / (float)steps;
        float percentOfPathMultiplier = stepSize;
        float xProfileScale = 1.0f;
        float yProfileScale = 1.0f;
        float xOffset = 0.0f;
        float yOffset = 0.0f;
        float zOffset = start;
        float xOffsetStepIncrement = topShearX / steps;
        float yOffsetStepIncrement = topShearY / steps;

        float percentOfPath = pathCutBegin;
        zOffset += percentOfPath;

        float hollow = this->hollow;

        // sanity checks
        float initialProfileRot = 0.0f;
        if (sides == 3)
        {
            if (hollowSides == 4)
            {
                if (hollow > 0.7f)
                    hollow = 0.7f;
                hollow *= 0.707f;
            }
            else hollow *= 0.5f;
        }
        else if (sides == 4)
        {
            initialProfileRot = 1.25f * (float)PI;
            if (hollowSides != 4)
                hollow *= 0.707f;
        }
        else if (sides == 24 && hollowSides == 4)
            hollow *= 1.414f;

        Profile profile(sides, profileStart, profileEnd, hollow, hollowSides, true);

        numPrimFaces = profile.numPrimFaces;

        int cut1Vert = -1;
        int cut2Vert = -1;
        if (hasProfileCut)
        {
            cut1Vert = hasHollow ? profile.coords.size() - 1 : 0;
            cut2Vert = hasHollow ? profile.numOuterVerts - 1 : profile.numOuterVerts;
        }

        if (initialProfileRot != 0.0f)
        {
            profile.AddRot(Quat(Coord(0.0f, 0.0f, 1.0f), initialProfileRot));
            if (viewerMode)
                profile.MakeFaceUVs();
        }

        Coord lastCutNormal1;
        Coord lastCutNormal2;
        float lastV = 1.0f;

        bool done = false;
        while (!done)
        {
            Profile newLayer = profile;

            if (taperX == 0.0f)
                xProfileScale = 1.0f;
            else if (taperX > 0.0f)
                xProfileScale = 1.0f - percentOfPath * taperX;
            else xProfileScale = 1.0f + (1.0f - percentOfPath) * taperX;

            if (taperY == 0.0f)
                yProfileScale = 1.0f;
            else if (taperY > 0.0f)
                yProfileScale = 1.0f - percentOfPath * taperY;
            else yProfileScale = 1.0f + (1.0f - percentOfPath) * taperY;

            if (xProfileScale != 1.0f || yProfileScale != 1.0f)
                newLayer.Scale(xProfileScale, yProfileScale);

            float twist = twistBegin + twistTotal * percentOfPath;
            if (twist != 0.0f)
                newLayer.AddRot(Quat(Coord(0.0f, 0.0f, 1.0f), twist));

            newLayer.AddPos(xOffset, yOffset, zOffset);

            if (step == 0)
            {
                newLayer.FlipNormals();

                // add the top faces to the viewerFaces list here
                if (viewerMode)
                {
                    Coord faceNormal = newLayer.faceNormal;
                    int numFaces = newLayer.faces.size();

                    for (int i = 0; i < numFaces; i++)
                    {
                        ViewerFace newViewerFace(newLayer.bottomFaceNumber);
                        const Face& face = newLayer.faces[i];
                        newViewerFace.v1 = newLayer.coords[face.v1];
                        newViewerFace.v2 = newLayer.coords[face.v2];
                        newViewerFace.v3 = newLayer.coords[face.v3];

                        newViewerFace.n1 = faceNormal;
                        newViewerFace.n2 = faceNormal;
                        newViewerFace.n3 = faceNormal;

                        newViewerFace.uv1 = newLayer.faceUVs[face.v1].Flip();
                        newViewerFace.uv2 = newLayer.faceUVs[face.v2].Flip();
                        newViewerFace.uv3 = newLayer.faceUVs[face.v3].Flip();

                        viewerFaces.push_back(newViewerFace);
                    }
                }
            }

            // append this layer

            int coordsLen = coords.size();
            newLayer.AddValue2FaceVertexIndices(coordsLen);

            coords.insert(coords.end(), newLayer.coords.begin(), newLayer.coords.end());

            newLayer.AddValue2FaceNormalIndices(normals.size());
            normals.insert(normals.end(), newLayer.vertexNormals.begin(), newLayer.vertexNormals.end());

            if (percentOfPath < pathCutBegin + 0.01f || percentOfPath > pathCutEnd - 0.01f)
                faces.insert(faces.end(), newLayer.faces.begin(), newLayer.faces.end());

            // fill faces between layers

            int numVerts = newLayer.coords.size();
            Face newFace;

            if (step > 0)
            {
                int startVert = coordsLen + 1;
                int endVert = coords.size();

                if (sides < 5 || hasProfileCut || hollow > 0.0f)
                    startVert--;

                for (int i = startVert; i < endVert; i++)
                {
                    int iNext = i + 1;
                    if (i == endVert - 1)
                        iNext = startVert;

                    int whichVert = i - startVert;

                    newFace.v1 = i;
                    newFace.v2 = i - numVerts;
                    newFace.v3 = iNext - numVerts;
                    faces.push_back(newFace);

                    newFace.v2 = iNext - numVerts;
                    newFace.v3 = iNext;
                    faces.push_back(newFace);

                    if (viewerMode)
                    {
                        // add the side faces to the list of viewerFaces here
                        int primFaceNum = 1;
                        if (whichVert >= sides)
                            primFaceNum = 2;
                        ViewerFace newViewerFace1(primFaceNum);
                        ViewerFace newViewerFace2(primFaceNum);

                        float u1 = newLayer.us[whichVert];
                        float u2 = 1.0f;
                        if (whichVert < newLayer.us.size() - 1)
                            u2 = newLayer.us[whichVert + 1];

                        if (whichVert == cut1Vert || whichVert == cut2Vert)
                        {
                            u1 = 0.0f;
                            u2 = 1.0f;
                        }
                        else if (sides < 5)
                        { // boxes and prisms have one texture face per side of the prim, so the U values have to be scaled
                            // to reflect the entire texture width
                            u1 *= sides;
                            u2 *= sides;
                            u2 -= (int)u1;
                            u1 -= (int)u1;
                            if (u2 < 0.1f)
                                u2 = 1.0f;

                            //newViewerFace2.primFaceNumber = newViewerFace1.primFaceNumber = whichVert + 1;
                        }

                        newViewerFace1.uv1.U = u1;
                        newViewerFace1.uv2.U = u1;
                        newViewerFace1.uv3.U = u2;

                        newViewerFace1.uv1.V = 1.0f - percentOfPath;
                        newViewerFace1.uv2.V = lastV;
                        newViewerFace1.uv3.V = lastV;

                        newViewerFace2.uv1.U = u1;
                        newViewerFace2.uv2.U = u2;
                        newViewerFace2.uv3.U = u2;

                        newViewerFace2.uv1.V = 1.0f - percentOfPath;
                        newViewerFace2.uv2.V = lastV;
                        newViewerFace2.uv3.V = 1.0f - percentOfPath;

                        newViewerFace1.v1 = coords[i];
                        newViewerFace1.v2 = coords[i - numVerts];
                        newViewerFace1.v3 = coords[iNext - numVerts];

                        newViewerFace2.v1 = coords[i];
                        newViewerFace2.v2 = coords[iNext - numVerts];
                        newViewerFace2.v3 = coords[iNext];

                        // profile cut faces
                        if (whichVert == cut1Vert)
                        {
                            newViewerFace1.n1 = newLayer.cutNormal1;
                            newViewerFace1.n2 = newViewerFace1.n3 = lastCutNormal1;

                            newViewerFace2.n1 = newViewerFace2.n3 = newLayer.cutNormal1;
                            newViewerFace2.n2 = lastCutNormal1;
                        }
                        else if (whichVert == cut2Vert)
                        {
                            newViewerFace1.n1 = newLayer.cutNormal2;
                            newViewerFace1.n2 = newViewerFace1.n3 = lastCutNormal2;

                            newViewerFace2.n1 = newViewerFace2.n3 = newLayer.cutNormal2;
                            newViewerFace2.n2 = lastCutNormal2;
                        }

                        else // outer and hollow faces
                        {
                            if ((sides < 5 && whichVert < newLayer.numOuterVerts) || (hollowSides < 5 && whichVert >= newLayer.numOuterVerts))
                            {
                                newViewerFace1.CalcSurfaceNormal();
                                newViewerFace2.CalcSurfaceNormal();
                            }
                            else
                            {
                                newViewerFace1.n1 = normals[i];
                                newViewerFace1.n2 = normals[i - numVerts];
                                newViewerFace1.n3 = normals[iNext - numVerts];

                                newViewerFace2.n1 = normals[i];
                                newViewerFace2.n2 = normals[iNext - numVerts];
                                newViewerFace2.n3 = normals[iNext];
                            }
                        }

                        newViewerFace2.primFaceNumber = newViewerFace1.primFaceNumber = newLayer.faceNumbers[whichVert];

                        viewerFaces.push_back(newViewerFace1);
                        viewerFaces.push_back(newViewerFace2);

                    }
                }
            }

            lastCutNormal1 = newLayer.cutNormal1;
            lastCutNormal2 = newLayer.cutNormal2;
            lastV = 1.0f - percentOfPath;

            // calc the step for the next iteration of the loop

            if (step < steps)
            {
                step += 1;
                percentOfPath += percentOfPathMultiplier;
                xOffset += xOffsetStepIncrement;
                yOffset += yOffsetStepIncrement;
                zOffset += stepSize;
                if (percentOfPath > pathCutEnd)
                    done = true;
            }
            else done = true;
            
            if (done && viewerMode)
            {
                // add the top faces to the viewerFaces list here
                ViewerFace newViewerFace;
                Coord faceNormal = newLayer.faceNormal;
                newViewerFace.primFaceNumber = 0;
                int numFaces = newLayer.faces.size();
                const std::vector<Face>& faces = newLayer.faces;

                for (int i = 0; i < numFaces; i++)
                {
                    const Face& face = faces[i];
                    newViewerFace.v1 = newLayer.coords[face.v1 - coordsLen];
                    newViewerFace.v2 = newLayer.coords[face.v2 - coordsLen];
                    newViewerFace.v3 = newLayer.coords[face.v3 - coordsLen];

                    newViewerFace.n1 = faceNormal;
                    newViewerFace.n2 = faceNormal;
                    newViewerFace.n3 = faceNormal;

                    newViewerFace.uv1 = newLayer.faceUVs[face.v1 - coordsLen].Flip();;
                    newViewerFace.uv2 = newLayer.faceUVs[face.v2 - coordsLen].Flip();;
                    newViewerFace.uv3 = newLayer.faceUVs[face.v3 - coordsLen].Flip();;

                    viewerFaces.push_back(newViewerFace);
                }
            }
        }
    }

    /// <summary>
    /// Extrude a profile into a circular path prim mesh. Used for prim types torus, tube, and ring.
    /// </summary>
    void PrimMesh::ExtrudeCircular()
    {
        coords.clear();
        faces.clear();
        viewerFaces.clear();
        normals.clear();

        int step = 0;
        int steps = 24;

        normalsProcessed = false;

        float twistBegin = this->twistBegin / 360.0f * twoPi;
        float twistEnd = this->twistEnd / 360.0f * twoPi;
        float twistTotal = twistEnd - twistBegin;

        // if the profile has a lot of twist, add more layers otherwise the layers may overlap
        // and the resulting mesh may be quite inaccurate. This method is arbitrary and doesn't
        // accurately match the viewer
        float twistTotalAbs = abs(twistTotal);
        if (twistTotalAbs > 0.01f)
        {
            if (twistTotalAbs > PI * 1.5f)
                steps *= 2;
            if (twistTotalAbs > PI * 3.0f)
                steps *= 2;
        }

        float yPathScale = holeSizeY * 0.5f;
        float pathLength = pathCutEnd - pathCutBegin;
        float totalSkew = skew * 2.0f * pathLength;
        float skewStart = pathCutBegin * 2.0f * skew - skew;
        float xOffsetTopShearXFactor = topShearX * (0.25f + 0.5f * (0.5f - holeSizeY));
        float yShearCompensation = 1.0f + abs(topShearY) * 0.25f;

        // It's not quite clear what pushY (Y top shear) does, but subtracting it from the start and end
        // angles appears to approximate it's effects on path cut. Likewise, adding it to the angle used
        // to calculate the sine for generating the path radius appears to approximate it's effects there
        // too, but there are some subtle differences in the radius which are noticeable as the prim size
        // increases and it may affect megaprims quite a bit. The effect of the Y top shear parameter on
        // the meshes generated with this technique appear nearly identical in shape to the same prims when
        // displayed by the viewer.

        float startAngle = (twoPi * pathCutBegin * revolutions) - topShearY * 0.9f;
        float endAngle = (twoPi * pathCutEnd * revolutions) - topShearY * 0.9f;
        float stepSize = twoPi / stepsPerRevolution;

        step = (int)(startAngle / stepSize);
        int firstStep = step;
        float angle = startAngle;
        float hollow = this->hollow;

        // sanity checks
        float initialProfileRot = 0.0f;
        if (sides == 3)
        {
            initialProfileRot = (float)PI;
            if (hollowSides == 4)
            {
                if (hollow > 0.7f)
                    hollow = 0.7f;
                hollow *= 0.707f;
            }
            else hollow *= 0.5f;
        }
        else if (sides == 4)
        {
            initialProfileRot = 0.25f * (float)PI;
            if (hollowSides != 4)
                hollow *= 0.707f;
        }
        else if (sides > 4)
        {
            initialProfileRot = (float)PI;
            if (hollowSides == 4)
            {
                if (hollow > 0.7f)
                    hollow = 0.7f;
                hollow /= 0.7f;
            }
        }

        bool needEndFaces = false;
        if (pathCutBegin != 0.0f || pathCutEnd != 1.0f)
            needEndFaces = true;
        else if (taperX != 0.0f || taperY != 0.0f)
            needEndFaces = true;
        else if (skew != 0.0f)
            needEndFaces = true;
        else if (twistTotal != 0.0f)
            needEndFaces = true;
        else if (radius != 0.0f)
            needEndFaces = true;

        Profile profile(sides, profileStart, profileEnd, hollow, hollowSides, needEndFaces);

        numPrimFaces = profile.numPrimFaces;

        int cut1Vert = -1;
        int cut2Vert = -1;
        if (hasProfileCut)
        {
            cut1Vert = hasHollow ? profile.coords.size() - 1 : 0;
            cut2Vert = hasHollow ? profile.numOuterVerts - 1 : profile.numOuterVerts;
        }

        if (initialProfileRot != 0.0f)
        {
            profile.AddRot(Quat(Coord(0.0f, 0.0f, 1.0f), initialProfileRot));
            if (viewerMode)
                profile.MakeFaceUVs();
        }

        Coord lastCutNormal1;
        Coord lastCutNormal2;
        float lastV = 1.0f;

        bool done = false;
        while (!done) // loop through the length of the path and add the layers
        {
            bool isEndLayer = false;
            if (angle <= startAngle + .01f || angle >= endAngle - .01f)
                isEndLayer = true;

            //Profile newLayer = profile.Copy(isEndLayer && needEndFaces);
            Profile newLayer = profile;

            float xProfileScale = (1.0f - abs(skew)) * holeSizeX;
            float yProfileScale = holeSizeY;

            float percentOfPath = angle / (twoPi * revolutions);
            float percentOfAngles = (angle - startAngle) / (endAngle - startAngle);

            if (taperX > 0.01f)
                xProfileScale *= 1.0f - percentOfPath * taperX;
            else if (taperX < -0.01f)
                xProfileScale *= 1.0f + (1.0f - percentOfPath) * taperX;

            if (taperY > 0.01f)
                yProfileScale *= 1.0f - percentOfPath * taperY;
            else if (taperY < -0.01f)
                yProfileScale *= 1.0f + (1.0f - percentOfPath) * taperY;

            if (xProfileScale != 1.0f || yProfileScale != 1.0f)
                newLayer.Scale(xProfileScale, yProfileScale);

            float radiusScale = 1.0f;
            if (radius > 0.001f)
                radiusScale = 1.0f - radius * percentOfPath;
            else if (radius < 0.001f)
                radiusScale = 1.0f + radius * (1.0f - percentOfPath);

            float twist = twistBegin + twistTotal * percentOfPath;

            float xOffset = 0.5f * (skewStart + totalSkew * percentOfAngles);
            xOffset += (float)sin(angle) * xOffsetTopShearXFactor;

            float yOffset = yShearCompensation * (float)cos(angle) * (0.5f - yPathScale) * radiusScale;

            float zOffset = (float)sin(angle + topShearY) * (0.5f - yPathScale) * radiusScale;

            // next apply twist rotation to the profile layer
            if (twistTotal != 0.0f || twistBegin != 0.0f)
                newLayer.AddRot(Quat(Coord(0.0f, 0.0f, 1.0f), twist));

            // now orient the rotation of the profile layer relative to it's position on the path
            // adding taperY to the angle used to generate the quat appears to approximate the viewer
            newLayer.AddRot(Quat(Coord(1.0f, 0.0f, 0.0f), angle + topShearY));
            newLayer.AddPos(xOffset, yOffset, zOffset);

            if (isEndLayer && angle <= startAngle + .01f)
            {
                newLayer.FlipNormals();

                // add the top faces to the viewerFaces list here
                if (viewerMode && needEndFaces)
                {
                    Coord faceNormal = newLayer.faceNormal;

                    for (unsigned i = 0; i < newLayer.faces.size(); ++i)
                    {
                        const Face& face = newLayer.faces[i];

                        ViewerFace newViewerFace;
                        newViewerFace.primFaceNumber = 0;
                        
                        newViewerFace.v1 = newLayer.coords[face.v1];
                        newViewerFace.v2 = newLayer.coords[face.v2];
                        newViewerFace.v3 = newLayer.coords[face.v3];

                        newViewerFace.n1 = faceNormal;
                        newViewerFace.n2 = faceNormal;
                        newViewerFace.n3 = faceNormal;

                        newViewerFace.uv1 = newLayer.faceUVs[face.v1];
                        newViewerFace.uv2 = newLayer.faceUVs[face.v2];
                        newViewerFace.uv3 = newLayer.faceUVs[face.v3];

                        viewerFaces.push_back(newViewerFace);
                    }
                }
            }

            // append the layer and fill in the sides

            int coordsLen = coords.size();
            newLayer.AddValue2FaceVertexIndices(coordsLen);

            coords.insert(coords.end(), newLayer.coords.begin(), newLayer.coords.end());
            newLayer.AddValue2FaceNormalIndices(normals.size());
            normals.insert(normals.end(), newLayer.vertexNormals.begin(), newLayer.vertexNormals.end());

            if (isEndLayer)
                faces.insert(faces.end(), newLayer.faces.begin(), newLayer.faces.end());

            // fill faces between layers

            int numVerts = newLayer.coords.size();
            Face newFace;
            if (step > firstStep)
            {
                int startVert = coordsLen + 1;
                int endVert = coords.size();

                if (sides < 5 || hasProfileCut || hollow > 0.0f)
                    startVert--;

                for (int i = startVert; i < endVert; i++)
                {
                    int iNext = i + 1;
                    if (i == endVert - 1)
                        iNext = startVert;

                    int whichVert = i - startVert;

                    newFace.v1 = i;
                    newFace.v2 = i - numVerts;
                    newFace.v3 = iNext - numVerts;
                    faces.push_back(newFace);

                    newFace.v2 = iNext - numVerts;
                    newFace.v3 = iNext;
                    faces.push_back(newFace);

                    if (viewerMode)
                    {
                        // add the side faces to the list of viewerFaces here
                        ViewerFace newViewerFace1;
                        ViewerFace newViewerFace2;
                        float u1 = newLayer.us[whichVert];
                        float u2 = 1.0f;
                        if (whichVert < newLayer.us.size() - 1)
                            u2 = newLayer.us[whichVert + 1];

                        if (whichVert == cut1Vert || whichVert == cut2Vert)
                        {
                            u1 = 0.0f;
                            u2 = 1.0f;
                        }
                        else if (sides < 5)
                        { // boxes and prisms have one texture face per side of the prim, so the U values have to be scaled
                            // to reflect the entire texture width
                            u1 *= sides;
                            u2 *= sides;
                            u2 -= (int)u1;
                            u1 -= (int)u1;
                            if (u2 < 0.1f)
                                u2 = 1.0f;

                            //newViewerFace2.primFaceNumber = newViewerFace1.primFaceNumber = whichVert + 1;
                        }

                        newViewerFace1.uv1.U = u1;
                        newViewerFace1.uv2.U = u1;
                        newViewerFace1.uv3.U = u2;

                        newViewerFace1.uv1.V = 1.0f - percentOfPath;
                        newViewerFace1.uv2.V = lastV;
                        newViewerFace1.uv3.V = lastV;

                        newViewerFace2.uv1.U = u1;
                        newViewerFace2.uv2.U = u2;
                        newViewerFace2.uv3.U = u2;

                        newViewerFace2.uv1.V = 1.0f - percentOfPath;
                        newViewerFace2.uv2.V = lastV;
                        newViewerFace2.uv3.V = 1.0f - percentOfPath;

                        newViewerFace1.v1 = coords[i];
                        newViewerFace1.v2 = coords[i - numVerts];
                        newViewerFace1.v3 = coords[iNext - numVerts];

                        newViewerFace2.v1 = coords[i];
                        newViewerFace2.v2 = coords[iNext - numVerts];
                        newViewerFace2.v3 = coords[iNext];

                        // profile cut faces
                        if (whichVert == cut1Vert)
                        {
                            newViewerFace1.n1 = newLayer.cutNormal1;
                            newViewerFace1.n2 = newViewerFace1.n3 = lastCutNormal1;

                            newViewerFace2.n1 = newViewerFace2.n3 = newLayer.cutNormal1;
                            newViewerFace2.n2 = lastCutNormal1;
                        }
                        else if (whichVert == cut2Vert)
                        {
                            newViewerFace1.n1 = newLayer.cutNormal2;
                            newViewerFace1.n2 = newViewerFace1.n3 = lastCutNormal2;

                            newViewerFace2.n1 = newViewerFace2.n3 = newLayer.cutNormal2;
                            newViewerFace2.n2 = lastCutNormal2;
                        }
                        else // periphery faces
                        {
                            if (sides < 5 && whichVert < newLayer.numOuterVerts)
                            {
                                newViewerFace1.n1 = normals[i];
                                newViewerFace1.n2 = normals[i - numVerts];
                                newViewerFace1.n3 = normals[i - numVerts];

                                newViewerFace2.n1 = normals[i];
                                newViewerFace2.n2 = normals[i - numVerts];
                                newViewerFace2.n3 = normals[i];
                            }
                            else if (hollowSides < 5 && whichVert >= newLayer.numOuterVerts)
                            {
                                newViewerFace1.n1 = normals[iNext];
                                newViewerFace1.n2 = normals[iNext - numVerts];
                                newViewerFace1.n3 = normals[iNext - numVerts];

                                newViewerFace2.n1 = normals[iNext];
                                newViewerFace2.n2 = normals[iNext - numVerts];
                                newViewerFace2.n3 = normals[iNext];
                            }
                            else
                            {
                                newViewerFace1.n1 = normals[i];
                                newViewerFace1.n2 = normals[i - numVerts];
                                newViewerFace1.n3 = normals[iNext - numVerts];

                                newViewerFace2.n1 = normals[i];
                                newViewerFace2.n2 = normals[iNext - numVerts];
                                newViewerFace2.n3 = normals[iNext];
                            }
                        }

                        newViewerFace1.primFaceNumber = newViewerFace2.primFaceNumber = newLayer.faceNumbers[whichVert];
                        viewerFaces.push_back(newViewerFace1);
                        viewerFaces.push_back(newViewerFace2);

                    }
                }
            }

            lastCutNormal1 = newLayer.cutNormal1;
            lastCutNormal2 = newLayer.cutNormal2;
            lastV = 1.0f - percentOfPath;

            // calculate terms for next iteration
            // calculate the angle for the next iteration of the loop

            if (angle >= endAngle)
            {
                done = true;
            }
            else
            {
                step += 1;
                angle = stepSize * step;
                if (angle > endAngle)
                    angle = endAngle;
            }

            if (done && viewerMode && needEndFaces)
            {
                // add the bottom faces to the viewerFaces list here
                Coord faceNormal = newLayer.faceNormal;
                ViewerFace newViewerFace;
                newViewerFace.primFaceNumber = newLayer.bottomFaceNumber;
                for (unsigned i = 0; i < newLayer.faces.size(); ++i)
                {
                    const Face& face = newLayer.faces[i];
                    
                    newViewerFace.v1 = newLayer.coords[face.v1 - coordsLen];
                    newViewerFace.v2 = newLayer.coords[face.v2 - coordsLen];
                    newViewerFace.v3 = newLayer.coords[face.v3 - coordsLen];

                    newViewerFace.n1 = faceNormal;
                    newViewerFace.n2 = faceNormal;
                    newViewerFace.n3 = faceNormal;

                    newViewerFace.uv1 = newLayer.faceUVs[face.v1 - coordsLen];
                    newViewerFace.uv2 = newLayer.faceUVs[face.v2 - coordsLen];
                    newViewerFace.uv3 = newLayer.faceUVs[face.v3 - coordsLen];

                    viewerFaces.push_back(newViewerFace);
                }
            }
        }
    }

    Coord PrimMesh::SurfaceNormal(const Coord& c1, const Coord& c2, const Coord& c3)
    {
        Coord edge1(c2.X - c1.X, c2.Y - c1.Y, c2.Z - c1.Z);
        Coord edge2(c3.X - c1.X, c3.Y - c1.Y, c3.Z - c1.Z);

        Coord normal = Coord::Cross(edge1, edge2);

        normal.Normalize();

        return normal;
    }

    Coord PrimMesh::SurfaceNormal(const Face& face)
    {
        return SurfaceNormal(coords[face.v1], coords[face.v2], coords[face.v3]);
    }

    /// <summary>
    /// Calculate the surface normal for a face in the list of faces
    /// </summary>
    /// <param name="faceIndex"></param>
    /// <returns></returns>
    Coord PrimMesh::SurfaceNormal(int faceIndex)
    {
        int numFaces = faces.size();
        if (faceIndex < 0 || faceIndex >= numFaces)
            throw Exception("faceIndex out of range");

        return SurfaceNormal(faces[faceIndex]);
    }

    /// <summary>
    /// Calculate surface normals for all of the faces in the list of faces in this mesh
    /// </summary>
    void PrimMesh::CalcNormals()
    {
        if (normalsProcessed)
            return;

        normalsProcessed = true;

        int numFaces = faces.size();

        normals.clear();

        for (int i = 0; i < numFaces; i++)
        {
            Face face = faces[i];

            normals.push_back(SurfaceNormal(i).Normalize());

            int normIndex = normals.size() - 1;
            face.n1 = normIndex;
            face.n2 = normIndex;
            face.n3 = normIndex;

            faces[i] = face;
        }
    }

    /// <summary>
    /// Adds a value to each XYZ vertex coordinate in the mesh
    /// </summary>
    /// <param name="x"></param>
    /// <param name="y"></param>
    /// <param name="z"></param>
    void PrimMesh::AddPos(float x, float y, float z)
    {
        int i;
        int numVerts = coords.size();
        Coord vert;

        for (i = 0; i < numVerts; i++)
        {
            vert = coords[i];
            vert.X += x;
            vert.Y += y;
            vert.Z += z;
            coords[i] = vert;
        }
    }

    /// <summary>
    /// Rotates the mesh
    /// </summary>
    /// <param name="q"></param>
    void PrimMesh::AddRot(Quat q)
    {
        int i;
        int numVerts = coords.size();

        for (i = 0; i < numVerts; i++)
            coords[i] *= q;

        int numNormals = normals.size();
        for (i = 0; i < numNormals; i++)
            normals[i] *= q;

        int numViewerFaces = viewerFaces.size();
        for (i = 0; i < numViewerFaces; i++)
        {
            ViewerFace v = viewerFaces[i];
            v.v1 *= q;
            v.v2 *= q;
            v.v3 *= q;

            v.n1 *= q;
            v.n2 *= q;
            v.n3 *= q;
            viewerFaces[i] = v;
        }
    }

    /// <summary>
    /// Scales the mesh
    /// </summary>
    /// <param name="x"></param>
    /// <param name="y"></param>
    /// <param name="z"></param>
    void PrimMesh::Scale(float x, float y, float z)
    {
        int i;
        int numVerts = coords.size();
        //Coord vert;

        Coord m(x, y, z);
        for (i = 0; i < numVerts; i++)
            coords[i] *= m;

        int numViewerFaces = viewerFaces.size();
        for (i = 0; i < numViewerFaces; i++)
        {
            ViewerFace v = viewerFaces[i];
            v.v1 *= m;
            v.v2 *= m;
            v.v3 *= m;
            viewerFaces[i] = v;
        }
    }

}
