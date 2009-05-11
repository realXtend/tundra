// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Interfaces_RenderServiceInterface_h
#define incl_Interfaces_RenderServiceInterface_h

#include "ServiceInterface.h"
#include "ResourceInterface.h"
#include "LogListenerInterface.h"

namespace Foundation
{
    //! \todo document -cm
    /*!
        \ingroup Services_group
    */
    class MODULE_API RenderServiceInterface : public ServiceInterface
    {
    public:
        RenderServiceInterface() {}
        virtual ~RenderServiceInterface() {}

        //! Renders the scene
        virtual void Render() = 0;
        virtual void Raycast() = 0;
        //! Resizes the rendering window
        /*! \param width New window width
            \param height New window height
         */
        virtual void Resize(Core::uint width, Core::uint height) = 0;
        //! Returns render window handle, or 0 if no window is opened
        virtual size_t GetWindowHandle() const = 0;
        //! Returns render window width, or 0 if no window is opened
        virtual int GetWindowWidth() const = 0;
        //! Returns render window height, or 0 if no window is opened
        virtual int GetWindowHeight() const = 0;
        //! subscribe a listener to renderer log
        virtual void SubscribeLogListener(const LogListenerPtr &listener) = 0;
        //! unsubsribe a listener to renderer log
        virtual void UnsubscribeLogListener(const LogListenerPtr &listener) = 0;
             
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
        virtual Core::request_tag_t RequestResource(const std::string& id, const std::string& type) = 0;   
        
        //! Removes a renderer-specific resource
        /*! \param id Resource id
            \param type Resource type
         */
        virtual void RemoveResource(const std::string& id, const std::string& type) = 0;  
    };
}

#endif

