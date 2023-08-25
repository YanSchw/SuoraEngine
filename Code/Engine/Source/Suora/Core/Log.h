#pragma once

#include "Suora/Core/Base.h"

// This ignores all warnings raised inside External headers
#pragma warning(push, 0)
#include <spdlog/spdlog.h>
#include <spdlog/fmt/ostr.h>
#pragma warning(pop)

#include <unordered_map>
#include <memory>
#include <string>
#include <sstream>
#include <stdexcept>

static std::string string_format(std::string format, const std::vector<std::stringstream>& streams)
{
	std::string index = "";

	for (int i = 0; i < format.size(); i++)
	{
		if (format[i] == '{')
		{
			format.erase(i, 1);
			while (format[i] != '}')
			{
				index += format[i];
				format.erase(i, 1);
			}
			format.erase(i, 1);
			int32_t INDEX = std::stoi(index);
			index = "";
			
			if (streams.size() > INDEX)
			{
				/*std::stringstream ss;
				ss << streams[INDEX].rdbuf();*/
				format.insert(i, streams[INDEX].str());
			}
		}
	}

	return format;
}

template <class T, class... Ts>
static void string_format_all(std::vector<std::stringstream>& streams, T const& first, Ts const&... rest)
{
	streams.push_back(std::stringstream());
	std::stringstream& ss = streams[streams.size() - 1]; 
	ss << first;

	if constexpr (sizeof...(rest) > 0) 
	{
		// if rest... is empty, there will be no further call to
		// string_format_all(...). 
		string_format_all(streams, rest...);
	}
}

template<typename ... Args>
static std::string string_format(const std::string& format, Args ... args)
{
	std::vector<std::stringstream> streams;

	if constexpr (sizeof...(args) > 0)
		string_format_all(streams, args...);

	return string_format(format, streams);
}


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

		static void LogVirtualMessage(LogCategory category, LogLevel verbosity, const std::string& str);

		template<typename... Args>
		static void LogMessage(LogCategory category, LogLevel verbosity, std::string fmt, Args &&... args)
		{
			GetLogger(category)->log((::spdlog::level::level_enum)verbosity, ::spdlog::format_string_t<Args...>(fmt), std::forward<Args>(args)...);
			LogVirtualMessage(category, verbosity, string_format(fmt, args...));
		}

		static Ref<spdlog::logger>& GetLogger(LogCategory category);
	private:
		static std::string CategoryToString(LogCategory category);

	};

}

// Main Logging Macro
#define SUORA_LOG(_Category, _LogLevel, ...)    ::Suora::Log::LogMessage(_Category, _LogLevel, __VA_ARGS__)

#define SUORA_TRACE(_Category, ...)         ::Suora::Log::LogMessage(_Category, ::Suora::LogLevel::Trace, __VA_ARGS__)
#define SUORA_INFO(_Category, ...)          ::Suora::Log::LogMessage(_Category, ::Suora::LogLevel::Info, __VA_ARGS__)
#define SUORA_WARN(_Category, ...)          ::Suora::Log::LogMessage(_Category, ::Suora::LogLevel::Warn, __VA_ARGS__)
#define SUORA_ERROR(_Category, ...)         ::Suora::Log::LogMessage(_Category, ::Suora::LogLevel::Error, __VA_ARGS__)
#define SUORA_CRITICAL(_Category, ...)      ::Suora::Log::LogMessage(_Category, ::Suora::LogLevel::Critical, __VA_ARGS__)

#define SuoraLog(...)         SUORA_INFO(::Suora::LogCategory::None, __VA_ARGS__)
#define SuoraWarn(...)        SUORA_WARN(::Suora::LogCategory::None, __VA_ARGS__)
#define SuoraError(...)       SUORA_ERROR(::Suora::LogCategory::None, __VA_ARGS__)