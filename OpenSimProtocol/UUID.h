// For conditions of distribution and use, see copyright notice in license.txt
#ifndef incl_UUID_h
#define incl_UUID_h

#include "RexTypes.h"

/// UUID is a 16-byte identifier for resources in a virtual world.
class UUID
{
public:
	/// Constructs an UUID from a string in form "1c1bbda2-304b-4cbf-ba3f-75324b044c73" or "1c1bbda2304b4cbfba3f75324b044c73".
	explicit UUID(const char *str);
	explicit UUID(const std::string &str);
	/// Constructs a null UUID.
	UUID();

	/// Sets all 16 bytes of the ID to '00'.
	void SetNull();

	void FromString(const char *str);
	void FromString(const std::string &str) { FromString(str.c_str()); }

	std::string ToString() const;

	bool operator ==(const UUID &rhs) const;
	bool operator <(const UUID &rhs) const;

	static const uint8_t cSizeBytes = 16;

	uint8_t data[cSizeBytes];
};

#endif
