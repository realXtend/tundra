// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "CameraHandler.h"
#include "WorldBuildingModule.h"

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

            Vector3df bounding_min;
            Vector3df bounding_max;
            if (entity_mesh)
                entity_mesh->GetBoundingBox(bounding_min, bounding_max);
            else if (entity_custom_object)
                entity_custom_object->GetBoundingBox(bounding_min, bounding_max);
            else
                return focus_completed;

            qDebug() << "B.Min    : x " << bounding_min.x << " y " << bounding_min.y << " z " << bounding_min.z;
            qDebug() << "B.Max    : x " << bounding_max.x << " y " << bounding_max.y << " z " << bounding_max.z;

            Vector3df scale_vector = entity_ec_placable->GetScale();
            qDebug() << "Scale    : x " << scale_vector.x << " y " << scale_vector.y << " z " << scale_vector.z;
            
            Vector3df position_offset(scale_vector.x, -scale_vector.y*2.5, scale_vector.z*1.5);
            qDebug() << "Offset   : x " << position_offset.x << " y " << position_offset.y << " z " << position_offset.z;

            Vector3df position_vector = entity_ec_placable->GetPosition();
            qDebug() << "Position : x " << position_vector.x << " y " << position_vector.y << " z " << position_vector.z;

            Vector3df look_at(position_vector.x, position_vector.y, position_vector.z + position_offset.z);
            qDebug() << "Look at  : x " << look_at.x << " y " << look_at.y << " z " << look_at.z << endl;

            cam_ec_placable->SetPosition(entity_ec_placable->GetPosition() + (entity_ec_placable->GetOrientation() * position_offset));
            cam_ec_placable->LookAt(look_at);

            focus_completed = true;
            return focus_completed;
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
            }
            //else
            //{
            //    Ogre::Viewport *vp = render_texture->getViewport(0);
            //    vp->setCamera(ec_camera->GetCamera());
            //    int i = 0;
            //}

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