#include "FirstPersonCharacter.h"
#include "Suora/Core/Engine.h"
#include "Suora/GameFramework/InputModule.h"


FirstPersonCharacter::FirstPersonCharacter()
{
	SetUpdateFlag(UpdateFlag::WorldUpdate);

	m_Camera = CreateChild<CameraNode>();
	m_Camera->SetPosition(GetPosition() + Vec3(0, 0.5f, 0));
	m_Camera->SetPerspectiveVerticalFOV(90.0f);

	PlayerInputNode* input = CreateChild<PlayerInputNode>();
	input->BindAxis2D("Default.Move", this, &FirstPersonCharacter::Move);
	input->BindAxis2D("Default.Look", this, &FirstPersonCharacter::Look);
	input->BindAction("Default.Jump", InputActionKind::Pressed, this, &FirstPersonCharacter::Jump);
	input->m_LockInputCursor = true;
}

void FirstPersonCharacter::Move(const Vec2& axis2D)
{
	AddMovementInput((GetForwardVector() * axis2D.y + GetRightVector() * axis2D.x) * m_MovementSpeed * Engine::Get()->GetDeltaTime());
}

void FirstPersonCharacter::Look(const Vec2& axis2D)
{
	m_EulerRot += Vec3(axis2D.y, axis2D.x * 1, 0) * 0.5f;
	m_EulerRot.x = glm::clamp(m_EulerRot.x, -90.f, 90.f);
	m_Camera->SetLocalEulerRotation(Vec3(m_EulerRot.x, 0, 0));
	SetEulerRotation(Vec3(0, m_EulerRot.y, 0));
}

void FirstPersonCharacter::Jump()
{
	Super::Jump(3.25f);
}
