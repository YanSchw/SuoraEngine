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

	/** RenderingParams also known as RenderingCache stores persistent RenderingInformation to be utilized by RenderPipeline */
	struct RenderingParams
	{
		RenderingParams();

		Ref<Framebuffer> GetGBuffer() const { return m_GBuffer; }
		Ref<Framebuffer> GetDeferredLitBuffer() const { return m_DeferredLitBuffer; }
		Ref<Framebuffer> GetDeferredDecalBuffer() const { return m_DeferredDecalBuffer; }
		Ref<Framebuffer> GetForwardReadyBuffer() const { return m_ForwardReadyBuffer; }
		Ref<Framebuffer> GetPostProcessTempBuffer() const { return m_PostProcessTempBuffer; }
		Ref<Framebuffer> GetFinalBuffer() const { return m_FinalBuffer; }

		bool DrawWireframe = false;
		bool EnableDeferredRendering = true;
		AntiAliasing AntiAliasingMode = AntiAliasing::FXAA;
		iVec2 Resolution = iVec2(1920, 1080);

	private:
		iVec2 LastResolution = Resolution;
		bool m_InitializedBuffers = false;
		Ref<Framebuffer> m_GBuffer;
		Ref<Framebuffer> m_DeferredLitBuffer;
		Ref<Framebuffer> m_DeferredDecalBuffer;
		Ref<Framebuffer> m_ForwardReadyBuffer;
		Ref<Framebuffer> m_PostProcessTempBuffer;
		Ref<Framebuffer> m_FinalBuffer;

		void ValidateBuffers();

		friend class RenderPipeline;
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

		void Render(Framebuffer& buffer, World& world, CameraNode& camera, RenderingParams& params);
		
		inline static Ref<VertexArray> __GetFullscreenQuad() { return GetFullscreenQuad(); }
		inline Ref<Shader> GetFullscreenPassShader() const { return m_FullscreenPassShader; }
		static Ref<Shader> GetFullscreenPassShaderStatic();

		static void RenderFramebufferIntoFramebuffer(Framebuffer& src, Framebuffer& target, Shader& shader, const glm::ivec4& rect, const String& uniformName = "u_Texture", int attachmentIndex = 0, bool shouldClear = false);
		static void BlitDepthBuffer(Framebuffer& src, Framebuffer& target, Shader& shader, const String& uniformName = "u_Texture");
		static void SetFullscreenViewport(Framebuffer& buffer);

		static void ClearDepth(uint32_t x, uint32_t y, uint32_t width, uint32_t height);

	protected:
		void ShadowPass(World& world, CameraNode& camera);

		void DeferredPass(World& world, CameraNode& camera, RenderingParams& params);
		void RenderGBuffer(World& world, CameraNode& camera, RenderingParams& params);
		void DecalPass(World& world, CameraNode& camera, RenderingParams& params);
		void DeferredSkyPass(World& world, CameraNode& camera, RenderingParams& params);
		void DeferredLightPass(Ref<Framebuffer> target, RenderingParams& params, World& world, CameraNode* camera, bool lowQuality = false, int quadTick = 0, bool volumetric = false);
		void DeferredCompositePass(World& world, CameraNode& camera, RenderingParams& params);

		void ForwardPass(World& world, CameraNode& camera, RenderingParams& params);

		void PostProcessPass(World& world, CameraNode& camera, RenderingParams& params);

		void UserInterfacePass(World& world, const glm::mat4& view, Framebuffer& target, RenderingParams& params);

	private:
		static Ref<VertexArray> GetFullscreenQuad();

		Ref<Shader> m_FullscreenPassShader;
		Ref<Shader> m_DepthBlitShader;
		Ref<Shader> m_ToneMapping;
		Ref<Shader> m_FXAA;

		// Deferred
		Ref<Shader> m_DeferredDecalPreparation;
		Ref<Shader> m_DeferredDirectionalLightShader;
		Ref<Shader> m_DeferredPointLightShader;
		Ref<ShaderStorageBuffer> m_DeferredPointLightMatrixBuffer;
		Ref<Shader> m_DeferredSkyShader;
		Ref<Shader> m_DeferredSkyLightShader;
		Ref<Shader> m_DeferredComposite;

		// Forward

		Ref<Decima> m_DecimaInstance;
		std::thread m_DecimaThread;

		inline static Ptr<RenderPipeline> s_Instance;

		friend class Ilum;
		friend class Decima;
		friend class ViewportPanel;
	};
}