#include "Precompiled.h"
#include "VirtualConsole.h"
#include "Suora/Common/Common.h"

namespace Suora
{

	void VirtualConsole::Tick()
	{
		if (m_Messages.size() > m_MaxMessageCount)
			m_Messages.erase(m_Messages.begin(), m_Messages.begin() + (m_Messages.size() - m_MaxMessageCount));
	}
	void VirtualConsole::Clear()
	{
		m_Messages.clear();
	}
	std::vector<ConsoleMessage> VirtualConsole::GetMessagesWithLevel(LogLevel level)
	{
		std::vector<ConsoleMessage> vec;

		for (auto& It : m_Messages)
		{
			if (It.m_Level == level)
			{
				vec.push_back(It);
			}
		}

		return vec;
	}

	std::vector<ConsoleMessage> VirtualConsole::GetLogMessages()
	{
		return GetMessagesWithLevel(LogLevel::Info);
	}
	std::vector<ConsoleMessage> VirtualConsole::GetDebugMessages()
	{
		return GetMessagesWithLevel(LogLevel::Debug);
	}
	std::vector<ConsoleMessage> VirtualConsole::GetWarnMessages()
	{
		return GetMessagesWithLevel(LogLevel::Warn);
	}
	std::vector<ConsoleMessage> VirtualConsole::GetErrorMessages()
	{
		return GetMessagesWithLevel(LogLevel::Error);
	}

	void VirtualConsole::PushMessage(ConsoleMessage msg)
	{
		while (StringUtil::ReplaceSequence(msg.m_Message, "\r", ""));
		m_Messages.push_back(msg);
	}
	void VirtualConsole::IssueCommand(const String& cmd)
	{
		SuoraError("Command: {0}", cmd);
	}
}