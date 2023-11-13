#pragma once

// This ignores all warnings raised inside External headers
#pragma warning(push, 0)
#include <spdlog/spdlog.h>
#pragma warning(pop)

class Log
{
public:
	static spdlog::logger* GetLogger();

};

// Main Logging Macro
#define BUILD_LOG(_LogLevel, ...)    ::Log::GetLogger()->log((::spdlog::level::level_enum)_LogLevel, __VA_ARGS__)

#define BUILD_INFO(...) BUILD_LOG(::spdlog::level::level_enum::info, __VA_ARGS__)
#define BUILD_DEBUG(...) BUILD_LOG(::spdlog::level::level_enum::debug, __VA_ARGS__)