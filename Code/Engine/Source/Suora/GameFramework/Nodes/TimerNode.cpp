#include "Precompiled.h"
#include "TimerNode.h"

namespace Suora
{

	TimerNode::TimerNode()
	{

	}
	TimerNode::~TimerNode()
	{

	}

	void TimerNode::Begin()
	{
		SetUpdateFlag(UpdateFlag::WorldUpdate);
	}

	void TimerNode::WorldUpdate(float deltaTime)
	{
		m_ElapsedTime += deltaTime;

		if (m_ElapsedTime >= m_TargetTime)
		{
			OnTimerComplete(TDelegate::NoParam);

			if (m_Loop) m_ElapsedTime = 0;
			else Destroy();
		}

	}

	void TimerNode::SetTimer(float time)
	{
		m_TargetTime = time;
	}

	void TimerNode::ResetTimer()
	{
		m_ElapsedTime = 0;
	}

}