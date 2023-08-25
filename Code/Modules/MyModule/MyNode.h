#pragma once
#include "CoreMinimal.h"
#include "Suora/GameFramework/Node.h"
#include "Suora/GameFramework/World.h"
#include "Suora/GameFramework/Nodes/PhysicsNodes.h"
#include "Suora/GameFramework/InputModule.h"
#include "Suora/GameFramework/Nodes/MeshNode.h"
#include "Suora/GameFramework/Nodes/CameraNode.h"
#include "Suora/Core/Object/Object.h"
#include "Suora/Core/NativeInput.h"
#include "Suora/Core/Engine.h"
#include "Suora/Core/Log.h"
#include "MyNode.generated.h"

using namespace Suora;

namespace MyModule
{

	class MyTrigger : public Suora::BoxCollisionNode
	{
		SUORA_CLASS(5784933434);
	public:
		MyTrigger()
		{
			IsTrigger = true;
			OnTriggerEnter.Register([](Suora::RigidbodyNode* rb, CollisionNode* other) { SUORA_ERROR(LogCategory::Gameplay, "OnTriggerEnter"); });
			OnTriggerStay.Register([](Suora::RigidbodyNode* rb, CollisionNode* other) { SUORA_ERROR(LogCategory::Gameplay, "OnTriggerStay"); });
			OnTriggerExit.Register([](Suora::RigidbodyNode* rb, CollisionNode* other) { SUORA_ERROR(LogCategory::Gameplay, "OnTriggerExit"); });
		}
	};

	class ProjectileBase : public Suora::RigidbodyNode
	{
		SUORA_CLASS(435867476);
	public:

		MEMBER()
		float ProjectileForce = 20000.0f;

		Suora::Vec3 m_LastPos = Suora::Vec::Zero;

		ProjectileBase()
		{
			SetUpdateFlag(Suora::UpdateFlag::WorldUpdate);
			CreateChild<Suora::MeshNode>();
		}
		void Begin() override
		{
			Super::Begin();

			m_LastPos = GetPosition();
			SetForce(GetForwardVector() * ProjectileForce);
		}
		void WorldUpdate(float deltaTime) override;
	};
	
	class MyCharacterController : public Suora::CharacterNode
	{
		SUORA_CLASS(869722254);
	public:

		Suora::Vec3 m_EulerRot = Suora::Vec::Zero;
		Suora::CameraNode* m_Camera;
		const Suora::Class BP_Projectile = Suora::Class::FromString("Node$bwaf6a58-d932-4c5d-b591-9b818d5b97c9");

		MEMBER()
		Suora::Class m_Class = Suora::Class::None;
		MEMBER()
		Suora::SubclassOf<ProjectileBase> m_Subclass;

	public:

		MyCharacterController()
		{
			m_Camera = CreateChild<Suora::CameraNode>();
			m_Camera->SetPosition(GetPosition() + Suora::Vec3(0, 0.5f, 0));

			CreateChild<Suora::BoxCollisionNode>();
		}
		void Begin() override
		{
			Super::Begin();

			GetChildNodeOfClass<Suora::CollisionNode>()->SetLocalScale(Suora::Vec3(1, 2, 1));
			
			Possess();

			SetUpdateFlag(Suora::UpdateFlag::WorldUpdate);

			m_Camera->SetPerspectiveVerticalFOV(90.0f);
		}
		void InputJump()
		{
			if (IsCharacterGrounded())
			{
				AddWorldOffset(Suora::Vec::Up * 0.01f);
				Jump(30000.0f);
			}
		}
		void Move(const Suora::Vec2& value)
		{
			m_MovementSpeed = Suora::NativeInput::GetKey(Suora::Key::LeftShift) ? 400.0f : 250.0f;
			AddMovementInput(Suora::Engine::Get()->GetDeltaTime() * (GetForwardVector() * value.y + GetRightVector() * value.x));
		}
		void Look(const Suora::Vec2& value)
		{
			m_EulerRot += Suora::Vec3(value.y, value.x * 1, 0) * 0.5f;
			m_EulerRot.x = glm::clamp(m_EulerRot.x, -90.f, 90.f);

		}
		void Fire()
		{
			GetWorld()->Spawn(BP_Projectile, m_Camera->GetPosition() + m_Camera->GetForwardVector() * 1.2f, m_Camera->GetRotation());

			Suora::HitResult result;
			Suora::RaycastParams params;
			params.IgnoredCollisionNodes.Add(GetChildNodeOfClass<Suora::BoxCollisionNode>());
		}
		void Aim()
		{
		}
		void AimReleased()
		{
		}
		void WorldUpdate(float deltaTime) override
		{
			Super::WorldUpdate(deltaTime);
			ProcessInput();
			m_Camera->Possess();
			GetWorld()->SetMainCamera(m_Camera);

			m_Camera->SetLocalEulerRotation(Suora::Vec3(m_EulerRot.x, 0, 0));
			SetEulerRotation(Suora::Vec3(0, m_EulerRot.y, 0));

		}
		void SetupInputModule(Suora::InputModule* input) override
		{
			Super::SetupInputModule(input);

			input->m_LockInputCursor = true;

			input->BindAction("Gameplay/Jump", Suora::InputActionParam::Pressed, this, &MyCharacterController::InputJump);
			input->BindAction("Gameplay/Fire", Suora::InputActionParam::Pressed, this, &MyCharacterController::Fire);
			input->BindAction("Gameplay/Aim", Suora::InputActionParam::Pressed, this, &MyCharacterController::Aim);
			input->BindAction("Gameplay/Aim", Suora::InputActionParam::Released, this, &MyCharacterController::AimReleased);
			input->BindAxis2D("Gameplay/Move", this, &MyCharacterController::Move);
			input->BindAxis2D("Gameplay/Look", this, &MyCharacterController::Look);
		}
		bool IsCharacterGrounded()
		{
			Suora::HitResult hit;
			Suora::RaycastParams params;
			params.IgnoredCollisionNodes.Add(GetChildNodeOfClass<Suora::CollisionNode>());
			return GetWorld()->Raycast(GetPosition(), GetPosition() - Suora::Vec::Up * 1.01f, hit, params);
		}

	};

	class DamagableNodeComponent : public Suora::Component
	{
		SUORA_CLASS(7582439);
	public:
		MEMBER()
		float m_Health = 100.0f;

		FUNCTION(Callable)
		void Damage(float amount);

		MEMBER()
		Suora::Delegate<Suora::DelegateNoParams> OnDeath;
	};
	
}