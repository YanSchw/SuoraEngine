#include "Log.h"

#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>

#include <memory>

static std::shared_ptr<spdlog::logger> s_Logger;

static bool s_WasInitialized = false;

spdlog::logger* Log::GetLogger()
{

	if (!s_WasInitialized)
	{
		s_WasInitialized = true;

		std::vector<spdlog::sink_ptr> logSinks;
		logSinks.emplace_back(std::make_shared<spdlog::sinks::stdout_color_sink_mt>());
		logSinks.emplace_back(std::make_shared<spdlog::sinks::basic_file_sink_mt>("SuoraBuildTool.log", true));

		logSinks[0]->set_pattern("%^[%T] %n: %v%$");
		logSinks[1]->set_pattern("[%T] [%l] %n: %v");


		s_Logger = std::make_shared<spdlog::logger>("[Build]", begin(logSinks), end(logSinks));
		spdlog::register_logger(s_Logger);
		s_Logger->set_level(spdlog::level::trace);
		s_Logger->flush_on(spdlog::level::trace);
	}

	return s_Logger.get();
}