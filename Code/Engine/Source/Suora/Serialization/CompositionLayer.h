#pragma once
#include "Suora/Core/Object/Interface.h"
#include "Suora/Serialization/Yaml.h"
#include "Suora/Common/VectorUtils.h"
#include "Suora/Assets/Material.h"
#include "CompositionLayer.generated.h"

namespace Suora
{
	struct MaterialSlots;

	class IObjectCompositionData : public Interface
	{
		SUORA_CLASS(67625);
	public:
		IObjectCompositionData();
		void OnImplementation() override;
		void AddMissingMembers();

		void Serialize(Yaml::Node& root);
		void Deserialize(Yaml::Node& root);

		void operator=(const IObjectCompositionData& other)
		{
			m_DefaultMemberValues = other.m_DefaultMemberValues;
			m_Class = other.m_Class;
			m_IsActorLayer = other.m_IsActorLayer;
		}

		struct DefaultMemberValue
		{
			DefaultMemberValue() = default;
			DefaultMemberValue(const ClassMember& member) : m_Member(member) {  }
			void Apply(Object* obj);
			void Reset(Object* obj);
			
			ClassMember m_Member;
			bool m_ValueChanged = false;

			int m_ValueInt = 0;
			float m_ValueFloat = 0.0f;
			bool m_ValueBool = false;
			Vec3 m_ValueVec3 = Vec3();
			Vec4 m_ValueVec4 = Vec4();
			Asset* m_ValueAssetPtr = nullptr;
			MaterialSlots m_ValueMaterialSlots;
			Class m_ValueClass = Class::None;
		};
		Array<DefaultMemberValue> m_DefaultMemberValues;

		Class m_Class = Class::None;
		bool m_IsActorLayer = false;
	};

	struct CompositionLayer
	{
		IObjectCompositionData m_RootCompositionData;

		// Node stuff
		std::string m_Name;
		Vec3 Transform_Pos = Vec3(0.0f), Transform_Rot = Vec3(0.0f), Transform_Scale = Vec3(1.0f);
		Array<CompositionLayer> m_SubCompositionLayers;
		Array<Array<int32_t>> m_SubCompositionIndentation;

		static void SerializeCompositionLayer(Suora::CompositionLayer& layer, Yaml::Node& yaml);
		static void DeserializeCompositionLayer(Suora::CompositionLayer& layer, Yaml::Node& yaml);
	};

}