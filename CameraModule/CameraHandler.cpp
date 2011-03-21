//$ HEADER_NEW_FILE $
// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "CameraHandler.h"
#include "Quaternion.h"

#include <SceneManager.h>
#include <EC_Placeable.h>
#include <EC_OgreCamera.h>
#include <EC_Mesh.h>
#include <EC_OgreCustomObject.h>

#include <Ogre.h>
#include <QUuid>

#include <QDebug>

namespace Camera
{
    CameraHandler::CameraHandler(Foundation::Framework *framework,CameraType type, QObject *parent) :
        QObject(parent),
        framework_(framework),
        pixelData_(0),
        zoom_acceleration_(0.001f),
        x_acceleration_(1),
        y_acceleration_(1),
        camera_type_(type),
        min_zoom_distance_(3),
        max_zoom_distance_(100),
        target_entity_(0)
    {
        render_texture_name_ = "EntityViewPortTexture_" + QUuid::createUuid().toString().toStdString();

        Ogre::TexturePtr entity_screenshot = Ogre::TextureManager::getSingleton().createManual(
            render_texture_name_, Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
            Ogre::TEX_TYPE_2D, 1, 1, 0, Ogre::PF_A8R8G8B8, Ogre::TU_RENDERTARGET);
            
        entity_screenshot->getBuffer()->getRenderTarget()->setAutoUpdated(false);
        
    }

    CameraHandler::~CameraHandler()
    {
        SAFE_DELETE(pixelData_);
    }

	bool CameraHandler::CreateCamera(Scene::ScenePtr scene)
    {
        if (!cam_entity_)
        {
            //create camera entity
            if (!scene)
                return false;

            cam_entity_ = scene->CreateEntity(scene->GetNextFreeId());
            if (!cam_entity_.get())
                return false;

            cam_entity_->AddComponent(framework_->GetComponentManager()->CreateComponent(EC_Placeable::TypeNameStatic()));
            cam_entity_->AddComponent(framework_->GetComponentManager()->CreateComponent(EC_OgreCamera::TypeNameStatic()));
            scene->EmitEntityCreated(cam_entity_);
            ComponentPtr component_placeable = cam_entity_->GetComponent(EC_Placeable::TypeNameStatic());
            EC_OgreCamera *ec_camera = cam_entity_->GetComponent<EC_OgreCamera>().get();
			/*ec_camera->GetCamera()->setPolygonMode(Ogre::PM_WIREFRAME);
            ec_camera->GetCamera()->setProjectionType(Ogre::PT_ORTHOGRAPHIC);
			ec_camera->GetCamera()->setOrthoWindowHeight(10);
			ec_camera->GetCamera()->setCastShadows(false);*/
			//ec_camera->GetCamera()->setNearClipDistance(1);
			//ec_camera->GetCamera()->setFarClipDistance (2);

            if (!component_placeable.get() || !ec_camera)
                return false;

            ec_camera->SetPlaceable(component_placeable);
            return true;
        }            
        return false;
    }

    void CameraHandler::DestroyCamera()
    {
        Ogre::TexturePtr entity_viewport_texture = Ogre::TextureManager::getSingleton().getByName(render_texture_name_);
        if (entity_viewport_texture.isNull())
            return;

        Ogre::RenderTexture *render_texture = entity_viewport_texture->getBuffer()->getRenderTarget();
        if (render_texture)
            render_texture->removeAllViewports();
    }

    bool CameraHandler::FocusToEntity(Scene::Entity *entity, Vector3df offset)
    {
        bool focus_completed = false;
        if (cam_entity_)
        {                
            // Get placeable from both focus entity and our camera id entity
            EC_Placeable *entity_ec_placeable = entity->GetComponent<EC_Placeable>().get();
            EC_Placeable *cam_ec_placeable = cam_entity_->GetComponent<EC_Placeable>().get();
            EC_OgreCamera *cam_ec_camera = cam_entity_->GetComponent<EC_OgreCamera>().get();
            
            if (!entity_ec_placeable || !cam_ec_placeable || !cam_ec_camera)
                return focus_completed;

            EC_Mesh *entity_mesh = entity->GetComponent<EC_Mesh>().get();
            EC_OgreCustomObject *entity_custom_object = entity->GetComponent<EC_OgreCustomObject>().get();

            Vector3df position_vector = entity_ec_placeable->GetPosition();
            Vector3df position_offset;
            Vector3df look_at;

            Vector3df bounding_min;
            Vector3df bounding_max;
            Vector3df der_size_vector;
            Ogre::Vector3 derived_scale;

            if (entity_mesh)
            {
                if (!entity_mesh->GetEntity())
                    return false;

                entity_mesh->GetBoundingBox(bounding_min, bounding_max);
                derived_scale = entity_mesh->GetEntity()->getParentNode()->_getDerivedScale();
                der_size_vector = Vector3df(derived_scale.x, derived_scale.y, derived_scale.z) * (bounding_max - bounding_min);                                
            }
            else if (entity_custom_object)
            {
                if (!entity_custom_object->GetEntity())
                    return false;

                entity_custom_object->GetBoundingBox(bounding_min, bounding_max);
                derived_scale = entity_custom_object->GetEntity()->getParentNode()->_getDerivedScale();
                der_size_vector = Vector3df(derived_scale.x, derived_scale.y, derived_scale.z) * (bounding_max - bounding_min);
                
                float max_distance = 0;
                if (der_size_vector.x > max_distance)
                    max_distance = der_size_vector.x;
                if (der_size_vector.y > max_distance)
                    max_distance = der_size_vector.y;
                if (der_size_vector.z > max_distance)
                    max_distance = der_size_vector.z;
            }
            else
                return focus_completed;
            
            switch (camera_type_)
            {
                case Perspective:
                {
                    position_offset = Vector3df(der_size_vector.x, -der_size_vector.y, der_size_vector.y);
                    position_offset = entity_ec_placeable->GetOrientation()* position_offset;
                    focus_completed = true;
                    break;
                }
                case Front:
                {
                    position_offset = Vector3df(der_size_vector.x, 0,0);
                    focus_completed = true;
                    break;
                }
                case Back:
                {
                    position_offset = Vector3df(-der_size_vector.x, 0,0);
                    focus_completed = true;
                    break;
                }
                case Left:
                {
                    position_offset = Vector3df(0,-der_size_vector.y,0);
                    focus_completed = true;                    
                    break;
                }
                case Rigth:
                {
                    position_offset = Vector3df(0,der_size_vector.y,0);
                    focus_completed = true;
                    break;
                }
                case Top:
                {
                    position_offset = Vector3df(0,0,der_size_vector.z);
                    focus_completed = true;
                    break;
                }
                case Bottom:
                {
                    position_offset = Vector3df(0,0,-der_size_vector.z);
                    focus_completed = true;
                    break;
                }
            }
            if (focus_completed)
            {
                look_at = Vector3df(position_vector.x, position_vector.y, position_vector.z);
                cam_ec_placeable->SetPosition(position_vector + position_offset);
                cam_ec_placeable->LookAt(look_at);
                target_entity_ = entity;
            }            
        }
        return focus_completed;
    }

    void CameraHandler::Move(qreal x, qreal y)
    {
        if (cam_entity_)
        {
            EC_Placeable *cam_ec_placeable = cam_entity_->GetComponent<EC_Placeable>().get();
            if (!cam_ec_placeable)
                return;

            Vector3df pos = cam_ec_placeable->GetPosition();

            switch (camera_type_)
            {
            case Perspective:
                {
                    EC_Placeable *entity_ec_placeable = target_entity_->GetComponent<EC_Placeable>().get();
                    RotateCamera(entity_ec_placeable->GetPosition(), x, y);
                    break;
                }
            case Front:
                {
                    pos.y -=x*x_acceleration_;
                    pos.z +=y*y_acceleration_;
                    cam_ec_placeable->SetPosition(pos);
                    break;
                }
            case Back:
                {                                        
                    pos.y +=x*x_acceleration_;
                    pos.z +=y*y_acceleration_;
                    cam_ec_placeable->SetPosition(pos);
                    break;
                }
            case Left:
                {                    
                    pos.x +=x*x_acceleration_;
                    pos.z -=y*y_acceleration_;
                    cam_ec_placeable->SetPosition(pos);
                    break;
                }
            case Rigth:
                {                    
                    pos.x +=x*x_acceleration_;
                    pos.z +=y*y_acceleration_;
                    cam_ec_placeable->SetPosition(pos);
                    break;
                }
            case Top:
                {                    
                    pos.x +=x*x_acceleration_;
                    pos.y -=y*y_acceleration_;
                    cam_ec_placeable->SetPosition(pos);
                    break;
                }
            case Bottom:
                {                    
                    pos.x +=x*x_acceleration_;
                    pos.y +=y*y_acceleration_;
                    cam_ec_placeable->SetPosition(pos);
                    break;
                }                
            }
        }
    }

    void CameraHandler::RotateCamera(Vector3df pivot, qreal x, qreal y)
    {
        if (cam_entity_)
        {            
            EC_OgreCamera *ec_camera = cam_entity_->GetComponent<EC_OgreCamera>().get();
            EC_Placeable *cam_ec_placeable = cam_entity_->GetComponent<EC_Placeable>().get();
            if (!ec_camera || !cam_ec_placeable)
                return;

            Vector3df pos = cam_ec_placeable->GetPosition();

            Vector3df dir(pos-pivot);
            Quaternion quat(-x,cam_ec_placeable->GetLocalYAxis());
            quat *= Quaternion(-y, cam_ec_placeable->GetLocalXAxis());
            dir = quat * dir;

            Vector3df new_pos(pivot+dir);
            cam_ec_placeable->SetPosition(new_pos);
            cam_ec_placeable->LookAt(pivot);
        }
    }

    bool CameraHandler::Zoom(qreal delta)
    {
        bool zoomed = false;
        
        if (cam_entity_)
        {
            //get camera entity placeable component
            EC_Placeable *placeable = cam_entity_->GetComponent<EC_Placeable>().get();

            //get target entity placeable component
            if (!target_entity_)
                return false;
            EC_Placeable *target_placeable = target_entity_->GetComponent<EC_Placeable>().get();

            if (!placeable && !target_placeable)
                return false;

            //get target and cam position
            Vector3df point = target_placeable->GetPosition();
            Vector3df pos = placeable->GetPosition();
            //get direction vector
            Vector3df dir = point-pos;
            Vector3df distance = dir;
            
            //modify direction vector with wheel delta and zoom aceleration constants
            dir *=delta*zoom_acceleration_;
            
            switch (camera_type_)
            {
            case Perspective:
                {
                    //something fishy, even if we check that we never go beyond min/max, we still might end up there and zoom will be disabled. So we check the also
                    //if were zooming in or out.
                    if ((delta>0 
                        && (distance.getLength() > min_zoom_distance_)
                        && (distance.getLength()+dir.getLength() > min_zoom_distance_)) 
                       || (delta<0 && (distance.getLength()+dir.getLength() < max_zoom_distance_)))
                    {
                        pos = placeable->GetPosition() + dir;
                        zoomed = true;
                    }
                    break;
                }
            case Front:
            case Back:
                {
                    //check with min and max distance if zoom in x is posible
                    if((delta>0 
                        && (abs(distance.x) > min_zoom_distance_) 
                        && (abs(distance.x) + dir.x > min_zoom_distance_)) 
                        || (delta<0 && (abs(distance.x) + dir.x < max_zoom_distance_)))
                    {
                        pos.x += dir.x;
                        zoomed = true;
                    }                        
                    break;
                }
            case Left:  
            case Rigth:
                {      
                    //check with min and max distance if zoom in y is posible
                    if((delta>0 
                        && (abs(distance.y) > min_zoom_distance_) 
                        && (abs(distance.y) + dir.x > min_zoom_distance_)) 
                        || (delta<0 && (abs(distance.y) + dir.y < max_zoom_distance_)))
                    {
                        pos.y += dir.y;
                        zoomed = true;
                    }                        
                    break;
                }
            case Top:
            case Bottom:
                {                                           
                    //check with min and max distance if zoom in z is posible
                    if((delta>0 
                        && (abs(distance.z) > min_zoom_distance_) 
                        && (abs(distance.z) + dir.z > min_zoom_distance_)) 
                        || (delta<0 && (abs(distance.z) + dir.z < max_zoom_distance_)))
                    {
                        pos.z += dir.z;
                        zoomed = true;
                    }                        
                    break;
                }
            }
            //if zoom is posible set cam entity new position
            if (zoomed)
                placeable->SetPosition(pos);            
        }
        return zoomed;
    }

    QPixmap CameraHandler::RenderCamera(QSize image_size)
    {
        if (cam_entity_)
        {
            // Get camera
            QImage captured_pixmap(image_size, QImage::Format_ARGB32_Premultiplied);
            captured_pixmap.fill(Qt::gray);

            // Get the camera ec
            EC_OgreCamera *ec_camera = cam_entity_->GetComponent<EC_OgreCamera>().get();
            if (!ec_camera)
                return QPixmap::fromImage(captured_pixmap);

            // Get our rendering texture
            Ogre::TexturePtr entity_viewport_texture = Ogre::TextureManager::getSingleton().getByName(render_texture_name_);
            if (entity_viewport_texture.isNull())
                return QPixmap::fromImage(captured_pixmap);

            // Re-create rendering texture if size has changed, this has to be done it seems with Ogre 1.7.1
            if ((int)entity_viewport_texture->getWidth() != image_size.width() || (int)entity_viewport_texture->getHeight() != image_size.height())
            {
                Ogre::TextureManager::getSingleton().remove(render_texture_name_);
                render_texture_name_ = "EntityViewPortTexture_" + QUuid::createUuid().toString().toStdString();
                entity_viewport_texture = Ogre::TextureManager::getSingleton().createManual(
                    render_texture_name_, Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
                    Ogre::TEX_TYPE_2D, image_size.width(), image_size.height(), 0, Ogre::PF_A8R8G8B8, Ogre::TU_RENDERTARGET);
                entity_viewport_texture->getBuffer()->getRenderTarget()->setAutoUpdated(false);
            }

            // Set camera aspect ratio
            ec_camera->GetCamera()->setAspectRatio(Ogre::Real(image_size.width()) / Ogre::Real(image_size.height()));

            // Get rendering texture and update it
            Ogre::RenderTexture *render_texture = entity_viewport_texture->getBuffer()->getRenderTarget();
            if (render_texture)
            {
                render_texture->removeAllViewports();
                if (render_texture->getNumViewports() == 0)
                {
                    Ogre::Viewport *vp = render_texture->addViewport(ec_camera->GetCamera());
                    vp->setOverlaysEnabled(false);
                    // Exclude highlight mesh from rendering
                    vp->setVisibilityMask(0x2);
                }
                render_texture->update();

                // Copy render target pixels into memory
                SAFE_DELETE(pixelData_);
                
                pixelData_ = new Ogre::uchar[image_size.height() * image_size.width() * 4];
                Ogre::Box bounds(0, 0, image_size.width(), image_size.height());
                Ogre::PixelBox pixels = Ogre::PixelBox(bounds, Ogre::PF_A8R8G8B8, (void*)pixelData_);

                //entity_viewport_texture->getBuffer()->blitToMemory(pixels);
                render_texture->copyContentsToMemory(pixels, Ogre::RenderTarget::FB_AUTO);

                // Create a QImage from the memory
                captured_pixmap = QImage(pixelData_, image_size.width(), image_size.height(), QImage::Format_ARGB32_Premultiplied);               
            }
            // Return image as a QPixmap
            return QPixmap::fromImage(captured_pixmap);
        }
        else
            return 0;
       
    }
}
