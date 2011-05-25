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

// Modified for use with realXtend Tundra.

#include "StableHeaders.h"
#include "DebugOperatorNew.h"
#include "MemoryLeakCheck.h"
#include "OgreBulletCollisionsDebugLines.h"

using namespace Ogre;

//------------------------------------------------------------------------------------------------
DebugLines::DebugLines() : SimpleRenderable()
{
	mRenderOp.vertexData = new Ogre::VertexData();
	mRenderOp.indexData = 0;
	mRenderOp.vertexData->vertexCount = 0;
	mRenderOp.vertexData->vertexStart = 0;
	mRenderOp.operationType = RenderOperation::OT_LINE_LIST;
	mRenderOp.useIndexes = false;

    setCastShadows (false);
    this->setMaterial("PhysicsDebug");
}


//------------------------------------------------------------------------------------------------
void DebugLines::clear()
{
    _lines.clear();
}

//------------------------------------------------------------------------------------------------
DebugLines::~DebugLines(void)
{ 
    clear();

    delete mRenderOp.vertexData;
}
//------------------------------------------------------------------------------------------------
void DebugLines::draw()
{
    if (_lines.empty())
    {
        mRenderOp.vertexData->vertexCount = 0;
        return;
    }
    
    // Initialization stuff
    mRenderOp.vertexData->vertexCount = _lines.size() * 2;

    Ogre::VertexDeclaration *decl = mRenderOp.vertexData->vertexDeclaration;
    Ogre::VertexBufferBinding *bind = mRenderOp.vertexData->vertexBufferBinding;

	if (_vbuf.isNull())
	{
		decl->addElement(0, 0, VET_FLOAT3, VES_POSITION);
		decl->addElement(0, 12, VET_COLOUR, VES_DIFFUSE);

		_vbuf = HardwareBufferManager::getSingleton().createVertexBuffer(
			decl->getVertexSize(0),
			mRenderOp.vertexData->vertexCount,
			HardwareBuffer::HBU_DYNAMIC_WRITE_ONLY_DISCARDABLE);

		bind->setBinding(0, _vbuf);
	}
	else
	{
	    if (_vbuf->getNumVertices() != mRenderOp.vertexData->vertexCount)
	    {
		    bind->unsetAllBindings();

		    _vbuf = HardwareBufferManager::getSingleton().createVertexBuffer(
		    	decl->getVertexSize(0),
		    	mRenderOp.vertexData->vertexCount,
		    	HardwareBuffer::HBU_DYNAMIC_WRITE_ONLY_DISCARDABLE);

    		bind->setBinding(0, _vbuf);
    	}
	}

    // Drawing stuff
    unsigned int size = (unsigned int)_lines.size();
    Ogre::Vector3 vaabMin = _lines[0]._start;
    Ogre::Vector3 vaabMax = _lines[0]._start;

    float *prPos = static_cast<float*>(_vbuf->lock(HardwareBuffer::HBL_DISCARD));

    Ogre::RenderSystem* rs = Root::getSingleton().getRenderSystem();
    Ogre::VertexElementType vet = VET_COLOUR_ARGB;
    if (rs)
        vet = rs->getColourVertexElementType();

    for(unsigned int i = 0; i < size; i++)
    {
        const Line& line = _lines[i];
        uint32 packedColor;
        if (vet == VET_COLOUR_ARGB)
            packedColor = line._color.getAsARGB();
        else
            packedColor = line._color.getAsABGR();
        
        *prPos++ = line._start.x;
        *prPos++ = line._start.y;
        *prPos++ = line._start.z;
        *((uint32*)prPos) = packedColor;
        prPos++;
        *prPos++ = line._end.x;
        *prPos++ = line._end.y;
        *prPos++ = line._end.z;
        *((uint32*)prPos) = packedColor;
        prPos++;
        
        if (line._start.x < vaabMin.x)
			vaabMin.x = line._start.x;
		else if (line._start.x > vaabMax.x)
			vaabMax.x = line._start.x;

        if (line._start.y < vaabMin.y)
			vaabMin.y = line._start.y;
		else if (line._start.y > vaabMax.y)
			vaabMax.y = line._start.y;

        if (line._start.z < vaabMin.z)
			vaabMin.z = line._start.z;
		else if (line._start.z > vaabMax.z)
			vaabMax.z = line._start.z;
		
        if (line._end.x < vaabMin.x)
			vaabMin.x = line._end.x;
		else if (line._end.x > vaabMax.x)
			vaabMax.x = line._end.x;

        if (line._end.y < vaabMin.y)
			vaabMin.y = line._end.y;
		else if (line._end.y > vaabMax.y)
			vaabMax.y = line._end.y;

        if (line._end.z < vaabMin.z)
			vaabMin.z = line._end.z;
		else if (line._end.z > vaabMax.z)
			vaabMax.z = line._end.z;
    }

    _vbuf->unlock();

    mBox.setInfinite();
	//mBox.Extents(vaabMin, vaabMax);
	
	clear();
}
//------------------------------------------------------------------------------------------------
Real DebugLines::getSquaredViewDepth(const Camera *cam) const
{
    Vector3 vMin, vMax, vMid, vDist;
    vMin = mBox.getMinimum();
    vMax = mBox.getMaximum();
    vMid = ((vMin - vMax) * 0.5) + vMin;
    vDist = cam->getDerivedPosition() - vMid;

    return vDist.squaredLength();
}
//------------------------------------------------------------------------------------------------
Real DebugLines::getBoundingRadius() const
{
    return Math::Sqrt(std::max(mBox.getMaximum().squaredLength(), mBox.getMinimum().squaredLength()));
}

