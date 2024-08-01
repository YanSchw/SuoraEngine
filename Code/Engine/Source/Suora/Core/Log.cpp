#include "Precompiled.h"
#include "Suora/Core/Log.h"

#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>

#include "Suora/Debug/VirtualConsole.h"
#include "Suora/Common/Common.h"
#include "Suora/Common/StringUtils.h"

namespace Suora 
{
	class VirtualConsoleSink : public spdlog::sinks::base_sink<std::mutex>
	{
	public:
		explicit VirtualConsoleSink(bool forceFlush = false, uint8_t bufferCapacity = 10, LogCategory category = LogCategory::None)
			: m_MessageBufferCapacity(forceFlush ? 1 : bufferCapacity), m_Category(category)
		{
		}
		VirtualConsoleSink(const VirtualConsoleSink&) = delete;
		VirtualConsoleSink& operator=(const VirtualConsoleSink&) = delete;
		~VirtualConsoleSink() override = default;

	protected:
		void sink_it_(const spdlog::details::log_msg& msg) override
		{
			spdlog::memory_buf_t formatted;
			base_sink<std::mutex>::formatter_->format(msg, formatted);
			const Ref<ConsoleMessage> outMsg = CreateRef<ConsoleMessage>(std::string_view(formatted.data(), formatted.size()), msg.source.filename, msg.source.funcname, msg.source.line, GetMessageLevel(msg.level), m_Category);

			m_MessageBuffer.push_back(outMsg);

			if (++m_MessagesBuffered == m_MessageBufferCapacity)
			{
				flush_();
			}
		}

		void flush_() override
		{
			for (const auto& msg : m_MessageBuffer)
			{
				VirtualConsole::PushMessage(*msg.get());
			}

			m_MessagesBuffered = 0;
			m_MessageBuffer.clear();
		}
	private:
		static LogLevel GetMessageLevel(const spdlog::level::level_enum level)
		{
			switch (level)
			{
			case spdlog::level::level_enum::off:			return static_cast<LogLevel>(0);
			case spdlog::level::level_enum::trace:			return LogLevel::Trace;
			case spdlog::level::level_enum::debug:			return LogLevel::Debug;
			case spdlog::level::level_enum::info:			return LogLevel::Info;
			case spdlog::level::level_enum::warn:			return LogLevel::Warn;
			case spdlog::level::level_enum::err:			return LogLevel::Error;
			case spdlog::level::level_enum::critical:		return LogLevel::Critical;
			case spdlog::level::level_enum::n_levels:		return static_cast<LogLevel>(0);
			}
			return LogLevel::Trace;
		}
	private:
		uint8_t m_MessagesBuffered = 0;
		uint8_t m_MessageBufferCapacity;
		std::vector<Ref<ConsoleMessage>> m_MessageBuffer;
		LogCategory m_Category = LogCategory::None;
	};


	static std::unordered_map<uint32_t, Ref<spdlog::logger>> s_Loggers;

	static Array<String> s_CustomCategories;

	void CreateSingleLogger(uint32_t categoryIdx)
	{
		std::vector<spdlog::sink_ptr> logSinks;
		logSinks.emplace_back(std::make_shared<spdlog::sinks::stdout_color_sink_mt>());
		logSinks.emplace_back(std::make_shared<spdlog::sinks::basic_file_sink_mt>("Suora.log", true));
		logSinks.emplace_back(std::make_shared<VirtualConsoleSink>(true, 10, (LogCategory)categoryIdx));

		logSinks[0]->set_pattern("%^[%T] %n: %v%$");
		logSinks[1]->set_pattern("[%T] [%l] %n: %v");
		logSinks[2]->set_pattern("%^[%T] %n: %v%$");

		s_Loggers[categoryIdx] = std::make_shared<spdlog::logger>(Log::CategoryToString((LogCategory)categoryIdx), begin(logSinks), end(logSinks));
		spdlog::register_logger(s_Loggers[categoryIdx]);
		s_Loggers[categoryIdx]->set_level(spdlog::level::trace);
		s_Loggers[categoryIdx]->flush_on(spdlog::level::trace);
	}

	void Log::Init()
	{
		for (uint32_t i = 0; i < (uint32_t)LogCategory::COUNT; i++)
		{
			CreateSingleLogger(i);
		}
	}

	Ref<spdlog::logger>& Log::GetLogger(LogCategory category)
	{
		return s_Loggers[static_cast<uint32_t>(category)];
	}

	LogCategory Log::CustomCategory(const String& label)
	{
		for (uint32_t i = 0; i < s_CustomCategories.Size(); i++)
		{
			if (s_CustomCategories[i] == label)
			{
				return static_cast<LogCategory>(static_cast<uint32_t>(LogCategory::COUNT) + i);
			}
		}

		s_CustomCategories.Add("[" + label + "]");
		const uint32_t categoryIdx = s_CustomCategories.Last() + static_cast<int32_t>(LogCategory::COUNT);
		s_Loggers[categoryIdx] = s_Loggers[0]->clone("[" + label + "]");
		return static_cast<LogCategory>(categoryIdx);
	}

	String Log::CategoryToString(const LogCategory category)
	{
		int32_t customIdx = static_cast<int32_t>(category) - static_cast<int32_t>(LogCategory::COUNT);
		if (customIdx >= 0)
		{
			return s_CustomCategories[customIdx];
		}

		switch (category)
		{
		case LogCategory::Core: return "[Core]";
		case LogCategory::Module: return "[Module]";
		case LogCategory::Debug: return "[Debug]";
		case LogCategory::Assert: return "[Assert]";
		case LogCategory::AssetManagement: return "[AssetManagement]";
		case LogCategory::Rendering: return "[Rendering]";
		case LogCategory::Gameplay: return "[Gameplay]";
		case LogCategory::Scripting: return "[Scripting]";
		case LogCategory::Editor: return "[Editor]";
		case LogCategory::None:
		case LogCategory::COUNT:
		default:
			return "[None]";
		}

		return String();
	}

}
