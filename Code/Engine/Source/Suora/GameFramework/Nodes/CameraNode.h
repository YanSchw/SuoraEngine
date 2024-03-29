#pragma once
#include "Suora/GameFramework/Node.h"
#include "Suora/Common/VectorUtils.h"
#include "CameraNode.generated.h"

namespace Suora
{

	class CameraNode : public Node3D
	{
		SUORA_CLASS(4578654769);

	public:
		enum class ProjectionType { Perspective = 0, Orthographic = 1 };
	public:
		CameraNode();

		void Begin() override;

		virtual void TickTransform(bool inWorldSpace) override;

		FUNCTION(Callable)
		void SetPerspective(float verticalFOV, float nearClip, float farClip);
		FUNCTION(Callable)
		void SetOrthographic(float size, float nearClip, float farClip);

		FUNCTION(Callable)
		void SetViewportSize(uint32_t width, uint32_t height);

		FUNCTION(Callable)
		void SetAspectRatio(float aspect) { m_AspectRatio = aspect; }
		FUNCTION(Callable, Pure)
		float GetAspectRatio() const { return m_AspectRatio; }

		FUNCTION(Callable, Pure)
		float GetPerspectiveVerticalFOV() const { return m_PerspectiveFOV; }
		FUNCTION(Callable)
		void SetPerspectiveVerticalFOV(float verticalFov) { m_PerspectiveFOV = verticalFov; RecalculateProjection(); }
		FUNCTION(Callable, Pure)
		float GetPerspectiveNearClip() const { return m_PerspectiveNear; }
		FUNCTION(Callable)
		void SetPerspectiveNearClip(float nearClip) { m_PerspectiveNear = nearClip; RecalculateProjection(); }
		FUNCTION(Callable, Pure)
		float GetPerspectiveFarClip() const { return m_PerspectiveFar; }
		FUNCTION(Callable)
		void SetPerspectiveFarClip(float farClip) { m_PerspectiveFar = farClip; RecalculateProjection(); }

		FUNCTION(Callable, Pure)
		float GetOrthographicSize() const { return m_OrthographicSize; }
		FUNCTION(Callable)
		void SetOrthographicSize(float size) { m_OrthographicSize = size; RecalculateProjection(); }
		FUNCTION(Callable, Pure)
		float GetOrthographicNearClip() const { return m_OrthographicNear; }
		FUNCTION(Callable)
		void SetOrthographicNearClip(float nearClip) { m_OrthographicNear = nearClip; RecalculateProjection(); }
		FUNCTION(Callable, Pure)
		float GetOrthographicFarClip() const { return m_OrthographicFar; }
		FUNCTION(Callable)
		void SetOrthographicFarClip(float farClip) { m_OrthographicFar = farClip; RecalculateProjection(); }

		ProjectionType GetProjectionType() const { return m_ProjectionType; }
		void SetProjectionType(ProjectionType type) { m_ProjectionType = type; RecalculateProjection(); }

		Vec3 ScreenPosToWorldDirection(const Vec2& pos, float windowWidth, float windowHeight) const;

		FUNCTION(Callable, Pure)
		bool IsInFrustum(const Vec3& pos, float radius) const;

		virtual void OnPossess() override;

	public:
		void RecalculateProjection();

		const Mat4& GetProjectionMatrix() const { return m_Projection; }
		const Mat4& GetViewProjectionMatrix() const { return m_ViewProjection; }

		void SetAutoPossess(bool autoPossess) { m_AutoPossess = autoPossess; }
		bool IsAutoPossessed() const { return m_AutoPossess; }
		void SetPossessable(bool possessable) { m_IsPossessable = possessable; }
		bool IsPossessable() const { return m_IsPossessable; }
		void SetClearColor(Color color) { m_ClearColor = color; }
		Color GetClearColor() const { return m_ClearColor; }

	protected:
		Mat4 m_Projection = Mat4(1.0f);
		Mat4 m_ViewProjection = Mat4(1.0f);

	private:
		ProjectionType m_ProjectionType = ProjectionType::Perspective;

		float m_PerspectiveFOV = 65.0f;
		float m_PerspectiveNear = 0.01f, m_PerspectiveFar = 1000.0f;

		float m_OrthographicSize = 10.0f;
		float m_OrthographicNear = -1.0f, m_OrthographicFar = 1.0f;

		float m_AspectRatio = 1.0f;

		PROPERTY()
		bool m_AutoPossess = true;
		PROPERTY()
		bool m_IsPossessable = true;
		PROPERTY()
		Color m_ClearColor = Color(0, 0, 0, 1);

		friend class World;
		friend class Renderer3D;
		friend class RenderPipeline;
		friend class DirectionalLightNode;
	};
}