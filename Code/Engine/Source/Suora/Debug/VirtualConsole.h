#pragma once
#include <vector>
#include "Suora/Common/StringUtils.h"
#include <cstdint>

namespace Suora
{

	struct ConsoleMessage
	{
		ConsoleMessage(const String& str, const LogCategory category, const LogLevel level)
			: m_Message(str), m_Category(category), m_Level(level) {}

		ConsoleMessage(const std::string_view message, const char* callerPath, const char* callerFunction, const int32_t callerLine, const LogLevel level, const LogCategory category)
			: m_Message(message.data(), message.size()), m_Level(level), m_Category(category), m_CallerPath(callerPath), m_CallerFunction(callerFunction), m_CallerLine(callerLine)
		{
		}

		String m_Message;
		LogCategory m_Category;
		LogLevel m_Level;

		const char* m_CallerPath = nullptr;
		const char* m_CallerFunction = nullptr;
		int32_t m_CallerLine = 0;
	};

	struct VirtualConsole
	{
		inline static std::vector<ConsoleMessage> m_Messages;
		inline static uint64_t m_MaxMessageCount = 250;

		static void Tick();
		static void Clear();
		static std::vector<ConsoleMessage> GetMessagesWithLevel(LogLevel level);
		static std::vector<ConsoleMessage> GetLogMessages();
		static std::vector<ConsoleMessage> GetDebugMessages();
		static std::vector<ConsoleMessage> GetWarnMessages();
		static std::vector<ConsoleMessage> GetErrorMessages();
		static void PushMessage(ConsoleMessage msg);

		static void IssueCommand(const String& cmd);

	};

}