#pragma once

namespace Suora
{
	class World;

	enum class UpdateFlag : uint32_t
	{
		NeverUpdate = 0,
		WorldUpdate = 1,
		LocalUpdate = 2
	};
	inline UpdateFlag operator|(UpdateFlag a, UpdateFlag b)
	{
		return static_cast<UpdateFlag>(static_cast<int32_t>(a) | static_cast<int32_t>(b));
	}
	
	inline UpdateFlag& operator|=(UpdateFlag& a, UpdateFlag b)
	{ 
		return (UpdateFlag&)((uint32_t&)a |= (uint32_t)b);
	}

#define LOCAL_UPDATE_LOCK() if (::Suora::UpdateRules::LocalUpdate()) \
								{ \
									SuoraError("LocalUpdateRule Violation in {0} // {1}() // {2}", __FILE__, __FUNCTION__, __LINE__); \
									SUORA_ASSERT(false); \
								} 


	struct UpdateRules
	{
		static bool LocalUpdate()
		{
			return s_LocalUpdate;
		}
	private:
		inline static bool s_LocalUpdate = false;
		friend class World;
	};

}