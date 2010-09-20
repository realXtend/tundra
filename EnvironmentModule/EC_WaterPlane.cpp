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

#include <QDebug>

#include <Ogre.h>
#include <OgreQuaternion.h>

#include "MemoryLeakCheck.h"


namespace Environment
{
   
    EC_WaterPlane::EC_WaterPlane(IModule *module)
        : IComponent(module->GetFramework()),
        xSizeAttr_(this, "Water plane x-size", 5000),
        ySizeAttr_(this, "Water plane y-size", 5000),
        depthAttr_(this, "Water plane depth", 20),
        positionAttr_(this, "Water plane position", Vector3df()),
        rotationAttr_(this, "Water plane rotation", Quaternion()),
        scaleUfactorAttr_(this, "U scale factor", 0.0002f),
        scaleVfactorAttr_(this, "V scale factor", 0.0002f),
        xSegmentsAttr_(this, "The number of segments to the plane in the x direction", 10),
        ySegmentsAttr_(this, "The number of segments to the plane in the y direction", 10),
        materialNameAttr_(this, "Water material", QString("Ocean")),
        fogColorAttr_(this, "Underwater fog color", Color()),
        fogStartAttr_(this, "Fog start distance", 0.0),
        fogEndAttr_(this, "Fog end distance", 0.0),
        entity_(0),
        node_(0),
        attached_(false)
    {
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
      ///todo this cannot be done this way, mesh orientation etc. must take care. 

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
    
      int tmpMax = pos.x + 0.5*xSize;
      int tmpMin = pos.x - 0.5*xSize;
          
      if ( x > tmpMin && x < tmpMax )
        {
            tmpMax = pos.y + 0.5*ySize;
            tmpMin = pos.y - 0.5*ySize;
        
            if ( y >tmpMin && y < tmpMax)
            {
                tmpMax = pos.z;
                tmpMin = pos.z - depth;
            
                if ( z> tmpMin && z < tmpMax)
                    return true;
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

    void EC_WaterPlane::AttributeUpdated(IAttribute* attribute, AttributeChange::Type change)
    {
        ChangeWaterPlane(attribute);
    }

    void EC_WaterPlane::SetPosition()
    {
        // Is attached?
        if ( entity_ != 0 && !entity_->isAttached() && !attached_ )
        {
            node_->attachObject(entity_);
            attached_ = true;
        }
        
        Vector3df vec = positionAttr_.Get();
        node_->setPosition(vec.x, vec.y, vec.z);

       
    }

    void EC_WaterPlane::SetOrientation()
    {
        // Is attached?
        if ( entity_ != 0 && !entity_->isAttached() && attached_  )
        {
            node_->attachObject(entity_);
            attached_ = true;
        }

         // Set orientation
        Quaternion rot = rotationAttr_.Get();
        node_->setOrientation(Ogre::Quaternion(rot.w, rot.x, rot.y, rot.z));

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
            
            // Is there placeable component? If not use given default position.
            if ( dynamic_cast<OgreRenderer::EC_OgrePlaceable*>(FindPlaceable().get()) == 0 )
            {
               SetPosition();
            }
           
        }
        else if ( name == rotationAttr_.GetNameString() )
        {
            // Change rotation

             // Is there placeable component? If not use given rotation 
            if ( dynamic_cast<OgreRenderer::EC_OgrePlaceable*>(FindPlaceable().get()) == 0 )
            {
               SetOrientation();
            }
            
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