#include "RexUUID.h"

#include <sstream>
#include <iomanip>

using namespace std;

std::string RexUUID::ToString() const
{
    stringstream str;
    int i = 0;

    for(int j = 0; j < 4; ++j)
        str << hex << setw(2) << setfill('0') << (int)data[i++];
    str << "-";

    for(int j = 0; j < 2; ++j)
        str << hex << setw(2) << setfill('0') << (int)data[i++];
    str << "-";

    for(int j = 0; j < 2; ++j)
        str << hex << setw(2) << setfill('0') << (int)data[i++];
    str << "-";

    for(int j = 0; j < 2; ++j)
        str << hex << setw(2) << setfill('0') << (int)data[i++];
    str << "-";

    for(int j = 0; j < 6; ++j)
        str << hex << setw(2) << setfill('0') << (int)data[i++];

    return str.str();
}

std::string RexUUID::ToShortString() const
{
    stringstream str;
    int i = 0;

    for(int j = 0; j < 4; ++j)
        str << hex << setw(2) << setfill('0') << (int)data[i++];

    return str.str();
}