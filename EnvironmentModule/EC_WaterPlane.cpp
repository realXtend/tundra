// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "DebugOperatorNew.h"
#include "EC_WaterPlane.h"
#include "EC_OgrePlaceable.h"
#include "IAttribute.h"

#include "Renderer.h"
#include "SceneManager.h"
#include "SceneEvents.h"
#include "EventManager.h"
#include "LoggingFunctions.h"
DEFINE_POCO_LOGGING_FUNCTIONS("EC_WaterPlane")

#include <Ogre.h>
#include <OgreQuaternion.h>
#include <OgreColourValue.h>

#include "MemoryLeakCheck.h"


namespace Environment
{
   
    EC_WaterPlane::EC_WaterPlane(IModule *module)
        : IComponent(module->GetFramework()),
        xSizeAttr_(this, "x-size", 5000),
        ySizeAttr_(this, "y-size", 5000),
        depthAttr_(this, "Depth", 20),
        positionAttr_(this, "Position", Vector3df()),
        rotationAttr_(this, "Rotation", Quaternion()),
        scaleUfactorAttr_(this, "U factor", 0.0002f),
        scaleVfactorAttr_(this, "V factor", 0.0002f),
        xSegmentsAttr_(this, "Segments x direction", 10),
        ySegmentsAttr_(this, "Segments y direction", 10),
        materialNameAttr_(this, "Material", QString("Ocean")),
        fogColorAttr_(this, "Fog color", Color(0.2f,0.4f,0.35f,1.0f)),
        fogStartAttr_(this, "Fog start distance", 100.f),
        fogEndAttr_(this, "Fog end distance", 2000.f),
        fogModeAttr_(this, "Fog mode", 3),
        entity_(0),
        node_(0),
        attached_(false)
    {
        static AttributeMetadata metadata;
        static bool metadataInitialized = false;
    
        if(!metadataInitialized)
        {
            metadata.enums[Ogre::FOG_NONE] = "NoFog";
            metadata.enums[Ogre::FOG_EXP] = "Exponentially";
            metadata.enums[Ogre::FOG_EXP2] = "ExponentiallySquare";
            metadata.enums[Ogre::FOG_LINEAR] = "Linearly";
         
            metadataInitialized = true;
        }

        fogModeAttr_.SetMetadata(&metadata);

        renderer_ = framework_->GetServiceManager()->GetService<OgreRenderer::Renderer>();
        if(!renderer_.expired())
        {
            Ogre::SceneManager* scene_mgr = renderer_.lock()->GetSceneManager();
            node_ = scene_mgr->createSceneNode();
        }

        QObject::connect(this, SIGNAL(OnAttributeChanged(IAttribute*, AttributeChange::Type)),
            SLOT(AttributeUpdated(IAttribute*, AttributeChange::Type)));

        lastXsize_ = xSizeAttr_.Get();
        lastYsize_ = ySizeAttr_.Get();
        
        CreateWaterPlane();
        QObject::connect(this, SIGNAL(ParentEntitySet()), this, SLOT(AttachEntity()));
        
        // If there exist placeable copy its position for default position and rotation.
       
        OgreRenderer::EC_OgrePlaceable* placeable = dynamic_cast<OgreRenderer::EC_OgrePlaceable*>(FindPlaceable().get());
        if ( placeable != 0)
        {
            Vector3df vec = placeable->GetPosition();
            positionAttr_.Set(vec,AttributeChange::Local);
       
            Quaternion rot =placeable->GetOrientation();
            rotationAttr_.Set(rot, AttributeChange::Local);
            ComponentChanged(AttributeChange::Local);
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

      int xSize = xSizeAttr_.Get(), ySize = ySizeAttr_.Get(), depth = depthAttr_.Get();
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

                int xSize = xSizeAttr_.Get();
                int ySize = ySizeAttr_.Get();
                float uTile =  scaleUfactorAttr_.Get() * xSize; /// Default x-size 5000 --> uTile 1.0
                float vTile =  scaleVfactorAttr_.Get() * ySize;

                Ogre::MeshPtr mesh = Ogre::MeshManager::getSingleton().createPlane(name_.toStdString().c_str(), Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, 
                    Ogre::Plane(Ogre::Vector3::UNIT_Z, 0),xSize, ySize, xSegmentsAttr_.Get(), ySegmentsAttr_.Get(), true, 1, uTile, vTile, Ogre::Vector3::UNIT_X);

                entity_ = sceneMgr->createEntity(renderer_.lock()->GetUniqueObjectName(), name_.toStdString().c_str());
                entity_->setMaterialName(materialNameAttr_.Get().toStdString().c_str());
                entity_->setCastShadows(false);
                // Tries to attach entity, if there is not EC_OgrePlaceable availible, it will not attach object
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
        Color col = fogColorAttr_.Get();
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
            scene_mgr->getRootSceneNode()->addChild(node_);
            node_->setVisible(true);
            
        }
        
        Vector3df vec = positionAttr_.Get();
        //node_->setPosition(vec.x, vec.y, vec.z);
        node_->_setDerivedPosition(Ogre::Vector3(vec.x, vec.y, vec.z));
       
    }

    void EC_WaterPlane::SetOrientation()
    {
        // Is attached?
        if ( entity_ != 0 && !entity_->isAttached() && attached_  )
        {
            Ogre::SceneManager* scene_mgr = renderer_.lock()->GetSceneManager();
            node_->attachObject(entity_);
            attached_ = true;
            scene_mgr->getRootSceneNode()->addChild(node_);
            node_->setVisible(true);
            
        }

         // Set orientation
        Quaternion rot = rotationAttr_.Get();
        node_->_setDerivedOrientation(Ogre::Quaternion(rot.w, rot.x, rot.y, rot.z));

    }

    void EC_WaterPlane::ChangeWaterPlane(IAttribute* attribute)
    {
        std::string name = attribute->GetNameString();
        if ( ( name == xSizeAttr_.GetNameString() 
          || name == ySizeAttr_.GetNameString()
          || name == scaleUfactorAttr_.GetNameString()
          || name == scaleVfactorAttr_.GetNameString() ) && 
          ( lastXsize_ != xSizeAttr_.Get() || lastYsize_ != ySizeAttr_.Get() ) )
        {
            RemoveWaterPlane();
            CreateWaterPlane();
            
            lastXsize_ = xSizeAttr_.Get();
            lastYsize_ = ySizeAttr_.Get();

        }
        else if ( name == positionAttr_.GetNameString() )
        {
            // Change position
            SetPosition();
           
        }
        else if ( name == rotationAttr_.GetNameString() )
        {
            // Change rotation

            // Is there placeable component? If not use given rotation 
            //if ( dynamic_cast<OgreRenderer::EC_OgrePlaceable*>(FindPlaceable().get()) == 0 )
            //{
               SetOrientation();
            //}
            
        }
        else if ( name == depthAttr_.GetNameString() )
        {
            // Change depth
            // Currently do nothing..
           
        }
        else if ( name ==  materialNameAttr_.GetNameString())
        {
            //Change material
            if ( entity_ != 0)
            {
                entity_->setMaterialName(materialNameAttr_.Get().toStdString().c_str());
            }
        }
        
        
    }


    ComponentPtr EC_WaterPlane::FindPlaceable() const
    {
        assert(framework_);
        ComponentPtr comp;
        if(!GetParentEntity())
            return comp;
        comp = GetParentEntity()->GetComponent<OgreRenderer::EC_OgrePlaceable>();
        return comp;

    }

   
    void EC_WaterPlane::AttachEntity()
    {
        OgreRenderer::EC_OgrePlaceable* placeable = dynamic_cast<OgreRenderer::EC_OgrePlaceable*>(FindPlaceable().get());
        if ((!entity_) || (!placeable) || attached_)
            return;

        Ogre::SceneNode* node = placeable->GetSceneNode();
        node->addChild(node_);
        node_->attachObject(entity_);

      
        attached_ = true;

    }
      
    void EC_WaterPlane::DetachEntity()
    {
        OgreRenderer::EC_OgrePlaceable* placeable = dynamic_cast<OgreRenderer::EC_OgrePlaceable*>(FindPlaceable().get());
        if ((!attached_) || (!entity_) || (!placeable))
            return;

        Ogre::SceneNode* node = placeable->GetSceneNode();
        node_->detachObject(entity_);
        node->removeChild(node_);

        attached_ = false;

    }

}