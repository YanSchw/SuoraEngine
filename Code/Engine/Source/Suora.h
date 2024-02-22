#pragma once

// For use by Suora applications

#include "Suora/Core/Base.h"

#include "Suora/Core/Application.h"
#include "Suora/Core/Log.h"
#include "Suora/Core/Assert.h"

#include "Suora/Platform/Platform.h"

#include "Suora/Core/NativeInput.h"
#include "Suora/Core/KeyCodes.h"
#include "Suora/Core/MouseCodes.h"

namespace glm { }

#include "Suora/Common/Common.h"
#include "Suora/Common/Delegate.h"
#include "Suora/Common/Math.h"
#include "Suora/Common/Array.h"
#include "Suora/Common/Random.h"
#include "Suora/Common/Filesystem.h"
#include "Suora/Common/SuoraID.h"

#include "Suora/Core/Object/Object.h"
#include "Suora/Core/Object/Interface.h"
#include "Suora/Core/Object/Pointer.h"
#include "Suora/Core/Object/ObjectFactory.h"

#include "Suora/Common/VectorUtils.h"
#include "Suora/Core/Engine.h"

#include "Suora/Assets/Asset.h"
#include "Suora/Assets/AssetManager.h"
#include "Suora/Assets/Mesh.h"
#include "Suora/Assets/Texture2D.h"
#include "Suora/Assets/Material.h"
#include "Suora/Assets/Level.h"
#include "Suora/Assets/Blueprint.h"

#include "Suora/GameFramework/Node.h"
#include "Suora/GameFramework/World.h"
#include "Suora/GameFramework/GameInstance.h"
#include "Suora/GameFramework/UserInterface.h"
#include "Suora/GameFramework/Nodes/OrganizationNodes.h"
#include "Suora/GameFramework/Nodes/ShapeNodes.h"
#include "Suora/GameFramework/Nodes/CharacterNode.h"
#include "Suora/GameFramework/Nodes/ParticleSystemNode.h"
#include "Suora/NodeScript/NodeScriptLibrary.h"

#include "Suora/Reflection/Class.h"
#include "Suora/Reflection/SubclassOf.h"
#include "Suora/Reflection/New.h"

#include "CoreMinimal.h"
namespace Suora {}
using namespace Suora;