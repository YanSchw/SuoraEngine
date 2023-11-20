#pragma once

#include "Suora/Common/Map.h"
#include "Suora/Renderer/RenderPipeline.h"

namespace Suora
{
	class Asset;
	class World;
	class Framebuffer;

	/* AssetPreviews to be displayed in the EditorUI */
	struct AssetPreview
	{
		/* Per AssetPreview data */
		Ref<Framebuffer> m_Preview;
		bool m_Init = false;
		bool m_Done = false;
		Ref<World> m_World = nullptr;

		AssetPreview();
		~AssetPreview();

		/* Initializes the m_World for rendering 3D AssetPreviews */
		void Init(Asset* asset);
		/* Renders the AssetPreview, returns true, if successful, false otherwise (e.g. because the Asset is not loaded yet) */
		bool Render(Asset* asset);

		/* Renders the next available AssetPreview */
		static void RenderAssetPreviews();

		/* Draws the AssetPreview in the EditorUI like specified. Certain Asset classes require a separate Rendering Tick,
		   and are therefore not immediately drawn! */
		static void DrawAssetPreview(Asset* asset, const Class& assetClass, float x, float y, float width, float height);
		static void Tick(float deltaTime);
	public:
		/* Map of all currently cached AssetPreviews */
		inline static Map<Asset*, Ref<AssetPreview>> s_AssetPreviews;

		/* For rendering 3D AssetPreviews */
		inline static RenderPipeline* s_Pipeline = nullptr;
		inline static RenderingParams s_RParams;
	};
}