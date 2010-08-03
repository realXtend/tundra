// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "CameraHandler.h"
#include "WorldBuildingModule.h"
#include "Quaternion.h"

#include <SceneManager.h>
#include <EC_OgrePlaceable.h>
#include <EC_OgreCamera.h>
#include <EC_OgreMesh.h>
#include <EC_OgreCustomObject.h>

#include <Ogre.h>
#include <QUuid>

#include <QDebug>

namespace WorldBuilding
{
    namespace View
    {
        CameraHandler::CameraHandler(Foundation::Framework *framework, QObject *parent) :
            QObject(parent),
            framework_(framework),
            camera_count_(0),
            pixelData_(0)
        {
            render_texture_name_ = "EntityViewPortTexture-" + QUuid::createUuid().toString().toStdString();

            Ogre::TexturePtr entity_screenshot = Ogre::TextureManager::getSingleton().createManual(
                render_texture_name_, Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
                Ogre::TEX_TYPE_2D, 1, 1, 0, Ogre::PF_A8R8G8B8, Ogre::TU_RENDERTARGET);
                
            entity_screenshot->getBuffer()->getRenderTarget()->setAutoUpdated(false);
        }

        CameraHandler::~CameraHandler()
        {
            SAFE_DELETE(pixelData_);
        }

        CameraID CameraHandler::CreateCustomCamera()
        {
            Scene::EntityPtr cam_entity = framework_->GetDefaultWorldScene()->CreateEntity(framework_->GetDefaultWorldScene()->GetNextFreeId());
            if (!cam_entity.get())
                return -1;

            cam_entity->AddComponent(framework_->GetComponentManager()->CreateComponent(OgreRenderer::EC_OgrePlaceable::TypeNameStatic()));
            cam_entity->AddComponent(framework_->GetComponentManager()->CreateComponent(OgreRenderer::EC_OgreCamera::TypeNameStatic()));

            Foundation::ComponentInterfacePtr component_placable = cam_entity->GetComponent(OgreRenderer::EC_OgrePlaceable::TypeNameStatic());
            OgreRenderer::EC_OgreCamera *ec_camera = cam_entity->GetComponent<OgreRenderer::EC_OgreCamera>().get();
            
            if (!component_placable.get() || !ec_camera)
                return -1;
            ec_camera->SetPlaceable(component_placable);

            camera_count_++;
            id_to_cam_entity_[camera_count_] = cam_entity.get();

            return camera_count_;
        }

        void CameraHandler::DestroyCamera(CameraID cam_id)
        {
            if (id_to_cam_entity_.contains(cam_id))
                id_to_cam_entity_.remove(cam_id);

            Ogre::TexturePtr entity_viewport_texture = Ogre::TextureManager::getSingleton().getByName(render_texture_name_);
            if (entity_viewport_texture.isNull())
                return;

            Ogre::RenderTexture *render_texture = entity_viewport_texture->getBuffer()->getRenderTarget();
            if (render_texture)
                render_texture->removeAllViewports();
        }

        bool CameraHandler::FocusToEntity(CameraID cam_id, Scene::Entity *entity, Vector3df offset)
        {
            bool focus_completed = false;

            if (!id_to_cam_entity_.contains(cam_id))
                return focus_completed;
            Scene::Entity *cam_entity = id_to_cam_entity_[cam_id];

            // Get placable from both focus entity and our camera id entity
            OgreRenderer::EC_OgrePlaceable *entity_ec_placable = entity->GetComponent<OgreRenderer::EC_OgrePlaceable>().get();
            OgreRenderer::EC_OgrePlaceable *cam_ec_placable = cam_entity->GetComponent<OgreRenderer::EC_OgrePlaceable>().get();
            OgreRenderer::EC_OgreCamera *cam_ec_camera = cam_entity->GetComponent<OgreRenderer::EC_OgreCamera>().get();
            
            if (!entity_ec_placable || !cam_ec_placable || !cam_ec_camera)
                return focus_completed;

            OgreRenderer::EC_OgreMesh *entity_mesh = entity->GetComponent<OgreRenderer::EC_OgreMesh>().get();
            OgreRenderer::EC_OgreCustomObject *entity_custom_object = entity->GetComponent<OgreRenderer::EC_OgreCustomObject>().get();

            Vector3df position_vector = entity_ec_placable->GetPosition();
            Vector3df position_offset;
            Vector3df look_at;

            Vector3df bounding_min;
            Vector3df bounding_max;
            Vector3df der_size_vector;
            Ogre::Vector3 derived_scale;

            if (entity_mesh)
            {
                entity_mesh->GetBoundingBox(bounding_min, bounding_max);
                derived_scale = entity_mesh->GetEntity()->getParentNode()->_getDerivedScale();
                der_size_vector = Vector3df(derived_scale.x, derived_scale.y, derived_scale.z) * (bounding_max - bounding_min);
                
                position_offset = Vector3df(der_size_vector.x, -der_size_vector.y, der_size_vector.y);
                look_at = Vector3df(position_vector.x, position_vector.y, position_vector.z + (position_offset.z/2));
            }
            else if (entity_custom_object)
            {
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

                position_offset = Vector3df(der_size_vector.x, -max_distance, max_distance/4);
                look_at = Vector3df(position_vector.x, position_vector.y, position_vector.z + (position_offset.z/2));
            }
            else
                return focus_completed;
            
            cam_ec_placable->SetPosition(position_vector + (entity_ec_placable->GetOrientation() * position_offset));
            cam_ec_placable->LookAt(look_at);

            focus_completed = true;
            return focus_completed;
        }

        void CameraHandler::RotateCamera(Vector3df pivot,CameraID id,qreal x, qreal y)
        {

            if (id_to_cam_entity_.contains(id))
            {

                Scene::Entity *cam_entity = id_to_cam_entity_[id];
                OgreRenderer::EC_OgreCamera *ec_camera = cam_entity->GetComponent<OgreRenderer::EC_OgreCamera>().get();
                OgreRenderer::EC_OgrePlaceable *cam_ec_placable = cam_entity->GetComponent<OgreRenderer::EC_OgrePlaceable>().get();
 

                Ogre::Camera* cam = ec_camera->GetCamera();
                Vector3df pos = cam_ec_placable->GetPosition();

                

                Vector3df dir(pos-pivot);
                Quaternion quat(x,cam_ec_placable->GetLocalYAxis());
                quat *= Quaternion(y, cam_ec_placable->GetLocalXAxis());
                dir = quat * dir;
        
                Vector3df new_pos(pivot+dir);
                cam_ec_placable->SetPosition(new_pos);
                cam_ec_placable->LookAt(pivot);
    

            }
        }

        QPixmap CameraHandler::RenderCamera(CameraID cam_id, QSize image_size)
        {
            // Get camera
            QImage captured_pixmap(image_size, QImage::Format_ARGB32_Premultiplied);
            captured_pixmap.fill(Qt::gray);

            // Check that this camera ID exists
            if (!id_to_cam_entity_.contains(cam_id))
                return QPixmap::fromImage(captured_pixmap);
            Scene::Entity *cam_entity = id_to_cam_entity_[cam_id];

            // Get the camera ec
            OgreRenderer::EC_OgreCamera *ec_camera = cam_entity->GetComponent<OgreRenderer::EC_OgreCamera>().get();
            if (!ec_camera)
                return QPixmap::fromImage(captured_pixmap);

            // Get our rendering texture
            Ogre::TexturePtr entity_viewport_texture = Ogre::TextureManager::getSingleton().getByName(render_texture_name_);
            if (entity_viewport_texture.isNull())
                return QPixmap::fromImage(captured_pixmap);

            // Resize rendering texture if needed
            if (entity_viewport_texture->getWidth() != image_size.width() || entity_viewport_texture->getHeight() != image_size.height())
            {
                entity_viewport_texture->freeInternalResources();
                entity_viewport_texture->setWidth(image_size.width());
                entity_viewport_texture->setHeight(image_size.height());
                entity_viewport_texture->createInternalResources();
            }

            // Set camera aspect ratio
            ec_camera->GetCamera()->setAspectRatio(Ogre::Real(image_size.width()) / Ogre::Real(image_size.height()));

            // Get rendering texture and update it
            Ogre::RenderTexture *render_texture = entity_viewport_texture->getBuffer()->getRenderTarget();
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
            Ogre::Box bounds(0, 0, image_size.height(), image_size.width());
            Ogre::PixelBox pixels = Ogre::PixelBox(bounds, Ogre::PF_A8R8G8B8, (void*)pixelData_);

            render_texture->copyContentsToMemory(pixels, Ogre::RenderTarget::FB_AUTO);

            // Create a QImage from the memory
            captured_pixmap = QImage(pixelData_, image_size.height(), image_size.width(), QImage::Format_ARGB32_Premultiplied);
            if (captured_pixmap.isNull())
                WorldBuildingModule::LogDebug("Capturing entity to viewport image failed.");

            // Return image as a QPixmap
            return QPixmap::fromImage(captured_pixmap);
        }
    }
}