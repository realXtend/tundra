// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "DebugOperatorNew.h"
#include "EC_WaterPlane.h"
#include "EC_Placeable.h"
#include "IAttribute.h"

#include "Renderer.h"
#include "SceneManager.h"
#include "SceneEvents.h"
#include "EventManager.h"
#include <OgreMaterialUtils.h>
#include "LoggingFunctions.h"
DEFINE_POCO_LOGGING_FUNCTIONS("EC_WaterPlane")

#include <Ogre.h>
#include <OgreQuaternion.h>
#include <OgreColourValue.h>
#include <OgreMath.h>
// NaN - check
#include <RexNetworkUtils.h>

#include "MemoryLeakCheck.h"


namespace Environment
{
   
    EC_WaterPlane::EC_WaterPlane(IModule *module)
        : IComponent(module->GetFramework()),
        xSizeAttr(this, "x-size", 5000),
        ySizeAttr(this, "y-size", 5000),
        depthAttr(this, "Depth", 20),
        positionAttr(this, "Position", Vector3df()),
        rotationAttr(this, "Rotation", Quaternion()),
        scaleUfactorAttr(this, "U factor", 0.0002f),
        scaleVfactorAttr(this, "V factor", 0.0002f),
        xSegmentsAttr(this, "Segments in x", 10),
        ySegmentsAttr(this, "Segments in y", 10),
        materialNameAttr(this, "Material", QString("Ocean")),
       //textureNameAttr(this, "Texture", QString("DefaultOceanSkyCube.dds")),
        fogColorAttr(this, "Fog color", Color(0.2f,0.4f,0.35f,1.0f)),
        fogStartAttr(this, "Fog start dist.", 100.f),
        fogEndAttr(this, "Fog end dist.", 2000.f),
        fogModeAttr(this, "Fog mode", 3),
        entity_(0),
        node_(0),
        attached_(false),
        attachedToRoot_(false)
    {
        static AttributeMetadata metadata;
        static bool metadataInitialized = false;
    
        if(!metadataInitialized)
        {
            metadata.enums[Ogre::FOG_NONE] = "NoFog";
            metadata.enums[Ogre::FOG_EXP] = "Exponential";
            metadata.enums[Ogre::FOG_EXP2] = "ExponentiallySquare";
            metadata.enums[Ogre::FOG_LINEAR] = "Linear";
         
            metadataInitialized = true;
        }

        fogModeAttr.SetMetadata(&metadata);

        renderer_ = framework_->GetServiceManager()->GetService<OgreRenderer::Renderer>();
        if(!renderer_.expired())
        {
            Ogre::SceneManager* scene_mgr = renderer_.lock()->GetSceneManager();
            node_ = scene_mgr->createSceneNode();
        }

        QObject::connect(this, SIGNAL(OnAttributeChanged(IAttribute*, AttributeChange::Type)),
            SLOT(AttributeUpdated(IAttribute*, AttributeChange::Type)));

        lastXsize_ = xSizeAttr.Get();
        lastYsize_ = ySizeAttr.Get();
        
        CreateWaterPlane();
        QObject::connect(this, SIGNAL(ParentEntitySet()), this, SLOT(AttachEntity()));
        
        // If there exist placeable copy its position for default position and rotation.
       
        EC_Placeable* placeable = dynamic_cast<EC_Placeable*>(FindPlaceable().get());
        if ( placeable != 0)
        {
            Vector3df vec = placeable->GetPosition();
            positionAttr.Set(vec,AttributeChange::Default);
       
            Quaternion rot =placeable->GetOrientation();
            rotationAttr.Set(rot, AttributeChange::Default);
            ComponentChanged(AttributeChange::Default);
        }
    }
    
    EC_WaterPlane::~EC_WaterPlane()
    {
       if (renderer_.expired())
        return;
        
       OgreRenderer::RendererPtr renderer = renderer_.lock();
       RemoveWaterPlane();
    
        if (node_ != 0)
        {
            Ogre::SceneManager* scene_mgr = renderer->GetSceneManager();
            scene_mgr->destroySceneNode(node_);
            node_ = 0;
        }
    }

 

    bool EC_WaterPlane::IsUnderWater()
    {
      // Check that is camera inside of defined waterplane. 
      ///todo this cannot be done this way, mesh orientation etc. must take care. Now we just assume that plane does not have orientation.

      if ( entity_ == 0)
        return false;

      Ogre::Camera *camera = renderer_.lock()->GetCurrentCamera();
      Ogre::Vector3 posCamera = camera->getDerivedPosition();
      
      Ogre::Vector3 pos;
    
      if (node_ != 0)
          pos = node_->_getDerivedPosition();  
      else
          return false;

      int xSize = xSizeAttr.Get(), ySize = ySizeAttr.Get(), depth = depthAttr.Get();
      int x = posCamera.x, y = posCamera.y, z = posCamera.z;
    
      // HACK this is strange, i thought that it should be 0.5 but some reason, visually it looks like that you can travel really "outside" from water. 
      int tmpMax = pos.x + 0.25*xSize;
      int tmpMin = pos.x - 0.25*xSize;
          
      if ( x >= tmpMin && x <= tmpMax )
        {
            tmpMax = pos.y + 0.25*ySize;
            tmpMin = pos.y - 0.25*ySize;
        
            if ( y >= tmpMin && y <= tmpMax)
            {
                tmpMax = pos.z;
                tmpMin = pos.z - depth;
            
                if ( z >= tmpMin && z <= tmpMax)
                {
                    return true;
                }
                else
                    return false;
            }
            else
                return false;
         }
      
         return false;
    }
    
    void EC_WaterPlane::CreateWaterPlane()
    {
        // Create waterplane    

        if (renderer_.lock() != 0) 
        {
            Ogre::SceneManager *sceneMgr = renderer_.lock()->GetSceneManager();
            assert(sceneMgr);
           
            if (node_ != 0)
            {

                int xSize = xSizeAttr.Get();
                int ySize = ySizeAttr.Get();
                float uTile =  scaleUfactorAttr.Get() * xSize; /// Default x-size 5000 --> uTile 1.0
                float vTile =  scaleVfactorAttr.Get() * ySize;
                
                Ogre::MeshPtr mesh = Ogre::MeshManager::getSingleton().createPlane(name_.toStdString().c_str(), Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, 
                    Ogre::Plane(Ogre::Vector3::UNIT_Z, 0),xSize, ySize, xSegmentsAttr.Get(), ySegmentsAttr.Get(), true, 1, uTile, vTile, Ogre::Vector3::UNIT_X);
                
                entity_ = sceneMgr->createEntity(renderer_.lock()->GetUniqueObjectName(), name_.toStdString().c_str());
                entity_->setMaterialName(materialNameAttr.Get().toStdString().c_str());
                entity_->setCastShadows(false);
                // Tries to attach entity, if there is not EC_Placeable availible, it will not attach object
                AttachEntity();
               
            }
        }

      
    }

    void EC_WaterPlane::RemoveWaterPlane()
    {
        // Remove waterplane
        
        if (renderer_.expired() || !entity_)
            return;
        
        OgreRenderer::RendererPtr renderer = renderer_.lock();

        DetachEntity();
        
        Ogre::SceneManager* scene_mgr = renderer->GetSceneManager();
        scene_mgr->destroyEntity(entity_);
        entity_ = 0;
        
        Ogre::MeshManager::getSingleton().remove(name_.toStdString().c_str());


    }

    Ogre::ColourValue EC_WaterPlane::GetFogColorAsOgreValue() const
    {
        Color col = fogColorAttr.Get();
        return Ogre::ColourValue(col.r, col.g, col.b, col.a);

    }

    void EC_WaterPlane::AttributeUpdated(IAttribute* attribute, AttributeChange::Type change)
    {
        ChangeWaterPlane(attribute);
    }

    void EC_WaterPlane::SetPosition()
    {
        // Is attached?
        if ( entity_ != 0 && !entity_->isAttached() && !attached_ )
        {
            Ogre::SceneManager* scene_mgr = renderer_.lock()->GetSceneManager();
            node_->attachObject(entity_);
            attached_ = true;
            attachedToRoot_ = true;
            scene_mgr->getRootSceneNode()->addChild(node_);
            node_->setVisible(true);
            
        }
        
        Vector3df vec = positionAttr.Get();
        //node_->setPosition(vec.x, vec.y, vec.z);

#if OGRE_VERSION_MINOR <= 6 && OGRE_VERSION_MAJOR <= 1
        Ogre::Vector3 current_pos = node_->_getDerivedPosition();
        Ogre::Vector3 tmp(vec.x,vec.y,vec.z);
        tmp = current_pos + tmp;
        
        Vector3df pos(tmp.x, tmp.y, tmp.z);
        if ( !RexTypes::IsValidPositionVector(pos) )
            return;
       
        node_->setPosition(tmp);
#else
        Ogre::Vector3 pos(vec.x, vec.y, vec.z);
        if ( !RexTypes::IsValidPositionVector(vec) )
            return;
        

        node_->_setDerivedPosition(pos);
#endif

    }

    void EC_WaterPlane::SetOrientation()
    {
        // Is attached?
        if ( entity_ != 0 && !entity_->isAttached() && attached_  )
        {
            Ogre::SceneManager* scene_mgr = renderer_.lock()->GetSceneManager();
            node_->attachObject(entity_);
            attached_ = true;
            attachedToRoot_ = true;
            scene_mgr->getRootSceneNode()->addChild(node_);
            node_->setVisible(true);
            
        }

         // Set orientation
        Quaternion rot = rotationAttr.Get();

#if OGRE_VERSION_MINOR <= 6 && OGRE_VERSION_MAJOR <= 1
        Ogre::Quaternion current_rot = node_->_getDerivedOrientation();
        Ogre::Quaternion tmp(rot.w, rot.x, rot.y, rot.z);
        Ogre::Quaternion rota = current_rot +  tmp;
        node_->setOrientation(rota);
        
#else
        node_->_setDerivedOrientation(Ogre::Quaternion(rot.w, rot.x, rot.y, rot.z));
#endif
    }

    void EC_WaterPlane::ChangeWaterPlane(IAttribute* attribute)
    {
        std::string name = attribute->GetNameString();
        if ( ( name == xSizeAttr.GetNameString() 
          || name == ySizeAttr.GetNameString()
          || name == scaleUfactorAttr.GetNameString()
          || name == scaleVfactorAttr.GetNameString() ) && 
          ( lastXsize_ != xSizeAttr.Get() || lastYsize_ != ySizeAttr.Get() ) )
        {
            RemoveWaterPlane();
            CreateWaterPlane();
            
            lastXsize_ = xSizeAttr.Get();
            lastYsize_ = ySizeAttr.Get();

        }
        else if ( name == xSegmentsAttr.GetNameString() || name == ySegmentsAttr.GetNameString() )
        {
            RemoveWaterPlane();
            CreateWaterPlane();
        }
        else if ( name == positionAttr.GetNameString() )
        {
            // Change position
            SetPosition();
           
        }
        else if ( name == rotationAttr.GetNameString() )
        {
            // Change rotation

            // Is there placeable component? If not use given rotation 
            //if ( dynamic_cast<EC_Placeable*>(FindPlaceable().get()) == 0 )
            //{
               SetOrientation();
            //}
            
        }
        else if ( name == depthAttr.GetNameString() )
        {
            // Change depth
            // Currently do nothing..
           
        }
        else if ( name ==  materialNameAttr.GetNameString())
        {
            //Change material
            if ( entity_ != 0)
            {
                entity_->setMaterialName(materialNameAttr.Get().toStdString().c_str());
            }
        }
        /*
        // Currently commented out, working feature but not enabled yet.
        else if (name == textureNameAttr.GetNameString() )
        {

            QString currentMaterial = materialNameAttr.Get();
            
            // Check that has texture really changed. 
            
            StringVector names;
            Ogre::MaterialPtr materialPtr = Ogre::MaterialManager::getSingleton().getByName(currentMaterial.toStdString().c_str());
            
            if ( materialPtr.get() == 0)
                return;

            OgreRenderer::GetTextureNamesFromMaterial(materialPtr, names);
            
            QString textureName = textureNameAttr.Get();
            
            for (StringVector::iterator iter = names.begin(); iter != names.end(); ++iter)
            {
                QString currentTextureName(iter->c_str());
                if ( currentTextureName == textureName)
                    return;
            }

            // So texture has really changed, let's change it. 
            OgreRenderer::SetTextureUnitOnMaterial(materialPtr, textureName.toStdString(), 0);

        }
        */
        
        
    }


    ComponentPtr EC_WaterPlane::FindPlaceable() const
    {
        assert(framework_);
        ComponentPtr comp;
        if(!GetParentEntity())
            return comp;
        comp = GetParentEntity()->GetComponent<EC_Placeable>();
        return comp;

    }

   
    void EC_WaterPlane::AttachEntity()
    {
        EC_Placeable* placeable = dynamic_cast<EC_Placeable*>(FindPlaceable().get());
        if ((!entity_) || (!placeable) || attached_)
            return;

        Ogre::SceneNode* node = placeable->GetSceneNode();
        node->addChild(node_);
        node_->attachObject(entity_);

      
        attached_ = true;

    }
      
    void EC_WaterPlane::DetachEntity()
    {
        EC_Placeable* placeable = dynamic_cast<EC_Placeable*>(FindPlaceable().get());
        if ((!attached_) || (!entity_) || (!placeable))
            return;

        if ( attachedToRoot_ )
        {
            Ogre::SceneManager* scene_mgr = renderer_.lock()->GetSceneManager();
            node_->detachObject(entity_);
            attached_ = false;
            attachedToRoot_ = false;
            scene_mgr->getRootSceneNode()->removeChild(node_);
            return;
            
        }

        Ogre::SceneNode* node = placeable->GetSceneNode();
        node_->detachObject(entity_);
        node->removeChild(node_);

        attached_ = false;

    }

}