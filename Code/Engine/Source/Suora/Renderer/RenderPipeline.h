#pragma once
#include "Suora/Core/Object/Object.h"
#include "Suora/Core/EngineSubSystem.h"
#include "VertexArray.h"
#include "Suora/Common/VectorUtils.h"
#include <glm/glm.hpp>
#include <thread>
#include "RenderPipeline.generated.h"

// For use in std::unordered_map
struct IVec2Hasher
{
	std::size_t operator()(const glm::ivec2& vec) const
	{
		return ((std::hash<int32_t>()(vec.x)) ^ (std::hash<int32_t>()(vec.y)));
	}
};

namespace Suora
{
	class Ilum;
	class Decima;
	class World;
	class CameraNode;
	class Shader;
	class Framebuffer;
	class VertexArray;
	class ShaderStorageBuffer;

	struct FramebufferTextureParams;
	enum class FramebufferTextureFormat : uint32_t;

	enum class GBuffer : int32_t
	{
		BaseColor = 0,
		Metallic,
		Roughness,
		WorldPosition,
		WorldNormal,
		Emissive,
		MeshID,
		ClusterID,
		GBufferSlotCount
	};

	enum class AntiAliasing : int32_t
	{
		None = 0,
		FXAA
	};

	struct RenderingParams
	{
		RenderingParams();

		bool DrawWireframe = false;
		bool EnableDeferredRendering = true;
		AntiAliasing AntiAliasingMode = AntiAliasing::FXAA;
	};

	/** All Rendering is maintained here */
	class RenderPipeline : public EngineSubSystem
	{
		SUORA_CLASS(6453444876);
	public:
		static FramebufferTextureParams GBufferSlotToBufferParams(GBuffer slot);
	public:
		RenderPipeline();
		~RenderPipeline();
		virtual void Initialize();

		void Render(Framebuffer& buffer, World& world, CameraNode& camera, Framebuffer& gbuffer, RenderingParams& params);
		virtual void Render(Framebuffer& buffer, World& world, CameraNode& camera, RenderingParams& params)
		{
			Render(buffer, world, camera, *GetGBuffer(), params);
		}
		inline static Ref<VertexArray> __GetFullscreenQuad() { return GetFullscreenQuad(); }
		inline Ref<Shader> GetFullscreenPassShader() const { return m_FullscreenPassShader; }
		static Ref<Shader> GetFullscreenPassShaderStatic();

		static void RenderFramebufferIntoFramebuffer(Framebuffer& src, Framebuffer& target, Shader& shader, const glm::ivec4& rect, const std::string& uniformName = "u_Texture", int attachmentIndex = 0, bool shouldClear = false);
		static void AddFramebufferToFramebuffer(Framebuffer& src, Framebuffer& target, int attachmentIndex = 0);
		static void BlitDepthBuffer(Framebuffer& src, Framebuffer& target, Shader& shader, const std::string& uniformName = "u_Texture");
		static void SetFullscreenViewport(Framebuffer& buffer);
		static Ref<Framebuffer> GetGBuffer() { return s_Instance->m_GBuffer; }

		static void ClearDepth(uint32_t x, uint32_t y, uint32_t width, uint32_t height);

	protected:
		void ShadowPass(World& world, CameraNode& camera);

		void DeferredPass(World& world, CameraNode& camera, Framebuffer& gbuffer, RenderingParams& params);
		void RenderGBuffer(World& world, CameraNode& camera, Framebuffer& gbuffer, RenderingParams& params);
		void DecalPass(World& world, CameraNode& camera, Framebuffer& gbuffer);
		void DeferredSkyPass(World& world, CameraNode& camera, Framebuffer& gbuffer);
		void DeferredLightPass(Ref<Framebuffer> target, Framebuffer& gBuffer, World& world, CameraNode* camera, bool lowQuality = false, int quadTick = 0, bool volumetric = false);
		void DeferredCompositePass(World& world, CameraNode& camera, Framebuffer& gbuffer);

		void ForwardPass(World& world, CameraNode& camera, Framebuffer& gbuffer, RenderingParams& params);

		void PostProcessPass(World& world, CameraNode& camera, Framebuffer& gbuffer, RenderingParams& params);

		void UserInterfacePass(World& world, const glm::mat4& view, Framebuffer& target, RenderingParams& params);

	private:
		static Ref<VertexArray> GetFullscreenQuad();

		Ref<Shader> m_FullscreenPassShader;
		Ref<Shader> m_AddShader;
		Ref<Shader> m_DepthBlitShader;
		Ref<Shader> m_ToneMapping;
		Ref<Shader> m_FXAA;
		Ref<Framebuffer> m_TemporaryAddBuffer;

		// Deferred
		Ref<Framebuffer> m_GBuffer;
		//Ref<Framebuffer> m_DeferredTempLitBuffer;
		std::unordered_map<glm::ivec2, Ref<Framebuffer>, IVec2Hasher> m_DeferredLitBuffer;
		Ref<Framebuffer> GetDeferredLitBuffer(const glm::ivec2& size);
		std::unordered_map<glm::ivec2, Ref<Framebuffer>, IVec2Hasher> m_DeferredDecalBuffer;
		Ref<Framebuffer> GetDeferredDecalBuffer(const glm::ivec2& size);
		Ref<Shader> m_DeferredDecalPreparation;
		Ref<Shader> m_DeferredDirectionalLightShader;
		Ref<Shader> m_DeferredPointLightShader;
		Ref<ShaderStorageBuffer> m_DeferredPointLightMatrixBuffer;
		Ref<Shader> m_DeferredSkyShader;
		Ref<Shader> m_DeferredSkyLightShader;
		Ref<Shader> m_DeferredComposite;

		// Forward
		std::unordered_map<glm::ivec2, Ref<Framebuffer>, IVec2Hasher> m_ForwardReadyBuffer;
		Ref<Framebuffer> GetForwardReadyBuffer(const glm::ivec2& size);

		std::unordered_map<glm::ivec2, Ref<Framebuffer>, IVec2Hasher> m_PostProcessTempBuffer;
		Ref<Framebuffer> GetPostProcessTempBuffer(const glm::ivec2& size);

		Ref<Decima> m_DecimaInstance;
		std::thread m_DecimaThread;

		std::unordered_map<glm::ivec2, Ref<Framebuffer>, IVec2Hasher> m_FinalFramebuffer;
		Ref<Framebuffer> GetFinalFramebuffer(const glm::ivec2& size);

		inline static Ptr<RenderPipeline> s_Instance;

		inline static Vec3 s_LastCameraPos = Vec3();


	public:
		static glm::ivec2 GetInternalResolution();
		static void SetInternalResolution(const glm::ivec2& resolution);

	protected:
		glm::ivec2 m_InternalResolution = glm::vec2(1920, 1080) * 1.0f;

		friend class Ilum;
		friend class Decima;
		friend class ViewportPanel;
	};
}