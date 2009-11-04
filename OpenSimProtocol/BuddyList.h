// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_RexLogic_BuddyList_h
#define incl_RexLogic_BuddyList_h

#include "RexUUID.h"

namespace OpenSimProtocol
{
	/**
	 *  Buddy information received Opensim based server within login response
	 *
	 */
	class Buddy
	{
	public:
		Buddy(RexTypes::RexUUID id, int rights_given, int rights_has);
		virtual RexTypes::RexUUID GetID() const;
		virtual int GetRightsGiven() const;
		virtual int GetRightsHas() const;

	protected:
		RexTypes::RexUUID id_;
		int rights_given_;
		int rights_has_;
	};
	typedef std::vector<Buddy*> BuddyVector;

	/**
	 *  Buddy list information from Opensim based server
	 *  The data is received in login response in xml format with "buddy-list" tag
	 * 
	 */
	class BuddyList
	{
	public:
		virtual ~BuddyList();
		virtual void AddBuddy(Buddy* buddy);
		virtual BuddyVector GetBuddies() const;
		virtual void Clear();
	protected:
		BuddyVector buddies_;
	};
	typedef boost::shared_ptr<BuddyList> BuddyListPtr;

} // end of namespace: OpenSimProtocol

#endif // incl_RexLogic_BuddyList_h
