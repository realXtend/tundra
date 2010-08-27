// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "EC_Terrain.h"

#include "Renderer.h"
#include "ModuleInterface.h"
#include "ServiceManager.h"

#include <Ogre.h>

namespace Environment
{

EC_Terrain::EC_Terrain(Foundation::ModuleInterface* module) :
    Foundation::ComponentInterface(module->GetFramework())
{
}

EC_Terrain::~EC_Terrain()
{
    Destroy();
}

void EC_Terrain::Destroy()
{
    assert(GetFramework());
    if (!GetFramework())
        return;

    boost::shared_ptr<OgreRenderer::Renderer> renderer = GetFramework()->GetServiceManager()->GetService<OgreRenderer::Renderer>().lock();
    if (!renderer) // Oops! Inconvenient dtor order - can't delete our own stuff since we can't get an instance to the owner.
        return;

    Ogre::SceneManager *sceneMgr = renderer->GetSceneManager();
    if (!sceneMgr) // Oops! Same as above.
        return;

    for(int y = 0; y < cNumPatchesPerEdge; ++y)
        for(int x = 0; x < cNumPatchesPerEdge; ++x)
        {
            Ogre::SceneNode *node = GetPatch(x, y).node;
            if (!node)
                continue;

            sceneMgr->getRootSceneNode()->removeChild(node);
//                sceneMgr->destroyManualObject(dynamic_cast<Ogre::ManualObject*>(node->getAttachedObject(0)));
            node->detachAllObjects();
            sceneMgr->destroySceneNode(node);
        }
}

float EC_Terrain::GetPoint(int x, int y) const
{
    if (x < 0)
        x = 0;
    if (y < 0)
        y = 0;
    if (x >= cNumPatchesPerEdge * 16)
        x = cNumPatchesPerEdge * 16 - 1;
    if (y >= cNumPatchesPerEdge * 16)
        y = cNumPatchesPerEdge * 16 - 1;

    return GetPatch(x / cPatchSize, y / cPatchSize).heightData[(y % cPatchSize) * cPatchSize + (x % cPatchSize)];
}

float EC_Terrain::InterpolateHeightValue(float x, float y)
{
    int xFloor = (int)floor(x);
    int xCeil = (int)ceil(x);
    int yFloor = (int)floor(y);
    int yCeil = (int)ceil(y);

    const int cMin = 0;
    const int cMax = 256;

    xFloor = clamp(xFloor, cMin, cMax);
    xCeil = clamp(xCeil, cMin, cMax);
    yFloor = clamp(yFloor, cMin, cMax);
    yCeil = clamp(yCeil, cMin, cMax);

    float xFrac = fmod(x, 1.f);
    float yFrac = fmod(y, 1.f);
    float h1;
    if (xFrac + yFrac >= 1.f)
    {
        //if xFrac >= yFrac
        h1 = GetPoint(xCeil, yCeil);
        xFrac = 1.f - xFrac;
        yFrac = 1.f - yFrac;
    }
    else
        h1 = GetPoint(xFloor, yFloor);

    float h2 = GetPoint(xCeil, yFloor);
    float h3 = GetPoint(xFloor, yCeil);
    return h1 * (1.f - xFrac - yFrac) + h2 * xFrac + h3 * yFrac;
}

Vector3df EC_Terrain::CalculateNormal(int x, int y, int xinside, int yinside)
{
    int px = x * cPatchSize + xinside;
    int py = y * cPatchSize + yinside;

    int xNext = clamp(px+1, 0, cNumPatchesPerEdge * Patch::cNumVerticesPerPatchEdge - 1);
    int yNext = clamp(py+1, 0, cNumPatchesPerEdge * Patch::cNumVerticesPerPatchEdge - 1);
    int xPrev = clamp(px-1, 0, cNumPatchesPerEdge * Patch::cNumVerticesPerPatchEdge - 1);
    int yPrev = clamp(py-1, 0, cNumPatchesPerEdge * Patch::cNumVerticesPerPatchEdge - 1);

    float x_slope = GetPoint(xPrev, py) - GetPoint(xNext, py);
    if ((px <= 0) || (px >= cNumPatchesPerEdge * Patch::cNumVerticesPerPatchEdge))
        x_slope *= 2;
    float y_slope = GetPoint(px, yPrev) - GetPoint(px, yNext);
    if ((py <= 0) || (py >= cNumPatchesPerEdge * Patch::cNumVerticesPerPatchEdge))
        y_slope *= 2;

    Vector3df normal(x_slope, y_slope, 2.0);
    normal.normalize();
    return normal;
}

}
