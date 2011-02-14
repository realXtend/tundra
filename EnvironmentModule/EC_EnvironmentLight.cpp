// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"

#include "DebugOperatorNew.h"
#include "EC_EnvironmentLight.h"
#include "EnvironmentModule.h"

#include "EC_Placeable.h"
#include "IAttribute.h"
#include "Renderer.h"
#include "SceneManager.h"
#include "SceneEvents.h"
#include "EventManager.h"
#include "OgreMaterialUtils.h"
#include "LoggingFunctions.h"
DEFINE_POCO_LOGGING_FUNCTIONS("EC_EnvironmentLight")

#include <Ogre.h>
#include <OgreQuaternion.h>
#include <OgreColourValue.h>
#include <OgreConversionUtils.h>

#ifdef CAELUM
#include <Caelum.h>
#endif

#include "MemoryLeakCheck.h"

namespace Environment
{
    EC_EnvironmentLight::EC_EnvironmentLight(IModule *module)
        : IComponent(module->GetFramework()),
       sunColorAttr(this, "Sun color", Color(0.639f,0.639f,0.639f)),
       ambientColorAttr(this, "Ambient color", Color(0.364f, 0.364f, 0.364f, 1)),
       sunDiffuseColorAttr(this, "Sun diffuse color", Color(0.93f, 0.93f, 0.93f, 1)),
       sunDirectionAttr(this, "Sun direction vector", Vector3df(-1, -1, -1)),
       fixedTimeAttr(this, "Use fixed time", false),
       currentTimeAttr(this, "Current time", 0.67f),
       sunCastShadowsAttr(this, "Sun cast shadows", true),
       useCaelumAttr(this, "Use Caelum", true),
       sunLight_(0)
#ifdef CAELUM
       ,caelumSystem_(0)
#endif
    {
        renderer_ = framework_->GetServiceManager()->GetService<OgreRenderer::Renderer>();
#ifdef CAELUM
       caelumSystem_ =  framework_->GetModule<EnvironmentModule >()->GetCaelum();
#endif 

        static AttributeMetadata currentTimeMetaData("", "0", "100", "0.1");
        currentTimeAttr.SetMetadata(&currentTimeMetaData);

        connect(this, SIGNAL(OnAttributeChanged(IAttribute*, AttributeChange::Type)),
            SLOT(AttributeUpdated(IAttribute*, AttributeChange::Type)));

        UpdateSun();
        UpdateAmbientLight();
    }
    
    EC_EnvironmentLight::~EC_EnvironmentLight()
    {
        if (renderer_.expired())
            return;
        
        OgreRenderer::RendererPtr renderer = renderer_.lock();
       
        RemoveSun();
#ifdef CAELUM
        // Does not own
        caelumSystem_ = 0;
#endif
    }

    void EC_EnvironmentLight::UpdateSun()
    {
        if (renderer_.lock() != 0) 
        {
            Ogre::SceneManager *sceneMgr = renderer_.lock()->GetSceneManager();
            assert(sceneMgr);
            UNREFERENCED_PARAM(sceneMgr);
#ifdef CAELUM 
            if ( caelumSystem_ != 0)
            {
                Caelum::BaseSkyLight* sun = caelumSystem_->getSun();
                if ( sun != 0)
                    sun->setLightColour(OgreRenderer::ToOgreColor(sunColorAttr.Get()));
            }
#else
           if (sunLight_ != 0)
           {
               sunLight_->setDiffuseColour(OgreRenderer::ToOgreColor(sunColorAttr.Get()));
               sunLight_->setCastShadows(sunCastShadowsAttr.Get());
               sunLight_->setDirection(OgreRenderer::ToOgreVector3(sunDirectionAttr.Get()));
           }
           else
            CreateOgreLight();
#endif
           
        }
    }

    void EC_EnvironmentLight::RemoveSun()
    {
        // Remove Sun
        
        if (renderer_.expired() )
            return;
        
        OgreRenderer::RendererPtr renderer = renderer_.lock();
        if (sunLight_ != 0)
        {
            Ogre::SceneManager *sceneManager = renderer->GetSceneManager();
            sceneManager->destroyLight(sunLight_);
            sunLight_ = 0;
        }


    }
    
    void EC_EnvironmentLight::CreateOgreLight()
    {
        if (renderer_.expired())
            return;
    
        OgreRenderer::RendererPtr renderer = renderer_.lock();
    
        Ogre::SceneManager* sceneManager = renderer->GetSceneManager();
        sunLight_ = sceneManager->createLight(renderer->GetUniqueObjectName("EC_Environment_ogresunlight"));
        
        sunLight_->setType(Ogre::Light::LT_DIRECTIONAL);
        
        sunLight_->setDirection(OgreRenderer::ToOgreVector3(sunDirectionAttr.Get()));
        sunLight_->setCastShadows(sunCastShadowsAttr.Get());
        sunLight_->setDiffuseColour(OgreRenderer::ToOgreColor(sunDiffuseColorAttr.Get()));
            
        ///\todo Read parameters from config file?
        //sunLight_->setDiffuseColour(0.93f, 1, 0.13f);
        //sunLight_->setDirection(-1, -1, -1);
        //sunlight_->setCastShadows(true);
        
    
    }

    void EC_EnvironmentLight::AttributeUpdated(IAttribute* attribute, AttributeChange::Type change)
    {
        ChangeEnvironmentLight(attribute);
    }

    void EC_EnvironmentLight::UpdateAmbientLight()
    {
        if (renderer_.lock() != 0) 
        {
            Ogre::SceneManager *sceneMgr = renderer_.lock()->GetSceneManager();
            assert(sceneMgr);
            
            sceneMgr->setAmbientLight(OgreRenderer::ToOgreColor(ambientColorAttr.Get()));
           
        }

    }
    
    void EC_EnvironmentLight::Update(float frameTime)
    {
          if (!useCaelumAttr.Get() ) 
          {
                UpdateSun();
                UpdateAmbientLight();
                return;
          }

#ifdef CAELUM
        if (renderer_.expired())
            return;

        if ( caelumSystem_ == 0)
            return;

        OgreRenderer::RendererPtr renderer = renderer_.lock();
        Ogre::Camera *camera = renderer->GetCurrentCamera();
        Ogre::SceneManager *sceneManager = renderer->GetSceneManager();

        float sunColorMultiplier = 1.5f;
        float MAX_SUNLIGHT_MULTIPLIER = 1.5f;

        // Set sunlight attenuation using diffuse multiplier.
        // Seems to be working ok, but feel free to fix if you find better logic and/or values.
        Ogre::ColourValue diffuseMultiplier(sunColorMultiplier, sunColorMultiplier, sunColorMultiplier, 1);
        caelumSystem_->getSun()->setDiffuseMultiplier(diffuseMultiplier);

        Ogre::Light* sun = caelumSystem_->getSun()->getMainLight();
        Ogre::Light* moon = caelumSystem_->getMoon()->getMainLight();

        float sunDirZaxis = caelumSystem_->getSun()->getMainLight()->getDirection().z;
        if (sunDirZaxis > 0)
        {
            sunColorMultiplier -= 0.005f;
            if (sunColorMultiplier <= 0.05f)
                sunColorMultiplier = 0.05f;
        }
        else if(sunDirZaxis < 0)
        {
            sunColorMultiplier += 0.010f;
            if (sunColorMultiplier >= MAX_SUNLIGHT_MULTIPLIER)
                sunColorMultiplier = MAX_SUNLIGHT_MULTIPLIER;
        }

        // Update Caelum system.
        caelumSystem_->notifyCameraChanged(camera);
        caelumSystem_->updateSubcomponents(frameTime);
         
        // Update EC-component editor in disconnect mode. 
        
        float julDay = caelumSystem_->getUniversalClock()->getJulianDay();
        float relDayTime = fmod(julDay, 1);
        UNREFERENCED_PARAM(relDayTime);
        Ogre::Vector3 sunDir = caelumSystem_->getSunDirection(julDay);
        Vector3df direction(sunDir.x, sunDir.y, sunDir.z);
        
        // Current sun direction vector. 
        sunDirectionAttr.Set(direction, AttributeChange::Disconnected);
        
        // Current sun color
        Ogre::ColourValue color = caelumSystem_->getSunLightColour(caelumSystem_->getUniversalClock()->getJulianSecond(), sunDir);
        sunColorAttr.Set(OgreRenderer::ToCoreColor(color), AttributeChange::Disconnected);
        
        // Current ambient color
        Ogre::ColourValue ambientColor = sceneManager->getAmbientLight();
        ambientColorAttr.Set(OgreRenderer::ToCoreColor(ambientColor), AttributeChange::Disconnected);

        // Disable specular from the sun & moon for now, because it easily leads to too strong results
        sun->setSpecularColour(0.0f, 0.0f, 0.0f);
        moon->setSpecularColour(0.0f, 0.0f, 0.0f);
        
#endif
    }

    void EC_EnvironmentLight::ChangeEnvironmentLight(IAttribute* attribute)
    {
        if (  attribute == &sunColorAttr ||  
              attribute == &sunDirectionAttr ||
              attribute == &sunCastShadowsAttr )
        {
            UpdateSun();
        }
        else if ( attribute == &ambientColorAttr )
        {
            UpdateAmbientLight();
        }
        else if ( attribute == &currentTimeAttr )
        {
            UpdateTime();
        }
    }

    void EC_EnvironmentLight::UpdateTime()
    {
#ifdef CAELUM
        if ( caelumSystem_ == 0)
            return;

        float time = currentTimeAttr.Get();

        if (time < 0.0)
            time += 1.0;
        if (time >= 1.0)
            time -= 1.0;
    
        int hours = (int)(time * 24.0);
        int minutes = ((int)(time * 24.0*60.0)) % 60;
        int seconds = ((int)(time * 24.0*60.0*60.0)) % 60;
    
        // Note: we actually don't use year/month/day, because then worlds would look different
        // based on real-life time of year
        caelumSystem_->getUniversalClock()->setGregorianDateTime(
            1900, 5, 1, hours,
            minutes, seconds);
        // Do not let Caelum clock proceed on its own, authoritative time comes from server
        caelumSystem_->getUniversalClock()->setTimeScale(0);        
#endif
    }
}
