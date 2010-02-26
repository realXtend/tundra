#include "StableHeaders.h"

#include "BuddyList.h"

namespace ProtocolUtilities
{
    Buddy::Buddy(RexUUID id, int rights_given, int rights_has): id_(id), rights_given_(rights_given), rights_has_(rights_has)
    {
    }

    RexUUID Buddy::GetID() const
    {
        return id_;
    }

    int Buddy::GetRightsGiven() const
    {
        return rights_given_;
    }

    int Buddy::GetRightsHas() const
    {
        return rights_has_;
    }

    BuddyList::~BuddyList(void)
    {
        Clear();
    }

    void BuddyList::AddBuddy(Buddy* buddy)
    {
        buddies_.push_back(buddy);
    }

    BuddyVector BuddyList::GetBuddies() const
    {
        return buddies_;
    }
    
    void BuddyList::Clear()
    {
        for(BuddyVector::iterator i = buddies_.begin(); i != buddies_.end(); ++i)
        {
            delete *i;
            *i = 0;
        }
        buddies_.clear();
        
    }

} // end of namespace: OpenSimProtocol
