#include "Precompiled.h"
#include "Suora/Core/Log.h"

#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>

#include "Suora/Debug/VirtualConsole.h"

namespace Suora 
{

	static std::unordered_map<uint32_t, Ref<spdlog::logger>> s_Loggers;

	void Log::Init()
	{
		std::vector<spdlog::sink_ptr> logSinks;
		logSinks.emplace_back(std::make_shared<spdlog::sinks::stdout_color_sink_mt>());
		logSinks.emplace_back(std::make_shared<spdlog::sinks::basic_file_sink_mt>("Suora.log", true));

		logSinks[0]->set_pattern("%^[%T] %n: %v%$");
		logSinks[1]->set_pattern("[%T] [%l] %n: %v");

		for (int32_t i = 0; i < (uint32_t)LogCategory::COUNT; i++)
		{
			s_Loggers[i] = std::make_shared<spdlog::logger>(CategoryToString((LogCategory)i), begin(logSinks), end(logSinks));
			spdlog::register_logger(s_Loggers[i]);
			s_Loggers[i]->set_level(spdlog::level::trace);
			s_Loggers[i]->flush_on(spdlog::level::trace);
		}
	}

	void Log::LogVirtualMessage(LogCategory category, LogLevel verbosity, const std::string& str)
	{
		VirtualConsole::m_Messages.push_back(DebugMessage(str, category, verbosity));
		VirtualConsole::Tick();
	}

	Ref<spdlog::logger>& Log::GetLogger(LogCategory category)
	{
		return s_Loggers[(uint32_t)category];
	}

	std::string Log::CategoryToString(LogCategory category)
	{
		switch (category)
		{
		case LogCategory::Core: return "[Core]";
		case LogCategory::Module: return "[Module]";
		case LogCategory::Debug: return "[Debug]";
		case LogCategory::Assert: return "[Assert]";
		case LogCategory::AssetManagement: return "[AssetManagement]";
		case LogCategory::Rendering: return "[Rendering]";
		case LogCategory::Gameplay: return "[Gameplay]";
		case LogCategory::Editor: return "[Editor]";
		case LogCategory::None:
		case LogCategory::COUNT:
		default:
			return "[None]";
		}

		return std::string();
	}

}

