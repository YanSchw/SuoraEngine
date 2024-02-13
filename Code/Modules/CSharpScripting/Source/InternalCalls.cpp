#include "Precompiled.h"
#include "InternalCalls.h"

#include "Suora/GameFramework/Node.h"

namespace Suora
{

	void Node_SetUpdateFlag(void* obj, int flag)
	{
		((Node*)obj)->SetUpdateFlag((UpdateFlag)flag);
	}

	void Node3D_SetPosition(void* obj, float x, float y, float z)
	{
		((Node3D*)obj)->SetPosition(Vec3(x, y, z));
	}

	void Node3D_AddWorldOffset(void* obj, float x, float y, float z)
	{
		((Node3D*)obj)->AddWorldOffset(Vec3(x, y, z));
	}

	void Node3D_SetRotation(void* obj, float x, float y, float z)
	{
		((Node3D*)obj)->SetRotation(Vec3(x, y, z));
	}

	void Node3D_RotateEuler(void* obj, float x, float y, float z)
	{
		((Node3D*)obj)->RotateEuler(Vec3(x, y, z));
	}

	void Node3D_SetScale(void* obj, float x, float y, float z)
	{
		((Node3D*)obj)->SetScale(Vec3(x, y, z));
	}

}