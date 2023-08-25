#include "Precompiled.h"
#include "CameraNode.h"
#include "../World.h"
#include "Suora/Renderer/Shader.h"
#include "Suora/Renderer/Renderer3D.h"
#include "Suora/Assets/Mesh.h"
#include "Suora/Assets/Material.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace Suora
{
	CameraNode::CameraNode()
	{
		RecalculateProjection();
	}

	void CameraNode::Begin()
	{
		if (m_AutoPossess && GetWorld()->GetMainCamera() == nullptr)
		{
			GetWorld()->SetMainCamera(this);
		}
		RecalculateProjection();
	}

	void CameraNode::SetPerspective(float verticalFOV, float nearClip, float farClip)
	{
		m_ProjectionType = ProjectionType::Perspective;
		m_PerspectiveFOV = verticalFOV;
		m_PerspectiveNear = nearClip;
		m_PerspectiveFar = farClip;
		RecalculateProjection();
	}
	void CameraNode::SetOrthographic(float size, float nearClip, float farClip)
	{
		m_ProjectionType = ProjectionType::Orthographic;
		m_OrthographicSize = size;
		m_OrthographicNear = nearClip;
		m_OrthographicFar = farClip;
		RecalculateProjection();
	}
	void CameraNode::SetViewportSize(uint32_t width, uint32_t height)
	{
		m_AspectRatio = (float)width / (float)height;
		RecalculateProjection();
	}


	// https://stackoverflow.com/questions/29997209/opengl-c-mouse-ray-picking-glmunproject
	glm::vec3 CameraNode::ScreenPosToWorldDirection(const glm::vec2& pos, float windowWidth, float windowHeight) const
	{
		// these positions must be in range [-1, 1] (!!!), not [0, width] and [0, height]
		const float mouseX = pos.x / (windowWidth * 0.5f) - 1.0f;
		const float mouseY = pos.y / (windowHeight * 0.5f) - 1.0f;

		glm::mat4 proj = glm::perspectiveLH(glm::radians(m_PerspectiveFOV), m_AspectRatio, m_PerspectiveNear, m_PerspectiveFar);
		glm::mat4 view = glm::lookAt(glm::vec3(0.0f), GetForwardVector(), GetUpVector());

		glm::mat4 invVP = glm::inverse(proj * view);
		glm::vec4 screenPos = glm::vec4(mouseX, -mouseY, 1.0f, 1.0f);
		glm::vec4 worldPos = invVP * screenPos;

		glm::vec3 dir = glm::normalize(glm::vec3(worldPos));

		return dir * -1.0f;
	}

	// https://stackoverflow.com/questions/65208526/problem-with-bounding-sphere-frustum-culling
	bool CameraNode::IsInFrustum(const Vec3& pos, float radius) const
	{
		if (GetProjectionType() == ProjectionType::Orthographic) return true;

		struct Plane 
		{
			float a, b, c, d;

			void set(glm::vec3 p0, glm::vec3 p1, glm::vec3 p2) 
			{
				using namespace glm;
				vec3 v = p1 - p0;
				vec3 u = p2 - p0;
				vec3 n = normalize(cross(v, u));
				a = n.x;
				b = n.y;
				c = n.z;
				d = dot(-n, p0);
			}
			void set(glm::vec3 p, glm::vec3 n) 
			{
				using namespace glm;
				a = n.x;
				b = n.y;
				c = n.z;
				//d = p.x * n.x + p.y * n.y + p.z * n.z;
				d = dot(-n, p);
			}
			float distanceTo(glm::vec3 p) const 
			{
				return a * p.x + b * p.y + c * p.z + d;
			}
		};

		Plane front, back, left, right, top, bottom;

		{
			using namespace glm;
			vec3 ndir = normalize(GetForwardVector());
			vec3 nup = normalize(GetUpVector());
			vec3 nright = normalize(cross(GetForwardVector(), GetUpVector()));
			//aligned planes
			vec3 onn = GetPosition() + ndir * GetPerspectiveNearClip();
			vec3 onf = GetPosition() + ndir * GetPerspectiveFarClip();
			front.set(onn, ndir);
			back.set(onf, -ndir);
			//far and near dimensions
			vec2 fardim = vec2(0.0f, GetPerspectiveFarClip() * tan(radians(GetPerspectiveVerticalFOV() / 1.0f))); // was / 2.0f
			fardim.x = fardim.y * GetAspectRatio();
			vec2 neardim = vec2(0.0f, GetPerspectiveNearClip() * tan(radians(GetPerspectiveVerticalFOV() / 1.0f)));// was / 2.0f
			neardim.x = neardim.y * GetAspectRatio();
			//view frustum corners //l = left, r = right, u = up, b=bottom, n= near, f = far
			vec3 n_lu = onn + nup * neardim.y * 0.5f - nright * neardim.x * 0.5f;
			vec3 n_ru = onn + nup * neardim.y * 0.5f + nright * neardim.x * 0.5f;
			vec3 n_rb = onn - nup * neardim.y * 0.5f + nright * neardim.x * 0.5f;
			vec3 n_lb = onn - nup * neardim.y * 0.5f - nright * neardim.x * 0.5f;


			vec3 f_lu = onf + nup * fardim.y * 0.5f - nright * fardim.x * 0.5f;
			vec3 f_ru = onf + nup * fardim.y * 0.5f + nright * fardim.x * 0.5f;
			vec3 f_rb = onf - nup * fardim.y * 0.5f + nright * fardim.x * 0.5f;
			vec3 f_lb = onf - nup * fardim.y * 0.5f - nright * fardim.x * 0.5f;
			//non aligned planes
			vec3 leftn = normalize(cross(n_lb - n_lu, f_lu - n_lu));
			left.set(n_lu, leftn);

			vec3 rightn = normalize(cross(n_ru - n_rb, f_rb - n_rb));
			right.set(n_ru, rightn);

			vec3 topn = normalize(cross(n_lu - n_ru, f_ru - n_ru));
			top.set(n_ru, topn);

			vec3 botn = normalize(cross(f_rb - n_rb, n_lb - n_rb));
			bottom.set(n_rb, topn);
		}

		const float dt = top.distanceTo(pos);
		const float db = bottom.distanceTo(pos);
		const float dl = left.distanceTo(pos);
		const float dr = right.distanceTo(pos);
		const float df = front.distanceTo(pos);
		const float dn = back.distanceTo(pos);
		bool ix, iy, iz;
		ix = (dl > 0 && dr > 0) || glm::abs(dl) <= radius || glm::abs(dr) <= radius;
		iy = (dt > 0 && db > 0) || glm::abs(dt) <= radius || glm::abs(db) <= radius;
		iz = (df > 0 && dn > 0) || glm::abs(df) <= radius || glm::abs(dn) <= radius;

		return ix && iy && iz;
	}

	void CameraNode::OnPossess()
	{
		Super::OnPossess();

		if (m_IsPossessable)
		{
			GetWorld()->SetMainCamera(this);
			RecalculateProjection();
		}
	}

	void CameraNode::RecalculateProjection()
	{
		if (m_ProjectionType == ProjectionType::Perspective)
		{
			m_Projection = glm::perspectiveLH(glm::radians(m_PerspectiveFOV), m_AspectRatio, m_PerspectiveNear, m_PerspectiveFar);
		}
		else
		{
			const float orthoLeft = -m_OrthographicSize * m_AspectRatio * 0.5f;
			const float orthoRight = m_OrthographicSize * m_AspectRatio * 0.5f;
			const float orthoBottom = -m_OrthographicSize * 0.5f;
			const float orthoTop = m_OrthographicSize * 0.5f;

			m_Projection = glm::orthoLH(orthoLeft, orthoRight,
				orthoBottom, orthoTop, m_OrthographicNear, m_OrthographicFar);
		}
	}

}