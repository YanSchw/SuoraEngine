#pragma once

#include "Suora/Renderer/RenderPipeline.h"

namespace Suora
{
	class Asset;
	class World;
	class Framebuffer;

	struct AssetPreview
	{
		Ref<Framebuffer> m_Preview;
		bool m_Init = false;
		bool m_Done = false;
		inline static RenderPipeline* s_Pipeline = nullptr;
		inline static RenderingParams s_RParams;
		Ref<World> m_World = nullptr;

		AssetPreview();
		~AssetPreview();

		void Init(Asset* asset);
		void Render(Asset* asset);


		inline static bool CanRerenderAssetPreviews = false;
		inline static std::unordered_map<Asset*, Ref<AssetPreview>> AssetPreviews;

		static void RerenderAssetPreviews();

		static void DrawAssetPreview(Asset* asset, const Class& assetClass, float x, float y, float width, float height);
		static void Tick(float deltaTime);
	};
}