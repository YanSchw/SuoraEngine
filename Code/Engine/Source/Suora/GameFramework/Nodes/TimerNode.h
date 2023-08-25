#pragma once
#include "Suora/GameFramework/Node.h"
#include "Suora/Common/Delegate.h"
#include <vector>
#include "TimerNode.generated.h"

namespace Suora
{

	class TimerNode : public Node
	{
		SUORA_CLASS(754685479);

	public:
		Delegate<DelegateNoParams> OnTimerComplete;
		bool m_Loop = false;

		TimerNode();
		~TimerNode();
		void Begin() override;
		void WorldUpdate(float deltaTime) override;

		void SetTimer(float time);
		void ResetTimer();

	private:
		float m_ElapsedTime = 0;
		float m_TargetTime = 0;
	};

}