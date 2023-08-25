#include "MyNode.h"

namespace MyModule
{

	void DamagableNodeComponent::Damage(float amount)
	{
		SuoraVerify(GetParent());

		m_Health -= amount;
		if (m_Health <= 0.0f)
		{
			OnDeath.Execute(Suora::TDelegate::NoParam);
			GetActorNode()->Destroy();
		}
	}

	void ProjectileBase::WorldUpdate(float deltaTime)
	{
		Super::WorldUpdate(deltaTime);

		Suora::HitResult result;
		if (GetWorld()->Raycast(m_LastPos, GetPosition(), result))
		{
			if (result.PhysicsBody && result.PhysicsBody->GetActorNode())
			{
				if (DamagableNodeComponent* comp = result.PhysicsBody->GetActorNode()->GetChildNodeOfClass<DamagableNodeComponent>())
				{
					comp->Damage(50.0f);
				}
			}
			
			Destroy();
			SUORA_LOG(Suora::LogCategory::Gameplay, Suora::LogLevel::Info, "Projectile Destroyed.");
		}
		m_LastPos = GetPosition();
	}

}