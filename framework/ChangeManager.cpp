
#include "StableHeaders.h"
#include "ChangeManager.h"

namespace Foundation
{
    void ChangeManager::change(Core::entity_id_t id, Change change, const boost::any &newValue)
    {
        SD_Change sd = { id, change, newValue };
        {
            Core::mutex_lock lock(mChangeQueueMutex);
            mChanges.push(sd);
        }
    }

    void ChangeManager::_propagateChanges()
    {
        // We should be in single threaded context here, no need for mutex
        // Core::mutex_lock lock(mChangeQueueMutex);

        while (mChanges.empty() == false)
        {
            const SD_Change &change = mChanges.front();

            DesiredChangesMap::iterator iter = mDesiredChanges.find(std::make_pair(change.mEntity, change.mType));

            if (iter != mDesiredChanges.end())
            {
                iter->second->notify(NULL, change.mNewValue);
            }
            

            mChanges.pop();
        }
    }
}
