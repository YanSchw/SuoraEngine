#pragma once
#include <vector>
#include <string>
#include "inttypes.h"

namespace Suora
{

	struct ConsoleMessage
	{
		ConsoleMessage(const std::string& str, LogCategory category, LogLevel level)
			: m_Message(str), m_Category(category), m_Level(level) {}

		ConsoleMessage(std::string_view message, const char* callerPath, const char* callerFunction, int32_t callerLine, LogLevel level, LogCategory category)
			: m_Message(message.data(), message.size()), m_CallerPath(callerPath), m_CallerFunction(callerFunction), m_CallerLine(callerLine), m_Level(level), m_Category(category)
		{
		}

		std::string m_Message;
		LogCategory m_Category;
		LogLevel m_Level;

		const char* m_CallerPath;
		const char* m_CallerFunction;
		int32_t m_CallerLine;
	};

	struct VirtualConsole
	{
		inline static std::vector<ConsoleMessage> m_Messages;
		inline static uint64_t m_MaxMessageCount = 250;

		static void Tick();
		static void Clear();
		static std::vector<ConsoleMessage> GetMessagesWithLevel(LogLevel level);
		static std::vector<ConsoleMessage> GetLogMessages();
		static std::vector<ConsoleMessage> GetWarnMessages();
		static std::vector<ConsoleMessage> GetErrorMessages();
		static void PushMessage(const ConsoleMessage& msg);

	};

}