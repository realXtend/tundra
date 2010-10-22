/***************************************************************************

This source file is part of OGREBULLET
(Object-oriented Graphics Rendering Engine Bullet Wrapper)
For the latest info, see http://www.ogre3d.org/phpBB2addons/viewforum.php?f=10

Copyright (c) 2007 tuan.kuranes@gmail.com (Use it Freely, even Statically, but have to contribute any changes)



Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
-----------------------------------------------------------------------------
*/

// Modified for use with realXtend Naali

#ifndef _OgreBulletCollisions_DEBUGLines_H_
#define _OgreBulletCollisions_DEBUGLines_H_

#include <Ogre.h>
#include "LinearMath/btIDebugDraw.h"

namespace Physics
{
    struct Line
    {
        Ogre::Vector3 _start;
        Ogre::Vector3 _end;
        Ogre::ColourValue _color;
    };
    
    //------------------------------------------------------------------------------------------------
    class  DebugLines:public Ogre::SimpleRenderable
    {
    public:
        DebugLines(void);
        ~DebugLines(void);

        void addLine (const btVector3& from, const btVector3& to, const btVector3& color)
        {
            Line newLine;
            newLine._start.x = from.x();
            newLine._start.y = from.y();
            newLine._start.z = from.z();
            newLine._end.x = to.x();
            newLine._end.y = to.y();
            newLine._end.z = to.z();
            newLine._color.r = color.x();
            newLine._color.g = color.y();
            newLine._color.b = color.z();
            newLine._color.a = 1.0f;
            _lines.push_back(newLine);
        }

        void draw ();
        void clear ();

        Ogre::Real getSquaredViewDepth (const Ogre::Camera *cam) const;
        Ogre::Real getBoundingRadius (void) const;

    protected:

        std::vector<Line> _lines;
        Ogre::HardwareVertexBufferSharedPtr _vbuf;
    };
}
#endif //_OgreBulletCollisions_DEBUGLines_H_

