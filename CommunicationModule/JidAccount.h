#pragma once

#include <string>

namespace Communication
{

	class JidAccount
	{
	public:
		JidAccount(void);
		~JidAccount(void);
		std::string name;
		std::string password;
	};

}