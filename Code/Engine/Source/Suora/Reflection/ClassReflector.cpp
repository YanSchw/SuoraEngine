#include "Precompiled.h"
#include "ClassReflector.h"
#include "Suora/Core/Object/Object.h"
#include "New.h"
#include "Suora/Assets/Blueprint.h"
#include "Suora/NodeScript/ScriptTypes.h"
#include "Suora/Common/Common.h"
#include "Suora/Common/VectorUtils.h"

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
	ClassMember::Type ClassMember::GetPrimitiveTypeEnumByTemplate<Object*>()
	{
		return ClassMember::Type::ObjectPtr;
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

	void ClassReflector::Create(const Class& cls, const std::function<void(ClassReflector&)>& reflLambda)
	{
		Ref<ClassReflector> refl = CreateRef<ClassReflector>();

		reflLambda(*refl);

		s_Reflectors[cls.GetNativeClassID()] = refl;
	}

	const ClassReflector& ClassReflector::GetByClass(const Class& cls)
	{
		if (!cls.IsNative()) return GetByClass(cls.GetParentClass());

		if (!s_Reflectors.ContainsKey(cls.GetNativeClassID()))
		{
			// This should not happen
			SuoraVerify(false);

			ClassReflector* refl = new ClassReflector();
			return *refl;
		}
		else
		{
			return *s_Reflectors[cls.GetNativeClassID()];
		}
	}

	String ClassReflector::GetClassName(const Class& cls)
	{
		return ClassReflector::GetByClass(cls).m_ClassName;
	}

	void ClassMember_Delegate::FeedSignature(const String& args)
	{
		String sub = args.substr(9, args.size() - 10);
		std::vector<String> arg = StringUtil::SplitString(sub, ',');
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