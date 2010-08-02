#include <string>
#include <iostream>

#include "clb/Network/SerializationStructCompiler.h"
#include "clb/Network/MessageListParser.h"
#include "clb/Network/MessageConnection.h"

using namespace std;

int main(int argc, char **argv)
{
	if (argc < 2)
	{
		cout << "No parameters given." << endl;
		return 0;
	}

	SerializedMessageList msg;
	msg.LoadMessagesFromFile(argv[1]);

	cout << "File " << argv[1] << " contains the following structs:" << endl;

	const std::list<SerializedElementDesc> &elements = msg.GetElements();
	for(std::list<SerializedElementDesc>::const_iterator iter = elements.begin();
		iter != elements.end(); ++iter)
	{
		const SerializedElementDesc &elem = *iter;
		
		if (elem.type == SerialStruct)
			if (elem.name.length() == 0)
				cout << "<unnamed struct>" << endl;
			else
				cout << elem.name << endl;
	}

	const std::list<SerializedMessageDesc> &messages = msg.GetMessages();
	for(std::list<SerializedMessageDesc>::const_iterator iter = messages.begin();
		iter != messages.end(); ++iter)
	{
		const SerializedMessageDesc &msg = *iter;
		SerializationStructCompiler compiler;	
		string messageName = compiler.ParseToValidCSymbolName(msg.name.c_str()) + ".h";
		compiler.CompileMessage(msg, ("Msg" + messageName).c_str());
	}
}
