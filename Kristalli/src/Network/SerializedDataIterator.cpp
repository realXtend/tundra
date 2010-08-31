/** @file
	@author Jukka Jylänki

	This work is copyrighted material and may NOT be used for any kind of commercial or 
	personal advantage and may NOT be copied or redistributed without prior consent
	of the author(s). 

	@brief
*/
#include <cassert>

#include "clb/Network/SerializedDataIterator.h"

#include "clb/Memory/DebugMemoryLeakCheck.h"

BasicSerializedDataType SerializedDataIterator::NextElementType() const
{
	if (currentElementStack.size() == 0)
		return SerialInvalid;

	assert(currentElementStack.back().elem);

	// If we don't know how many instances there are of the next element, it's the next field 
	// to be filled - our iterator is pointing to the dynamicCount property of that field.
	if (currentElementStack.back().elem->varyingCount && currentElementStack.back().dynamicCountSpecified == false)
		return SerialDynamicCount;

	return currentElementStack.back().elem->type;
}

const SerializedElementDesc *SerializedDataIterator::NextElementDesc() const
{
	return currentElementStack.size() > 0 ? currentElementStack.back().elem : 0;
}

void SerializedDataIterator::ProceedToNextVariable()
{
	if (currentElementStack.size() == 0)
		return;

	ElemInfo &nextVar = currentElementStack.back();

	if (nextVar.elem->type == SerialStruct)
	{
		++nextVar.nextElem;
		if (nextVar.nextElem >= (int)nextVar.elem->elements.size())
		{
			nextVar.nextElem = 0;
			++nextVar.nextIndex;
			if (nextVar.nextIndex >= nextVar.count)
			{
				currentElementStack.pop_back();
				ProceedToNextVariable();
				return;
			}
		}

		DescendIntoStructure();
	}
	else
	{
		++nextVar.nextIndex;
		if (nextVar.nextIndex >= nextVar.count)
		{
			currentElementStack.pop_back();
			ProceedToNextVariable();
		}
	}
}

void SerializedDataIterator::ProceedNVariables(int count)
{
	///\todo Can optimize a great deal here.
	for(int i = 0; i < count; ++i) 
		ProceedToNextVariable();
}

void SerializedDataIterator::ProceedToNextElement()
{
	ElemInfo &nextVar = currentElementStack.back();

	++nextVar.nextElem;
	if (nextVar.nextElem >= (int)nextVar.elem->elements.size())
	{
		nextVar.nextElem = 0;
		++nextVar.nextIndex;
		if (nextVar.nextIndex >= nextVar.count)
		{
			currentElementStack.pop_back();
			ProceedToNextElement();
		}
	}
	else
	{
/*		currentElementStack.push_back(ElemInfo());
		ElemInfo &newVar = currentElementStack.back();
		newVar.elem = nextVar.elem->elements[nextVar.nextElem];
		newVar.nextIndex = 0;
		newVar.nextElem = 0;
		newVar.count = (newVar.elem->multiplicity == ElemVarying) ? 0 : newVar.elem->count;
*/
		DescendIntoStructure();
	}
}

void SerializedDataIterator::SetVaryingElemSize(u32 count)
{
	ElemInfo &nextVar = currentElementStack.back();
	assert(nextVar.dynamicCountSpecified == false);
	assert(nextVar.elem->varyingCount == true);
	assert(nextVar.nextIndex == 0);
	nextVar.count = count;
	nextVar.dynamicCountSpecified = true;

	// If this was a varying-arity structure node, descend down into filling the struct.
	DescendIntoStructure();
}

void SerializedDataIterator::DescendIntoStructure()
{
	ElemInfo &nextVar = currentElementStack.back();
//	if (nextVar.elem->type != SerialStruct) // Cannot descend to a struct if the next var is not a struct.
//		return;
//	if (nextVar.count == 0) // Cannot descend if the multiplicity of current level was not specified.
//		return;
	if (nextVar.dynamicCountSpecified == false && nextVar.elem->varyingCount == true)
		return;
	if (nextVar.nextElem >= (int)nextVar.elem->elements.size())
		return;

	ElemInfo newVar;
	newVar.elem = nextVar.elem->elements[nextVar.nextElem];
	newVar.nextIndex = 0;
	newVar.nextElem = 0;
 	newVar.count = (newVar.elem->varyingCount ? 0 : newVar.elem->count); // A varying block? Then the user has to supply multiplicity.
	newVar.dynamicCountSpecified = false;
	currentElementStack.push_back(newVar);

	// Descend again in case we have a struct-in-struct-in-struct...
	DescendIntoStructure();
}

void SerializedDataIterator::ResetTraversal()
{
	currentElementStack.clear();

	ElemInfo newVar;
	newVar.elem = desc.data;
	newVar.nextIndex = 0;
	newVar.nextElem = 0;
 	newVar.count = (newVar.elem->varyingCount ? 0 : newVar.elem->count); // A varying block? Then the user has to supply multiplicity.
	newVar.dynamicCountSpecified = false;
	currentElementStack.push_back(newVar);

	// Descend again in case we have a struct-in-struct-in-struct...
	DescendIntoStructure();
}
