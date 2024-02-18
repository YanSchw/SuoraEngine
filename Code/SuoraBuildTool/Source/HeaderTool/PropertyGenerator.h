#pragma once
#include <string>

namespace Suora::Tools
{
	struct PropertyGenerator
	{
		static std::string GeneratePropertyRef(std::string memberType);
	};
}