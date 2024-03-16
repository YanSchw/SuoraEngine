#pragma once
#include "Suora/Editor/Panels/MajorTab.h"
#include "CSharpEditorUtility.generated.h"

namespace Suora
{

	class CSharpGenerateSolutionMenuItem : public MajorMenuItem
	{
		SUORA_CLASS(768534832);
	public:
		virtual String GetLabel() const { return "Generate C# Solution"; }
		virtual bool IsFileItem() const { return true; }

		CSharpGenerateSolutionMenuItem();
	};

}