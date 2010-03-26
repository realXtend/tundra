// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_OgreRenderer_RendererEvents_h
#define incl_OgreRenderer_RendererEvents_h

#include "EventDataInterface.h"

namespace OgreRenderer
{
    //! renderer events
    namespace Events
    {
        //! Sent when main rendering is finished and screen buffers are about to be swapped.
        /*! \ingroup OgreRenderingModuleClient
         */
        static const event_id_t POST_RENDER = 1;
        
        //! Sent when the rendering window has been closed and the application loop is about to terminate.
        /*! \ingroup OgreRenderingModuleClient
         */
        static const event_id_t WINDOW_CLOSED = 2;

        //! Sent when the rendering window has been resized. Uses WindowResized event data structure.
        /*! \ingroup OgreRenderingModuleClient
         */
        static const event_id_t WINDOW_RESIZED = 3;

        //! Event data for window resize
        /*! \ingroup OgreRenderingModuleClient
         */
        class WindowResized : public Foundation::EventDataInterface
        {
            WindowResized();
        public:
            WindowResized(int width, int height) : width_(width), height_(height) {}
            virtual ~WindowResized() {}
        
            //! New width of the window
            const int width_;
            //! New height of the window
            const int height_;
        };
    }
}
    
#endif
