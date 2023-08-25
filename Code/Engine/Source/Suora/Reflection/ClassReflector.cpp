#include "Precompiled.h"
#include "ClassReflector.h"
#include "Suora/Core/Object/Object.h"
#include "New.h"
#include "Suora/Assets/Blueprint.h"
#include "Suora/NodeScript/ScriptTypes.h"
#include "Suora/Common/Common.h"

namespace Suora
{
	template<>
	ClassMember::Type ClassMember::GetPrimitiveTypeEnumByTemplate<int>()
	{
		return ClassMember::Type::Integer32;
	}
	template<>
	ClassMember::Type ClassMember::GetPrimitiveTypeEnumByTemplate<float>()
	{
		return ClassMember::Type::Float;
	}
	template<>
	ClassMember::Type ClassMember::GetPrimitiveTypeEnumByTemplate<bool>()
	{
		return ClassMember::Type::Bool;
	}
	template<>
	ClassMember::Type ClassMember::GetPrimitiveTypeEnumByTemplate<Vec3>()
	{
		return ClassMember::Type::Vector3;
	}
	template<>
	ClassMember::Type ClassMember::GetPrimitiveTypeEnumByTemplate<Vec4>()
	{
		return ClassMember::Type::Vector4;
	}
	template<>
	ClassMember::Type ClassMember::GetPrimitiveTypeEnumByTemplate<Class>()
	{
		return ClassMember::Type::Class;
	}
	template<>
	ClassMember::Type ClassMember::GetPrimitiveTypeEnumByTemplate<Asset*>()
	{
		return ClassMember::Type::AssetPtr;
	}

//====================================================================================================//

	Array<Ref<ClassMember>> ClassReflector::GetAllClassMember() const
	{
		Array<Ref<ClassMember>> member;

		if (m_NativeParentClass != Class::None)
		{
			Array<Ref<ClassMember>> temp = GetByClass(m_NativeParentClass).GetAllClassMember();
			member += temp;
		}
		Array<Ref<ClassMember>> temp = Array<Ref<ClassMember>>(m_ClassMembers);
		member += temp;

		return member;
	}

	const ClassReflector& ClassReflector::GetByClass(const Class& cls)
	{
		if (!cls.IsNative()) return GetByClass(cls.GetParentClass());

		if (m_Reflectors.find(cls.GetNativeClassID()) == m_Reflectors.end())
		{
			ClassReflector* refl = new ClassReflector();

			cls.GetClassDefaultObject()->ReflClass(*refl);
			m_Reflectors[cls.GetNativeClassID()] = refl;

			return *refl;
		}
		else
		{
			return *m_Reflectors[cls.GetNativeClassID()];
		}
	}

	std::string ClassReflector::GetClassName(const Class& cls)
	{
		return ClassReflector::GetByClass(cls).m_ClassName;
	}

	void ClassMember_Delegate::FeedSignature(const std::string& args)
	{
		std::string sub = args.substr(9, args.size() - 10);
		std::vector<std::string> arg = Util::SplitString(sub, ',');
		for (auto& It : arg)
		{
			if (It != "TDelegate::NoParams")
			{
				m_StrArgs.Add(It);
				m_Args.Add(StringToScriptDataType(It));
			}
		}
	}

}