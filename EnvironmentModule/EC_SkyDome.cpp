// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#include "EC_SkyDome.h"

#include "Renderer.h"
#include "SceneManager.h"
#include "LoggingFunctions.h"

DEFINE_POCO_LOGGING_FUNCTIONS("EC_SkyDome")

#include <Ogre.h>

#include "MemoryLeakCheck.h"

namespace Environment
{
    EC_SkyDome::EC_SkyDome(IModule *module) :
        IComponent(module->GetFramework()),
        materialAttr(this, "Material" , "Rex/SkyPlane"),
        textureAttr(this, "Texture", "clouds.jpg"),
        distanceAttr(this, "Distance", 50.f),
        curvatureAttr(this, "Curvature", 10.f),
        tilingAttr(this, "Tiling", 8.f),
        xSegmentsAttr(this, "Segments in x-axis",16),
        ySegmentsAttr(this, "Segments in y-axis",16),
        orientationAttr(this, "Orientation", Quaternion(f32(M_PI/2.0), Vector3df(1.0,0.0,0.0))),
        ySegmentsKeepAttr(this, "Y-segments keep", -1),
        drawFirstAttr(this, "Draw first", true)
    {
        connect(this, SIGNAL(OnAttributeChanged(IAttribute*, AttributeChange::Type)), SLOT(AttributeUpdated(IAttribute*)));

        renderer_ = module->GetFramework()->GetServiceManager()->GetService<OgreRenderer::Renderer>();

        CreateSky();

        lastMaterial_ = materialAttr.Get();
        lastTexture_ = textureAttr.Get();
        lastTiling_ = tilingAttr.Get();
        lastDistance_ = distanceAttr.Get();
        lastCurvature_ = curvatureAttr.Get();
        lastySegmentsKeep_ = ySegmentsKeepAttr.Get();
        lastxSegments_ = xSegmentsAttr.Get();
        lastySegments_ = ySegmentsAttr.Get();
        lastDrawFirst_ = drawFirstAttr.Get();
    }

    EC_SkyDome::~EC_SkyDome()
    {
        DisableSky();
    }

    void EC_SkyDome::CreateSky()
    {
        if (renderer_.expired())
            return;

        OgreRenderer::RendererPtr renderer = renderer_.lock();
        try
        {
            QString currentMaterial = materialAttr.Get();
            if  (xSegmentsAttr.Get() == 0 || ySegmentsAttr.Get() == 0)
            {
                 LogError("Could not set SkyDome, because x-segments or y-segments values was zero");
                 return;
            }
            
            Quaternion orientation = orientationAttr.Get();
            Ogre::Quaternion rotation(orientation.w, orientation.x, orientation.y, orientation.z); 

            renderer->GetSceneManager()->setSkyDome(true,currentMaterial.toStdString().c_str(), curvatureAttr.Get(),tilingAttr.Get(),
                distanceAttr.Get(), drawFirstAttr.Get(), rotation, xSegmentsAttr.Get(), ySegmentsAttr.Get(), -1);
        }
        catch (Ogre::Exception& e)
        {
            LogError("Could not set SkyDome: " + std::string(e.what()));
            return;
        }
    }

    void EC_SkyDome::AttributeUpdated(IAttribute* attr)
    {
        if ((attr == &materialAttr && materialAttr.Get() != lastMaterial_ ) ||
            (attr ==  &tilingAttr && tilingAttr.Get() != lastTiling_ ) ||
            (attr == &curvatureAttr && curvatureAttr.Get() != lastCurvature_) ||
            (attr == &distanceAttr && distanceAttr.Get() != lastDistance_) ||
            (attr == &xSegmentsAttr &&  xSegmentsAttr.Get() != lastxSegments_) ||
            (attr == &ySegmentsAttr && ySegmentsAttr.Get() != lastySegments_) ||
            (attr == &drawFirstAttr && drawFirstAttr.Get() != lastDrawFirst_) ||
            (attr == &ySegmentsKeepAttr && ySegmentsKeepAttr.Get() != lastySegmentsKeep_))
        {
            DisableSky();
            CreateSky();

            lastMaterial_ = materialAttr.Get();
            lastTexture_ = textureAttr.Get();
            lastTiling_ = tilingAttr.Get();
            lastDistance_ = distanceAttr.Get();
            lastCurvature_ = curvatureAttr.Get();
            lastxSegments_ = xSegmentsAttr.Get();
            lastySegments_ = ySegmentsAttr.Get();
            lastDrawFirst_ = drawFirstAttr.Get();
            lastySegmentsKeep_ = ySegmentsKeepAttr.Get();
        }
        else if(attr == &textureAttr)
        {
            SetTexture();
        }
    }

    void EC_SkyDome::SetTexture()
    {
        QString currentMaterial = materialAttr.Get();
        QString textureName = textureAttr.Get();

        Ogre::MaterialPtr materialPtr = Ogre::MaterialManager::getSingleton().getByName(currentMaterial.toStdString().c_str());
        if (!materialPtr.isNull())
        {
            if ( materialPtr->getNumTechniques() >= 1 && materialPtr->getTechnique(0)->getNumPasses() >= 1 &&  materialPtr->getTechnique(0)->getPass(0)->getNumTextureUnitStates() >= 1)
            {
                materialPtr->getTechnique(0)->getPass(0)->getTextureUnitState(0)->setTextureName(textureName.toStdString().c_str());
                materialPtr->getTechnique(0)->getPass(0)->getTextureUnitState(0)->setTextureAddressingMode(Ogre::TextureUnitState::TAM_WRAP);
            }
            else
                return;

            DisableSky();
            CreateSky();
        }
    }

    void EC_SkyDome::DisableSky()
     {
        if (renderer_.expired())
            return;
        
        OgreRenderer::RendererPtr renderer = renderer_.lock();   
        
        Ogre::SceneManager* scene_mgr = renderer->GetSceneManager();
        scene_mgr->setSkyDome(false, "");
        
    }

}