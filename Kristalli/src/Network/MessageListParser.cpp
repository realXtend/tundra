/** @file
	@author Jukka Jylänki

	This work is copyrighted material and may NOT be used for any kind of commercial or 
	personal advantage and may NOT be copied or redistributed without prior consent
	of the author(s). 

	@brief
*/
#include "TinyXML/TinyXML.h"

#include "clb/Network/MessageListParser.h"

#include "clb/Memory/DebugMemoryLeakCheck.h"

#define NUMELEMS(x) (sizeof(x)/sizeof(x[0]))

namespace
{
	const char *data[] = { "", "bit", "u8", "s8", "u16", "s16", "u32", "s32", "u64", "s64", "float", "double", "struct" };
	const size_t typeSizes[] = { -1, -1, 1, 1, 2, 2, 4, 4, 8, 8, 4, 8, -1 }; ///< -1 here denotes 'does not apply'.
}

BasicSerializedDataType StringToSerialType(const char *type)
{
	for(int i = 0; i < NUMELEMS(data); ++i)
		if (!strcmp(type, data[i]))
			return (BasicSerializedDataType)i;

	return SerialInvalid;
}

const char *SerialTypeToString(BasicSerializedDataType type)
{
	assert(type >= SerialInvalid);
	assert(type < NumSerialTypes); 
	return data[type];
}

size_t SerialTypeSize(BasicSerializedDataType type)
{
	assert(type >= SerialInvalid);
	assert(type < NumSerialTypes); 
	return typeSizes[type];	
}

SerializedElementDesc *SerializedMessageList::ParseNode(TiXmlElement *node, SerializedElementDesc *parentNode)
{
	elements.push_back(SerializedElementDesc());
	SerializedElementDesc *elem = &elements.back();
	elem->parent = parentNode;

	if (!strcmp(node->Value(), "message"))
	{
		elem->count = 1;
		elem->varyingCount = false;
		elem->type = SerialStruct;
	}
	else
	{
		// Cannot have both static count and dynamic count!
		assert(!node->Attribute("count") || !node->Attribute("varyingCount")); ///\todo Convert assert() to error checking.

		if (node->Attribute("count"))
		{
			node->QueryIntAttribute("count", &elem->count);
			elem->varyingCount = false;
		}
		else if (node->Attribute("dynamicCount"))
		{
			node->QueryIntAttribute("dynamicCount", &elem->count);
			elem->varyingCount = true;
		}
		else
		{
			elem->count = 1;
			elem->varyingCount = false;
		}

		elem->type = StringToSerialType(node->Value());
	}

	elem->name = node->Attribute("name") ? node->Attribute("name") : "";

	// If this node is a structure, parse all its members.
	if (elem->type == SerialStruct)
	{
		TiXmlElement *child = node->FirstChildElement();
		while(child)
		{
			SerializedElementDesc *childElem = ParseNode(child, elem);
			elem->elements.push_back(childElem);

			child = child->NextSiblingElement();
		}
	}

	return elem;
}

bool ParseBool(const char *str)
{
	if (!str)
		return false;

	if (!_stricmp(str, "true") || !_stricmp(str, "1"))
		return true;
	else
		return false;
}

void SerializedMessageList::ParseMessages(TiXmlElement *root)
{
	TiXmlElement *node = root->FirstChildElement("message");
	while(node)
	{
		SerializedMessageDesc desc;
		int success = node->QueryIntAttribute("id", (int*)&desc.id);
		if (success == TIXML_NO_ATTRIBUTE)
		{
//			std::cerr << "Error parsing message!" << std::endl; //\todo Print out a more descriptive warning.
			node = node->NextSiblingElement("message");
			continue; 
		}
		success = node->QueryIntAttribute("priority", (int*)&desc.priority);
		if (success == TIXML_NO_ATTRIBUTE)
			desc.priority = 0x7FFFFFFF;
		if (node->Attribute("name"))
			desc.name = node->Attribute("name");
		desc.reliable = ParseBool(node->Attribute("reliable"));
		desc.inOrder = ParseBool(node->Attribute("inOrder"));
		desc.data = ParseNode(node, 0);

		// Work a slight convenience - if there is a single struct inside a single struct inside a single struct - jump straight through to the data.

		messages.push_back(desc);

		node = node->NextSiblingElement("message");
	}
}

void SerializedMessageList::ParseStructs(TiXmlElement *root)
{
	TiXmlElement *node = root->FirstChildElement("struct");
	while(node)
	{
		ParseNode(node, 0);

		node = node->NextSiblingElement("struct");
	}
}

void SerializedMessageList::LoadMessagesFromFile(const char *filename)
{
	TiXmlDocument doc(filename);
	bool success = doc.LoadFile();
	if (!success)
		return; ///\todo Print out/return an error.

	TiXmlElement *xmlRoot = doc.FirstChildElement();

	ParseStructs(xmlRoot);
	ParseMessages(xmlRoot);
}


const SerializedMessageDesc *SerializedMessageList::FindMessageByID(u32 id)
{
	for(std::list<SerializedMessageDesc>::iterator iter = messages.begin();
		iter != messages.end(); ++iter)
		if (iter->id == id)
			return &*iter;

	return 0;
}

const SerializedMessageDesc *SerializedMessageList::FindMessageByName(const char *name)
{
	for(std::list<SerializedMessageDesc>::iterator iter = messages.begin();
		iter != messages.end(); ++iter)
		if (iter->name == name)
			return &*iter;

	return 0;
}
