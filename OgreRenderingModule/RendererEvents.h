// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_OgreRenderer_RendererEvents_h
#define incl_OgreRenderer_RendererEvents_h

namespace OgreRenderer
{
    //! renderer events
    namespace Event
    {
        //! postrender event id
        static const Core::event_id_t POST_RENDER = 1;
        
        //! window closed event id
        static const Core::event_id_t WINDOW_CLOSED = 2;
    }
}
    
#endif