#include "Precompiled.h"
#include "NodeScriptLibrary.h"

#include "Suora/Core/Application.h"
#include "Suora/GameFramework/Node.h"

namespace Suora
{

	float NodeScriptLibrary::Sin(float f)
	{
		return glm::sin(f);
	}

	float NodeScriptLibrary::Cos(float f)
	{
		return glm::cos(f);
	}

	void NodeScriptLibrary::CloseGame()
	{
		Application::Get().Close();
	}

	void NodeScriptLibrary::PrintNodeName(Node* node)
	{
		SuoraLog(node->GetName());
	}

	void NodeScriptLibrary::PrintHello()
	{
		SuoraLog("Hello!");
	}

}