#pragma once

#include <Jolt/Jolt.h>
#include "CoreMinimal.h"

namespace Suora::Physics
{

	struct Convert
	{
		static JPH::RVec3 ToRVec3(const Suora::Vec3& vec)
		{
			return JPH::RVec3(vec.x, vec.y, vec.z);
		}
		static Suora::Vec3 ToVec3(const JPH::RVec3& vec)
		{
			return Suora::Vec3(vec.GetX(), vec.GetY(), vec.GetZ());
		}
		static JPH::Quat ToJoltQuat(const Suora::Quat& quat)
		{
			return {quat.x, quat.y, quat.z, quat.w};
		}
		static Suora::Quat ToSuoraQuat(const JPH::Quat& quat)
		{
			return { quat.GetW(), quat.GetX(), quat.GetY(), quat.GetZ() };
		}
	};

}