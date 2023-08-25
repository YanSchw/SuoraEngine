#pragma once
#include <vector>
#include <string>
#include "inttypes.h"

namespace Suora
{

	struct DebugMessage
	{
		DebugMessage(const std::string& str, LogCategory category, LogLevel level)
			: m_Message(str), m_Category(category), m_Level(level) {}

		std::string m_Message;
		LogCategory m_Category;
		LogLevel m_Level;
	};

	struct VirtualConsole
	{
		inline static std::vector<DebugMessage> m_Messages;
		inline static uint64_t m_MaxMessageCount = 250;

		static void Tick()
		{
			if (m_Messages.size() > m_MaxMessageCount)
				m_Messages.erase(m_Messages.begin(), m_Messages.begin() + (m_Messages.size() - m_MaxMessageCount));
		}
		static void Clear()
		{
			m_Messages.clear();
		}
		static std::vector<DebugMessage> GetMessagesWithLevel(LogLevel level)
		{
			std::vector<DebugMessage> vec;

			for (auto& It : m_Messages)
			{
				if (It.m_Level == level)
				{
					vec.push_back(It);
				}
			}

			return vec;
		}
		static std::vector<DebugMessage> GetLogMessages()
		{
			return GetMessagesWithLevel(LogLevel::Info);
		}
		static std::vector<DebugMessage> GetWarnMessages()
		{
			return GetMessagesWithLevel(LogLevel::Warn);
		}
		static std::vector<DebugMessage> GetErrorMessages()
		{
			return GetMessagesWithLevel(LogLevel::Error);
		}

	};

}