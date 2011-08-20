// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_OgreRenderer_OgreListeners_h
#define incl_OgreRenderer_OgreListeners_h

#include "Renderer.h"
#include "Entity.h"

#include <OgreAny.h>
#include <OgreException.h>
#include <OgreRenderQueue.h>
#include <OgreLog.h>

namespace OgreRenderer
{
    //! Ogre renderable listener to find out visible objects for each frame
    class RenderableListener : public Ogre::RenderQueue::RenderableListener
    {

    public:
        RenderableListener(Renderer* renderer) : renderer_(renderer)
        {
        }

        ~RenderableListener()
        {
        }

        virtual bool renderableQueued(Ogre::Renderable* rend, Ogre::uint8 groupID,
            Ogre::ushort priority, Ogre::Technique** ppTech, Ogre::RenderQueue* pQueue)
        {
            Ogre::Any any = rend->getUserAny();
            if (any.isEmpty())
                return true;

            Scene::Entity *entity = 0;
            try
            {
                entity = Ogre::any_cast<Scene::Entity*>(any);
                if (entity)
                    renderer_->visible_entities_.insert(entity->GetId());
            }
            catch (Ogre::InvalidParametersException &)
            {
            }
            return true;
        }

    private:
        Renderer* renderer_;
    };

    //! Ogre log listener, for passing ogre log messages
    class LogListener : public Ogre::LogListener
    {

    public:
        LogListener() : Ogre::LogListener() 
        {
        }
        
        virtual ~LogListener() 
        {
        }

        //! Subscribe new listener for ogre log messages
        void SubscribeListener(const Foundation::LogListenerPtr &listener)
        {
            listeners_.push_back(listener);
        }

        //! Unsubscribe listener for ogre log messages
        void UnsubscribeListener(const Foundation::LogListenerPtr &listener)
        {
            listeners_.remove(listener);
        }

        void messageLogged(const std::string& message, Ogre::LogMessageLevel lml, bool maskDebug, const std::string &logName)
        {
            for (ListenerList::iterator it = listeners_.begin(); it != listeners_.end(); ++it)
                (*it)->LogMessage(message);
        }

    private:
        typedef std::list<Foundation::LogListenerPtr> ListenerList;

        //! list of subscribed listeners
        ListenerList listeners_;
    };
}

#endif
