#pragma once
#include "Suora/Editor/Panels/MajorTab.h"
#include "ExportProjectPanel.generated.h"

namespace Suora
{

	class ExportSettings : public Object
	{
		SUORA_CLASS(87594334897);
	public:
		ExportSettings();

		std::filesystem::path m_OutputPath;

		// Platform Windows
		std::filesystem::path m_MSBuildPath;
	};

	class ExportProjectPanel : public MajorTab
	{
		SUORA_CLASS(589743678432);
	public:
		Ref<ExportSettings> m_ExportSettings = nullptr;

		ExportProjectPanel();

		void Init() override;
		virtual void Update(float deltaTime) override;
		virtual void DrawToolbar(float& x, float y, float height) override;
		virtual Texture* GetIconTexture() override;
		void SaveAsset() override;
	
	};

}