#pragma once
#include "Suora/Core/Log.h"
#include "Suora/Core/Object/Object.h"
#include "Suora/Common/VectorUtils.h"
#include "NodeScriptLibrary.generated.h"

namespace Suora
{
	class Node;

	class NodeScriptLibrary : public Object
	{
		SUORA_CLASS(246832423);
	public:

		FUNCTION(Callable, Pure)
		static Vec3 MakeVector3(float x, float y, float z)
		{
			return Vec3(x, y, z);
		}

		FUNCTION(Callable, Pure)
		static float Sin(float f);

		FUNCTION(Callable, Pure)
		static float Cos(float f);

		FUNCTION(Callable)
		static void CloseGame();

		FUNCTION(Callable)
		static void PrintNodeName(Node* node);
		FUNCTION(Callable)
		static void PrintHello();
			
	};
}