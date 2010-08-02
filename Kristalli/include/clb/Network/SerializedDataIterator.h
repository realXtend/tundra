/** @file
	@author Jukka Jylänki

	This work is copyrighted material and may NOT be used for any kind of commercial or 
	personal advantage and may NOT be copied or redistributed without prior consent
	of the author(s). 

	@brief
*/
#ifndef SerializedDataIterator_h
#define SerializedDataIterator_h

#include "clb/Core/Ptr.h"
#include "MessageListParser.h"

class SerializedDataIterator : public clb::RefCountable
{
	struct ElemInfo
	{
		/// The element we are accessing next.
		SerializedElementDesc *elem;
		/// The index of the elem we are accessing next.
		int nextElem;
		/// The index of the instance we are accessing next.
		int nextIndex;
		/// The total number of instances of this element we are accessing.
		int count;
		/// If this element is a dynamic count -one, then this tracks whether the count has been passed in.
		bool dynamicCountSpecified;
	};

	void ProceedToNextElement();
	void DescendIntoStructure();

	/// Stores the tree traversal progress.
	std::vector<ElemInfo> currentElementStack;
	/// The type of the message we are building.
	const SerializedMessageDesc &desc;

public:
	SerializedDataIterator(const SerializedMessageDesc &desc_)
	:desc(desc_)
	{
		ResetTraversal();
	}

	BasicSerializedDataType NextElementType() const;

	const SerializedElementDesc *NextElementDesc() const;

	void ProceedToNextVariable();

	void ProceedNVariables(int count);

	/// Sets the number of instances in a varying element. When iterating over
	/// the message to insert data into serialized form, this information needs
	/// to be passed to this iterator in order to continue.
	void SetVaryingElemSize(u32 count);

	void ResetTraversal();
};


#endif
