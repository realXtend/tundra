// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#include "EC_SkyBox.h"
#include "Renderer.h"
#include "SceneManager.h"
#include <OgreMaterialUtils.h>

#include <Ogre.h>


#include "LoggingFunctions.h"
DEFINE_POCO_LOGGING_FUNCTIONS("EC_SkyBox")


#include "MemoryLeakCheck.h"


namespace Environment
{
     EC_SkyBox::EC_SkyBox(IModule *module)
        : IComponent(module->GetFramework()),
        materialAttr(this, "Material" , "Rex/skybox"),
        textureAttr(this, "Texture"),
        orientationAttr(this, "Orientation", Quaternion(f32(M_PI/2.0), Vector3df(1.0,0.0,0.0))),
        distanceAttr(this, "Distance",50.0),
        drawFirstAttr(this, "Draw first", true)
     {
         QObject::connect(this, SIGNAL(OnAttributeChanged(IAttribute*, AttributeChange::Type)),
            SLOT(AttributeUpdated(IAttribute*, AttributeChange::Type)));
         
      
         // Find out default textures. 
               
         renderer_ = module->GetFramework()->GetServiceManager()->GetService<OgreRenderer::Renderer>();

         StringVector names;
         Ogre::MaterialPtr materialPtr = Ogre::MaterialManager::getSingleton().getByName(materialAttr.Get().toStdString().c_str());
       
         if ( materialPtr.get() != 0)
         {    
             OgreRenderer::GetTextureNamesFromMaterial(materialPtr, names);
             QVariantList lst;
             if ( names.size() == 6 )
             {
                // This code block is not currently working, but if for some reason GetTextureNamesFromMaterialn understands cubic_textures this codeblock is runned
                for ( int i = 0; i < 6; ++i)
                {
                    lst.push_back(QString(names[i].c_str()));
                }
             }
             else
             {
            
                // Add default values, hardcoded
                /// HACK use hardcoded-values because ogre textureunit state class cannot find out texture names for cubic_texture type.
                lst.push_back(QString(names[0].c_str()));
                lst.push_back(QString("rex_sky_back.dds"));
                lst.push_back(QString("rex_sky_left.dds"));
                lst.push_back(QString("rex_sky_right.dds"));
                lst.push_back(QString("rex_sky_top.dds"));
                lst.push_back(QString("rex_sky_bot.dds"));
              
             
             }
          

            textureAttr.Set(lst, AttributeChange::LocalOnly);
         }
         
        // Disable old sky.
         
        // DisableSky();
         CreateSky();

         lastMaterial_ = materialAttr.Get();
         lastOrientation_ = orientationAttr.Get();
         lastDistance_ = distanceAttr.Get();
         lastDrawFirst_ = drawFirstAttr.Get();
         lastTextures_ = textureAttr.Get();
     }

    EC_SkyBox::~EC_SkyBox()
    {
        DisableSky();
        renderer_.reset();
    }

    void EC_SkyBox::CreateSky()
    {
        if (renderer_.expired())
            return;
    
        OgreRenderer::RendererPtr renderer = renderer_.lock();  
        Ogre::SceneManager* scene_mgr = renderer->GetSceneManager();
        
        QString currentMaterial = materialAttr.Get();

        Ogre::MaterialPtr materialPtr = Ogre::MaterialManager::getSingleton().getByName(currentMaterial.toStdString().c_str());
        materialPtr->setReceiveShadows(false);
        
        try
        {
            //RexTypes::Vector3 v = angleAxisAttr.Get();
            //Ogre::Quaternion rotation(Ogre::Degree(90.0), Ogre::Vector3(1, 0, 0));
           
            Quaternion orientation = orientationAttr.Get();
            Ogre::Quaternion rotation(orientation.w, orientation.x, orientation.y, orientation.z); 
            scene_mgr->setSkyBox(true, currentMaterial.toStdString().c_str(), distanceAttr.Get(), drawFirstAttr.Get(), rotation);
        }
        catch (Ogre::Exception& e)
        {
            LogError("Could not set SkyBox: " + std::string(e.what()));
            return;
        }
      
        
    }
       
    void EC_SkyBox::AttributeUpdated(IAttribute* attribute, AttributeChange::Type change)
    {
        ChangeSkyBox(attribute);
    }

    void EC_SkyBox::ChangeSkyBox(IAttribute* attribute)
    {
        std::string name = attribute->GetNameString();
        
        if ( ( name == materialAttr.GetNameString() && materialAttr.Get() != lastMaterial_ ) 
             || ( name ==  distanceAttr.GetNameString() && distanceAttr.Get() != lastDistance_ )
             || ( name == drawFirstAttr.GetNameString() && drawFirstAttr.Get() != lastDrawFirst_ )
             )
        {
            DisableSky();
            CreateSky();
            
            lastMaterial_ = materialAttr.Get();
            lastDistance_ = distanceAttr.Get();
            lastDrawFirst_ = drawFirstAttr.Get();

        } 
        else if ( name == textureAttr.GetNameString() )
        {
            // What texture has changed?
            SetTextures();
        
        }
      

    }

    void EC_SkyBox::SetTextures()
    {
        QVariantList lst = textureAttr.Get();
        std::vector<std::string> texture_names;
        texture_names.reserve(6);

        for ( int i = 0; i < lst.size() && i <= 6; ++i)
        {
            texture_names.push_back(lst[i].toString().toStdString());
        }

        
        Ogre::MaterialPtr materialPtr = Ogre::MaterialManager::getSingleton().getByName(materialAttr.Get().toStdString().c_str());
        
        if (!materialPtr.isNull() && texture_names.size() == 6)
        {
                materialPtr->getTechnique(0)->getPass(0)->getTextureUnitState(0)->setCubicTextureName(&texture_names[0], false);
                //skyMaterial->getTechnique(0)->getPass(0)->getTextureUnitState(0)->setTextureScale(1, -1);
              
               
        }
        else if ( !materialPtr.isNull() )
        {
             
            for ( int i = 0; i < texture_names.size(); ++i)
            {
                materialPtr->getTechnique(0)->getPass(0)->getTextureUnitState(0)->setFrameTextureName(Ogre::String(texture_names[i].c_str()), i);
            }
        }
         
        DisableSky();            
        CreateSky();
    
    }


    void EC_SkyBox::DisableSky()
     {
        if (renderer_.expired())
            return;
        
        OgreRenderer::RendererPtr renderer = renderer_.lock();   
        
        Ogre::SceneManager* scene_mgr = renderer->GetSceneManager();
        scene_mgr->setSkyBox(false, "");
        
    }

}