#pragma once

#include "Suora/Core/Base.h"

// This ignores all warnings raised inside External headers
#pragma warning(push, 0)
#include <spdlog/spdlog.h>
#pragma warning(pop)

#include <string>

namespace Suora 
{

	enum class LogCategory : uint32_t
	{
		None = 0,
		Core,
		Module,
		Debug,
		Assert,
		AssetManagement,
		Rendering,
		Gameplay,
		Editor,
		COUNT
	};

	enum class LogLevel : uint32_t
	{
		Trace = 0,
		Debug,
		Info,
		Warn,
		Error,
		Critical
	};

	class Log
	{
	public:
		static void Init();

		static Ref<spdlog::logger>& GetLogger(LogCategory category);
	private:
		static std::string CategoryToString(const LogCategory category);

	};

}

// Main Logging Macro
#define SUORA_LOG(_Category, _LogLevel, ...)    ::Suora::Log::GetLogger(_Category)->log((::spdlog::level::level_enum)_LogLevel, __VA_ARGS__)

#define SUORA_TRACE(_Category, ...)         SUORA_LOG(_Category, ::Suora::LogLevel::Trace, __VA_ARGS__)
#define SUORA_INFO(_Category, ...)          SUORA_LOG(_Category, ::Suora::LogLevel::Info, __VA_ARGS__)
#define SUORA_WARN(_Category, ...)          SUORA_LOG(_Category, ::Suora::LogLevel::Warn, __VA_ARGS__)
#define SUORA_ERROR(_Category, ...)         SUORA_LOG(_Category, ::Suora::LogLevel::Error, __VA_ARGS__)
#define SUORA_CRITICAL(_Category, ...)      SUORA_LOG(_Category, ::Suora::LogLevel::Critical, __VA_ARGS__)

#define SuoraLog(...)         SUORA_INFO(::Suora::LogCategory::None, __VA_ARGS__)
#define SuoraWarn(...)        SUORA_WARN(::Suora::LogCategory::None, __VA_ARGS__)
#define SuoraError(...)       SUORA_ERROR(::Suora::LogCategory::None, __VA_ARGS__)