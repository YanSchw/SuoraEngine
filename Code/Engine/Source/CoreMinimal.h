#pragma once

// This is a small Header that gives you easy access to the Engine API and GameFramework
#include "Suora/Core/Base.h"
#include "Suora/Core/Log.h"
#include "Suora/Core/Assert.h"

#include "Suora/Reflection/Class.h"
#include "Suora/Reflection/SubclassOf.h"
#include "Suora/Reflection/New.h"

#include "Suora/Common/VectorUtils.h"
#include "Suora/Common/Delegate.h"
#include "Suora/Common/Math.h"
#include "Suora/Common/Array.h"

#include "Suora/Core/Object/Object.h"
#include "Suora/Core/Object/Pointer.h"

namespace Suora
{
	class Application;
	class Engine;
	class GameInstance;
	class World;
	class Node;
	class Node3D;
	class UINode;
	class Component;
	class LevelNode;

	class Asset;
	class AssetManager;
	class Blueprint;
}
using namespace Suora;