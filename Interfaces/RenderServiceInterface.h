// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Interfaces_RenderServiceInterface_h
#define incl_Interfaces_RenderServiceInterface_h

#include "ServiceInterface.h"
#include "ResourceInterface.h"
#include "LogListenerInterface.h"
#include "CoreModuleApi.h"
#include "Vector3D.h"
#include "Quaternion.h"

#include <QVariant>
#include <set>

class QRect;

namespace Scene
{
    class Entity;
}

namespace Foundation
{
    //! Result of a raycast. Other fields are valid only if entity_ is non-null
    struct RaycastResult
    {
        //! Entity that was hit, null if none
        Scene::Entity* entity_;
        //! World coordinates of hit position
        Vector3df pos_;
        //! Submesh index in entity, starting from 0
        uint submesh_;
        //! U coord in entity. 0 if no texture mapping
        float u_;
        //! V coord in entity. 0 if no texture mapping
        float v_;
    };

    //! Render service interface.
    /*!
        \ingroup Services_group
        Manages the rendering window, handles scene rendering, and manages renderer related resources.
        Implemented by the \ref OgreRenderingModule.
    */
    class MODULE_API RenderServiceInterface : public ServiceInterface
    {
    public:
        /// Default constructor
        RenderServiceInterface() {}

        /// Destructor.
        virtual ~RenderServiceInterface() {}

        //! Renders the scene
        virtual void Render() = 0;

        //! Do raycast into the world from viewport coordinates.
        /*! The coordinates are a position in the render window, not scaled to [0,1].
            \param x Horizontal position for the origin of the ray
            \param y Vertical position for the origin of the ray
            \return Raycast result structure
        */
        virtual RaycastResult Raycast(int x, int y) = 0;

        //! Do a frustum query to the world from viewport coordinates.
        /*! Returns the found entities as a QVariantList so that
            Python and Javascript can get the result directly from here.
            \param viewrect The query rectangle in 2d window coords.
        */
        virtual QVariantList FrustumQuery(QRect &viewrect) = 0;

        /// Returns the backbuffer image that contains the UI layer of the application screen.
        /// Used to perform alpha-keying based input.
        virtual QImage &GetBackBuffer() = 0;

        //! Returns render window width, or 0 if no window is opened
        virtual int GetWindowWidth() const = 0;

        //! Returns render window height, or 0 if no window is opened
        virtual int GetWindowHeight() const = 0;

        //! subscribe a listener to renderer log
        virtual void SubscribeLogListener(const LogListenerPtr &listener) = 0;

        //! unsubsribe a listener to renderer log
        virtual void UnsubscribeLogListener(const LogListenerPtr &listener) = 0;

        //! set maximum view distance
        virtual void SetViewDistance(float distance) = 0;

        //! get maximum view distance
        virtual float GetViewDistance() const = 0;

        //! force UI repaint
        virtual void RepaintUi() = 0;

        //! get visible entities last frame
        virtual const std::set<entity_id_t>& GetVisibleEntities() = 0;
        
        //! take sceenshot to a location
        //! \param filePath File path.
        //! \param fileName File name.
        virtual void TakeScreenshot(const std::string& filePath, const std::string& fileName) = 0;

        //! Render current main window content to texture
        virtual QPixmap RenderImage(bool use_main_camera = true) = 0;

        //! Render current main window with focus on the avatar
        //! @todo make this focus non hard coded but as param
        virtual QPixmap RenderAvatar(const Vector3df &avatar_position, const Quaternion &avatar_orientation) = 0;

        //! Gets a renderer-specific resource
        /*! Does not automatically queue a download request
            \param id Resource id
            \param type Resource type
            \return pointer to resource, or null if not found
         */
        virtual ResourcePtr GetResource(const std::string& id, const std::string& type) = 0;

        //! Requests a renderer-specific resource to be downloaded from the asset system
        /*! A RESOURCE_READY event will be sent when the resource is ready to use
            \param id Resource id
            \param type Resource type
            \return Request tag, or 0 if request could not be queued
         */
        virtual request_tag_t RequestResource(const std::string& id, const std::string& type) = 0;

        //! Removes a renderer-specific resource
        /*! \param id Resource id
            \param type Resource type
         */
        virtual void RemoveResource(const std::string& id, const std::string& type) = 0;
    };
}

#endif
