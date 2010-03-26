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

namespace PrimMesher
{
    struct Coord;
    
    struct Quat
    {
        /// <summary>X value</summary>
        float X;
        /// <summary>Y value</summary>
        float Y;
        /// <summary>Z value</summary>
        float Z;
        /// <summary>W value</summary>
        float W;

        Quat();

        Quat(float x, float y, float z, float w);

        Quat(const Coord& axis, float angle);

        float Length();

        Quat& Normalize();
    };

    struct Coord
    {
        float X;
        float Y;
        float Z;

        Coord();

        Coord(float x, float y, float z);

        float Length();

        Coord& Invert();

        Coord& Normalize();

        static Coord Cross(const Coord& c1, const Coord& c2);

        Coord operator + (const Coord& a)
        {
            return Coord(X + a.X, Y + a.Y, Z + a.Z);
        }

        Coord operator * (const Coord& m)
        {
            return Coord(X * m.X, Y * m.Y, Z * m.Z);
        }

        Coord& operator *= (const Coord& m)
        {
            *this = *this * m;
            return *this;
        }

        Coord operator * (const Quat& q)
        {
            // From http://www.euclideanspace.com/maths/algebra/realNormedAlgebra/quaternions/transforms/

            Coord c2;

            c2.X = q.W * q.W * X +
                2.0f * q.Y * q.W * Z -
                2.0f * q.Z * q.W * Y +
                     q.X * q.X * X +
                2.0f * q.Y * q.X * Y +
                2.0f * q.Z * q.X * Z -
                     q.Z * q.Z * X -
                     q.Y * q.Y * X;

            c2.Y =
                2.0f * q.X * q.Y * X +
                     q.Y * q.Y * Y +
                2.0f * q.Z * q.Y * Z +
                2.0f * q.W * q.Z * X -
                     q.Z * q.Z * Y +
                     q.W * q.W * Y -
                2.0f * q.X * q.W * Z -
                     q.X * q.X * Y;

            c2.Z =
                2.0f * q.X * q.Z * X +
                2.0f * q.Y * q.Z * Y +
                     q.Z * q.Z * Z -
                2.0f * q.W * q.Y * X -
                     q.Y * q.Y * Z +
                2.0f * q.W * q.X * Y -
                     q.X * q.X * Z +
                     q.W * q.W * Z;

            return c2;
        }
        
        Coord& operator *= (const Quat& q)
        {
            *this = *this * q;
            return *this;
        }
    };

    struct UVCoord
    {
        float U;
        float V;

        UVCoord();

        UVCoord(float u, float v);
        
        UVCoord Flip();
    };

    struct Face
    {
        int primFace;

        // vertices
        int v1;
        int v2;
        int v3;

        //normals
        int n1;
        int n2;
        int n3;

        // uvs
        int uv1;
        int uv2;
        int uv3;

        Face();

        Face(int pv1, int pv2, int pv3);

        Face(int pv1, int pv2, int pv3, int pn1, int pn2, int pn3);

        Coord SurfaceNormal(const std::vector<Coord>& coordList);
    };

    struct ViewerFace
    {
        int primFaceNumber;

        Coord v1;
        Coord v2;
        Coord v3;

        Coord n1;
        Coord n2;
        Coord n3;

        UVCoord uv1;
        UVCoord uv2;
        UVCoord uv3;

        ViewerFace();

        ViewerFace(int pprimFaceNumber);

        void Scale(float x, float y, float z);

        void AddRot(const Quat& q);

        void CalcSurfaceNormal();
    };

    struct Angle
    {
        float angle;
        float X;
        float Y;

        Angle();
        
        Angle(float pangle, float px, float py);
    };

    struct AngleList
    {
        float iX, iY; // intersection point

        std::vector<Angle> angles;
        std::vector<Coord> normals;

        Angle interpolatePoints(float newPoint, const Angle& p1, const Angle& p2);

        void intersection(double x1, double y1, double x2, double y2, double x3, double y3, double x4, double y4);

        void makeAngles(int sides, float startAngle, float stopAngle);
    };

    /// <summary>
    /// generates a profile for extrusion
    /// </summary>
    struct Profile
    {
        std::vector<Coord> coords;
        std::vector<Face> faces;
        std::vector<Coord> vertexNormals;
        std::vector<float> us;
        std::vector<UVCoord> faceUVs;
        std::vector<int> faceNumbers;

        Coord faceNormal;
        Coord cutNormal1;
        Coord cutNormal2;

        int numOuterVerts;
        int numHollowVerts;

        int bottomFaceNumber;
        int numPrimFaces;

        Profile();

        Profile(int psides, float pprofileStart, float pprofileEnd, float phollow, int phollowSides, bool pcreateFaces);

        void MakeFaceUVs();

        void AddPos(Coord v);

        void AddPos(float x, float y, float z);

        void AddRot(Quat q);

        void Scale(float x, float y);

        /// <summary>
        /// Changes order of the vertex indices and negates the center vertex normal. Does not alter vertex normals of radial vertices
        /// </summary>
        void FlipNormals();

        void AddValue2FaceVertexIndices(int num);

        void AddValue2FaceNormalIndices(int num);
    };

    struct PrimMesh
    {
        std::vector<Coord> coords;
        std::vector<Coord> normals;
        std::vector<Face> faces;

        std::vector<ViewerFace> viewerFaces;

        int sides;
        int hollowSides;
        float profileStart;
        float profileEnd;
        float hollow;
        int twistBegin;
        int twistEnd;
        float topShearX;
        float topShearY;
        float pathCutBegin;
        float pathCutEnd;
        float dimpleBegin;
        float dimpleEnd;
        float skew;
        float holeSizeX; // called pathScaleX in pbs
        float holeSizeY;
        float taperX;
        float taperY;
        float radius;
        float revolutions;
        int stepsPerRevolution;

        bool hasProfileCut;
        bool hasHollow;
        bool normalsProcessed;
        bool viewerMode;

        int numPrimFaces;

        PrimMesh();
    
        /// <summary>
        /// Constructs a PrimMesh object and creates the profile for extrusion.
        /// </summary>
        /// <param name="sides"></param>
        /// <param name="profileStart"></param>
        /// <param name="profileEnd"></param>
        /// <param name="hollow"></param>
        /// <param name="hollowSides"></param>
        PrimMesh(int psides, float pprofileStart, float pprofileEnd, float phollow, int phollowSides);
 
        /// <summary>
        /// Extrudes a profile along a straight line path. Used for prim types box, cylinder, and prism.
        /// </summary>
        void ExtrudeLinear();
 
        /// <summary>
        /// Extrude a profile into a circular path prim mesh. Used for prim types torus, tube, and ring.
        /// </summary>
        void ExtrudeCircular();
 
        Coord SurfaceNormal(const Coord& c1, const Coord& c2, const Coord& c3);

        Coord SurfaceNormal(const Face& face);

        /// <summary>
        /// Calculate the surface normal for a face in the list of faces
        /// </summary>
        /// <param name="faceIndex"></param>
        /// <returns></returns>
        Coord SurfaceNormal(int faceIndex);
    
        /// <summary>
        /// Calculate surface normals for all of the faces in the list of faces in this mesh
        /// </summary>
        void CalcNormals();

        /// <summary>
        /// Adds a value to each XYZ vertex coordinate in the mesh
        /// </summary>
        /// <param name="x"></param>
        /// <param name="y"></param>
        /// <param name="z"></param>
        void AddPos(float x, float y, float z);

        /// <summary>
        /// Rotates the mesh
        /// </summary>
        /// <param name="q"></param>
        void AddRot(Quat q);

        /// <summary>
        /// Scales the mesh
        /// </summary>
        /// <param name="x"></param>
        /// <param name="y"></param>
        /// <param name="z"></param>
        void Scale(float x, float y, float z);
    };
}
