/** @file
	@author Jukka Jylänki

	This work is copyrighted material and may NOT be used for any kind of commercial or 
	personal advantage and may NOT be copied or redistributed without prior consent
	of the author(s). 

	@brief
*/
#ifndef SerializationStructCompiler_h
#define SerializationStructCompiler_h

#include <string>
#include <fstream>

#include "MessageListParser.h"

/// Generates .h files out of XML description files for serializable structs and network messages.
class SerializationStructCompiler
{
public:
	void CompileStruct(const SerializedElementDesc &structure, const char *outfile);
	void CompileMessage(const SerializedMessageDesc &message, const char *outfile);

	static std::string ParseToValidCSymbolName(const char *str);

private:
	void WriteFilePreamble(std::ofstream &out);
	void WriteStruct(const SerializedElementDesc &elem, int level, std::ofstream &out);
	void WriteMessage(const SerializedMessageDesc &message, std::ofstream &out);

	void WriteMemberDefinition(const SerializedElementDesc &elem, int level, std::ofstream &out);
	void WriteStructMembers(const SerializedElementDesc &elem, int level, std::ofstream &out);
	void WriteNestedStructs(const SerializedElementDesc &elem, int level, std::ofstream &out);
	void WriteStructSizeMemberFunction(const SerializedElementDesc &elem, int level, std::ofstream &out);
	void WriteSerializeMemberFunction(const SerializedElementDesc &elem, int level, std::ofstream &out);
	void WriteDeserializeMemberFunction(const SerializedElementDesc &elem, int level, std::ofstream &out);

	static std::string Indent(int level);
};

#endif
