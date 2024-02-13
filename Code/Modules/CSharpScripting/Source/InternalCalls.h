#pragma once

#include "CoreMinimal.h"

namespace Suora
{

	void Node_SetUpdateFlag(void* obj, int flag);

	void Node3D_SetPosition(void* obj, float x, float y, float z);
	void Node3D_AddWorldOffset(void* obj, float x, float y, float z);
	void Node3D_SetRotation(void* obj, float x, float y, float z);
	void Node3D_RotateEuler(void* obj, float x, float y, float z);
	void Node3D_SetScale(void* obj, float x, float y, float z);

}