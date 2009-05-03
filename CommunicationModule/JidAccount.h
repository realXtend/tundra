#pragma once

#include <string>

namespace Communication
{
	/* deprecated */
	class JidAccount
	{
	public:
		JidAccount(void);
		~JidAccount(void);
		std::string name;
		std::string password;
	};

}