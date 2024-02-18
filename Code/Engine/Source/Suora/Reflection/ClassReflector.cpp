#include "Precompiled.h"
#include "ClassReflector.h"
#include "Suora/Core/Object/Object.h"
#include "New.h"
#include "Suora/Assets/Blueprint.h"
#include "Suora/NodeScript/ScriptTypes.h"
#include "Suora/Common/Common.h"
#include "Suora/Common/VectorUtils.h"
#include "Property.h"

namespace Suora
{

	Array<Ref<ClassMemberProperty>> ClassReflector::GetAllClassMemberProperties() const
	{
		Array<Ref<ClassMemberProperty>> properties;

		if (m_NativeParentClass != Class::None)
		{
			Array<Ref<ClassMemberProperty>> temp = GetByClass(m_NativeParentClass).GetAllClassMemberProperties();
			properties += temp;
		}
		Array<Ref<ClassMemberProperty>> temp = Array<Ref<ClassMemberProperty>>(m_ClassProperties);
		properties += temp;

		return properties;
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

}