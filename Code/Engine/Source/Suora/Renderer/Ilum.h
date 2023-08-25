#pragma once

#include <vector>
#include <glm/glm.hpp>
#include "Suora/GameFramework/Nodes/CameraNode.h"
#include "Suora/Renderer/Buffer.h"

// For use in std::unordered_map
struct IVec3Hasher
{
	std::size_t operator()(const glm::ivec3& vec) const
	{
		return ((std::hash<int32_t>()(vec.x)) ^ (std::hash<int32_t>()(vec.y)) ^ (std::hash<int32_t>()(vec.z)));
	}
};
struct IVec2Hasher_
{
	std::size_t operator()(const glm::ivec2& vec) const
	{
		return ((std::hash<int32_t>()(vec.x)) ^ (std::hash<int32_t>()(vec.y)));
	}
};

namespace Suora
{
	class CameraNode;
	class Framebuffer;
	class World;
	class Shader;

	/* Ilum works in these steps:
	/*	1. Spawn new Surfels (e.g. in Screen-Space or in Surfel-View)
	/*	2. Capture Surfels (over multiple frames.)
	/*	3. Proccess / Gather the Indirect Ilumination
	/*	4. Apply the Indirect Ilumination (For <Multibounce> this is also done in step 2)
	*/

	class Ilum
	{
	public:
		struct LightProbe
		{
			Vec3 Position;
			Vec3 TargetPosition;
		};

		Ilum(World* world);
		~Ilum();
		void Init();

		void Tick(float deltaTime, Framebuffer& gBuffer, CameraNode& camera, World& world);

	private:
		void SurfelPass(Framebuffer& gBuffer, CameraNode& camera, World& world);
		void LightPropagationPass(Framebuffer& gBuffer);

	public:
		static bool IsInIlumPass()
		{
			return s_InIlumPass;
		}
		void ApplyIlumination(Framebuffer& gBuffer, CameraNode& camera, World& world, Framebuffer& lightBuffer);

	private:
		Ptr<World> m_World;
		bool m_Initialized = false;
		inline static bool s_InIlumPass = false;
	public:
		bool m_LightProbeScreenSpace = true;
		Ref<Framebuffer> m_SurfelCache;
		Ref<Framebuffer> m_SurfelDirectLightCache;
		Ref<Framebuffer> m_SurfelIlumCache;
		Ref<Framebuffer> m_SurfelIlumCacheAcc;
		Ref<Shader> m_SurfelPropagationShader;
		Ref<Shader> m_ApplySurfels;
		Ref<Shader> m_Denoiser;

		/** LightProbeGrid */
		Ref<Framebuffer> m_IluminationCache;
		Ref<Framebuffer> m_IluminationCacheAcc;
		Ref<Shader> m_LightPropagationShader;
		glm::ivec3 m_LightProbeGridPos = glm::ivec3();
		Vec3 m_LightProbeGridOffset = Vec3(1.5f, 2.25f, 1.5f);

	private:
		Ref<Shader> m_PosReadback;
		Ref<ShaderStorageBuffer> m_PosReadbackSSBO;
		Ref<Framebuffer> m_EmptyViewport; Ref<Framebuffer> m_EmptyViewport2;
		Ref<Framebuffer> m_SecondaryView;
		Ref<Shader> m_WriteToSurfelCache;

		friend class World;
		friend class RenderPipeline;
		friend class ViewportPanel;
	};

}

#if 0
namespace Suora
{
	class CameraNode;
	class Framebuffer;
	class World;
	class Shader;

	/* Ilum works in these steps:
	/*	1. Spawn new Surfels (e.g. in Screen-Space or in Surfel-View)
	/*	2. Capture Surfels (over multiple frames.)
	/*	3. Proccess / Gather the Indirect Ilumination
	/*	4. Apply the Indirect Ilumination (For <Multibounce> this is also done in step 2)
	*/

	class Ilum
	{
	public:
		struct LightProbe
		{
			Vec3 Position;
			Vec3 TargetPosition;
		};

		Ilum(World* world);
		~Ilum();
		void Init();

		void Tick(float deltaTime, Framebuffer& gBuffer);

		LightProbe& operator[](uint32_t i)
		{
			return m_LightProbes[i];
		}
		void ApplyIlumination(Ref<Framebuffer> target, Ref<Framebuffer> gBuffer, float intensity);
	private:
		Vec3 ReadScreenPosition();
		void SpawnSurfelsInScreenSpace(Ref<Framebuffer> gBuffer, int nSurfel);
		void Capture(const Vec3& position);
		void GatherFinalIlumination();

	public:
		bool m_Multibounce = false;
		static bool IsInIlumPass()
		{
			return s_InIlumPass;
		}

	private:
		World* m_World = nullptr;
		bool m_Initialized = false;
		inline static bool s_InIlumPass = false;
	public:
		LightProbe m_LightProbes[64];
		int32_t m_lightProbeIndex = 0;
		bool m_LightProbeScreenSpace = true;
		Ref<Framebuffer> m_ViewGBuffers;
		Ref<Framebuffer> m_ViewTop;
		Ref<Framebuffer> m_ViewBottom;
		Ref<Framebuffer> m_ViewLeft;
		Ref<Framebuffer> m_ViewRight;
		Ref<Framebuffer> m_ViewForward;
		Ref<Framebuffer> m_ViewBackward;
		Ref<Framebuffer> m_IluminationBuffer;
	private:
		Ref<Shader> m_PositionReadbackShader;
		Ref<ShaderStorageBuffer> m_PositionSSBO;
		Ref<Shader> m_GatherIluminationShader;

		uint32_t m_ViewResolution = 32;
		uint32_t m_ProbeSlotResolution = 16;

		std::unordered_map<glm::ivec2, Ref<Framebuffer>, IVec2Hasher_> m_PositonBufferSrc;
		Ref<Framebuffer> GetPositonBufferSrc(const glm::ivec2& size);

		friend class World;
		friend class RenderPipeline;
	};

}

#endif