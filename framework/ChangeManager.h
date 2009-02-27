// For conditions of distribution and use, see copyright notice in license.txt

#ifndef __inc_Foundation_ChangeManager_h__
#define __inc_Foundation_ChangeManager_h__

#include "ComponentInterface.h"
//#include <queue>
#include <Poco/Delegate.h>
#include <Poco/BasicEvent.h>
//#include <boost/any.hpp>

namespace Foundation
{
    template <class T>
    struct DelegateStorage
    {
        Poco::Delegate<T, const boost::any> CallbackFnt;
    };


    typedef Core::uint Change;
    typedef std::vector<Change> ChangeVector;
    typedef boost::shared_ptr<ChangeVector> ChangeVectorPtr;

    //! Propagates changes to shared data across different entity components
    /*!
        Each entity component should register the shared data they contain with
        registerGlobalChange() function.

        Each component should also register the global notifications from other
        components to changes in shared data they are interested in.

        When change occurs in shared data, propagate the change to other components with change() function.
    */
    class ChangeManager
    {
        struct RegisteredChange;
        typedef std::pair<std::string, Change> ComponentChangePair;

        typedef Poco::BasicEvent< const boost::any> ChangeEvent;
        typedef boost::shared_ptr<ChangeEvent> ChangeEventPtr;

        //! entity based change
        typedef std::pair<Core::entity_id_t, Change> EntityChangePair;

    public:
        ChangeManager() : mChangeIds(0) {}
        ~ChangeManager() {}

        //! Each component contains their own id's for changes to shared data.
        /*! This function can be used to transform that local id into global id that is recognized by all components and modules

            \param component global id of the component
            \param local local id of the shared data
        */
        Change registerGlobalChange(const std::string &component, Change local)
        {
            ComponentChangePair componentChange = std::make_pair(component, local);

            assert (mRegisteredChanges.find(componentChange) == mRegisteredChanges.end());
            mRegisteredChanges[componentChange] = mChangeIds;

            return mChangeIds++;
        }

        //! Each component contains their own id's for changes to shared data.
        /*! This function can be used to transform that local ids into global ids that is recognized by all components and modules
            This function registers all local changes in the range [start, end]. It is assumed the range is continous.

            \param component global id of the component
            \param start local id of the shared data
            \param end local id of the shared data
        */
        void registerGlobalChanges(const std::string &component, Change start, Change end)
        {
            for (Change i=start ; i<=end ; ++i)
            {
                registerGlobalChange(component, i);
            }
        }

        //! Returns global id of components shared data id
        Change getGlobalChange(const std::string &component, Change local)
        {
            ComponentChangePair componentChange = std::make_pair(component, local);
            assert (mRegisteredChanges.find(componentChange) != mRegisteredChanges.end());

            return mRegisteredChanges[componentChange];
        }

        template <class T>
        void registerDesiredChange(const std::string &component, Change change, Core::entity_id_t id, Poco::Delegate<T, const boost::any> &fnt)
        {
            Change globalChange = getGlobalChange(component, change);
            EntityChangePair entityChange = std::make_pair(id, change);

            mDesiredChanges[entityChange] = ChangeEventPtr(new ChangeEvent);
            *mDesiredChanges[entityChange] += fnt;
        }

        void registerPotentialChanges(const ComponentInterface *component, const ChangeVectorPtr &changes);

        //! Change occurred in shared data, queue the change
        void change(Core::entity_id_t id, Change change, const boost::any &newValue);

        //! propagate any changes to shared data to all components. Do not call directly
        void _propagateChanges();

    private:
        struct SD_Change
        {
            Core::entity_id_t mEntity;
            Change mType;
            boost::any mNewValue;
        };
        typedef std::map< ComponentChangePair, Change > ComponentChangeMap;
        typedef std::map<EntityChangePair, ChangeEventPtr> DesiredChangesMap;
        typedef std::queue<SD_Change> ChangeQueue;

        //! All module specific registered changes
        ComponentChangeMap mRegisteredChanges;

        //! desired changes by component
        DesiredChangesMap mDesiredChanges;

        boost::mutex mChangeQueueMutex;
        ChangeQueue mChanges;

        Change mChangeIds;
    };
}

#endif // __inc_Foundation_ChangeManager_h__
