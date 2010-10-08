// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#include "EC_SkyPlane.h"

#include "Renderer.h"
#include "SceneManager.h"

#include <Ogre.h>


#include "LoggingFunctions.h"
DEFINE_POCO_LOGGING_FUNCTIONS("EC_SkyPlane")


#include "MemoryLeakCheck.h"


namespace Environment
{
     EC_SkyPlane::EC_SkyPlane(IModule *module)
        : IComponent(module->GetFramework()),
        materialAttr(this, "Material" , "Rex/SkyPlane"),
        textureAttr(this, "Texture", "clouds.jpg"),
        tilingAttr(this, "Tiling", 150.f),
        scaleAttr(this, "Scale", 300.f),
        bowAttr(this, "Bow", 1.5f),
        distanceAttr(this, "Distance", 50.f),
        xSegmentsAttr(this, "Segments in x-axis",150),
        ySegmentsAttr(this, "Segments in y-axis",150),
        drawFirstAttr(this, "Draw first", true)
     {
         QObject::connect(this, SIGNAL(OnAttributeChanged(IAttribute*, AttributeChange::Type)),
            SLOT(AttributeUpdated(IAttribute*, AttributeChange::Type)));
         
         renderer_ = module->GetFramework()->GetServiceManager()->GetService<OgreRenderer::Renderer>();

         // Disable old sky.
         //DisableSky();
         CreateSky();

         lastMaterial_ = materialAttr.Get();
         lastTiling_ = tilingAttr.Get();
         lastScale_ = scaleAttr.Get();
         lastBow_ = bowAttr.Get();
         lastDistance_ = distanceAttr.Get();
         lastxSegments_ = xSegmentsAttr.Get();
         lastySegments_ = ySegmentsAttr.Get();
         lastDrawFirst_ = drawFirstAttr.Get();
     }

    EC_SkyPlane::~EC_SkyPlane()
    {
        DisableSky();
    }

    void EC_SkyPlane::CreateSky()
    {
        if (renderer_.expired())
            return;
    
        OgreRenderer::RendererPtr renderer = renderer_.lock();  
        Ogre::SceneManager* scene_mgr = renderer->GetSceneManager();
    
        try
        {
       
            Ogre::Plane plane;
            plane.d = distanceAttr.Get();
            plane.normal = Ogre::Vector3::NEGATIVE_UNIT_Z;
            
            QString currentMaterial = materialAttr.Get();
            float scale = scaleAttr.Get();
            float tiling = tilingAttr.Get();
            bool drawFirst = drawFirstAttr.Get();
                
            if ( xSegmentsAttr.Get() == 0 || ySegmentsAttr.Get() == 0)
            {
                 LogError("Could not set SkyPlane, because x-segments or y-segments values very zero");
                 return;
            }
            
            scene_mgr->setSkyPlane(true, plane, currentMaterial.toStdString().c_str(), 
                                                scaleAttr.Get(), tilingAttr.Get(), drawFirstAttr.Get(), 
                                                bowAttr.Get(), xSegmentsAttr.Get(), ySegmentsAttr.Get());

        }
        catch (Ogre::Exception& e)
        {
            LogError("Could not set SkyPlane: " + std::string(e.what()));
            return;
        }
    }
       
    void EC_SkyPlane::AttributeUpdated(IAttribute* attribute, AttributeChange::Type change)
    {
        ChangeSkyPlane(attribute);
    }

    void EC_SkyPlane::ChangeSkyPlane(IAttribute* attribute)
    {
        std::string name = attribute->GetNameString();
        
        if ( ( name == materialAttr.GetNameString() && materialAttr.Get() != lastMaterial_ ) 
             || ( name ==  tilingAttr.GetNameString() && tilingAttr.Get() != lastTiling_ )
             || ( name ==  scaleAttr.GetNameString() && scaleAttr.Get() != lastScale_ )
             || ( name ==  bowAttr.GetNameString() && bowAttr.Get() != lastBow_ )
             || ( name ==  distanceAttr.GetNameString() && distanceAttr.Get() != lastDistance_ )
             || ( name ==  xSegmentsAttr.GetNameString() &&  xSegmentsAttr.Get() != lastxSegments_)
             || ( name == ySegmentsAttr.GetNameString() && ySegmentsAttr.Get() != lastySegments_ )
             || ( name == drawFirstAttr.GetNameString() && drawFirstAttr.Get() != lastDrawFirst_ )
             )
        {
            DisableSky();
            CreateSky();
            
            lastMaterial_ = materialAttr.Get();
            lastTiling_ = tilingAttr.Get();
            lastScale_ = scaleAttr.Get();
            lastBow_ = bowAttr.Get();
            lastDistance_ = distanceAttr.Get();
            lastxSegments_ = xSegmentsAttr.Get();
            lastySegments_ = ySegmentsAttr.Get();
            lastDrawFirst_ = drawFirstAttr.Get();

        } 
        else if ( name == textureAttr.GetNameString() )
        {
            SetTexture();
        }
      

    }

    void EC_SkyPlane::SetTexture()
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

    void EC_SkyPlane::DisableSky()
     {
        if (renderer_.expired())
            return;
        
        OgreRenderer::RendererPtr renderer = renderer_.lock();   
        
        Ogre::SceneManager* scene_mgr = renderer->GetSceneManager();
        scene_mgr->setSkyPlane(false, Ogre::Plane(), "");
        
    }

}