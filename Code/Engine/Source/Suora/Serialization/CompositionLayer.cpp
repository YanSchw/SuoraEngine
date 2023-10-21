#include "Precompiled.h"
#include "CompositionLayer.h"
#include "Suora/GameFramework/Node.h"
#include "Suora/Assets/Asset.h"
#include "Suora/Assets/AssetManager.h"
#include "Suora/Common/Common.h"

namespace Suora
{

	IObjectCompositionData::IObjectCompositionData()
	{
	}

	void IObjectCompositionData::OnImplementation()
	{
		Object* obj = GetRootObject();
		const ClassReflector& refl = ClassReflector::GetByClass(obj->GetClass());
		m_Class = obj->GetClass();

		for (Ref<ClassMember> it : refl.GetAllClassMember())
		{
			DefaultMemberValue Value = DefaultMemberValue(*(it.get()));

			switch (it->m_Type)
			{
			case ClassMember::Type::Integer32: Value.m_ValueInt = *ClassMember::AccessMember<int>(obj, it->m_MemberOffset);
				break;
			case ClassMember::Type::Float: Value.m_ValueFloat = *ClassMember::AccessMember<float>(obj, it->m_MemberOffset);
				break;
			case ClassMember::Type::Bool: Value.m_ValueBool = *ClassMember::AccessMember<bool>(obj, it->m_MemberOffset);
				break;
			case ClassMember::Type::Vector3: Value.m_ValueVec3 = *ClassMember::AccessMember<Vec3>(obj, it->m_MemberOffset);
				break;
			case ClassMember::Type::Vector4: Value.m_ValueVec4 = *ClassMember::AccessMember<Vec4>(obj, it->m_MemberOffset);
				break;
			case ClassMember::Type::AssetPtr: Value.m_ValueAssetPtr = *ClassMember::AccessMember<Asset*>(obj, it->m_MemberOffset);
				break;
			case ClassMember::Type::MaterialSlots: Value.m_ValueMaterialSlots = *ClassMember::AccessMember<MaterialSlots>(obj, it->m_MemberOffset);
				break;
			case ClassMember::Type::Class: Value.m_ValueClass = *ClassMember::AccessMember<Class>(obj, it->m_MemberOffset);
				break;
			case ClassMember::Type::SubclassOf: Value.m_ValueClass = ClassMember::AccessMember<TSubclassOf>(obj, it->m_MemberOffset)->GetClass();
				break;
			case ClassMember::Type::Delegate: /* Nothing */
				break;
			default: SuoraError("{0}, ReflectionType missing!", __FUNCTION__); break;
			}

			m_DefaultMemberValues.Add(Value);
		}

	}
	void IObjectCompositionData::AddMissingMembers()
	{
		Object* obj = GetRootObject();
		const ClassReflector& refl = ClassReflector::GetByClass(obj->GetClass());

		for (Ref<ClassMember> it : refl.GetAllClassMember())
		{
			DefaultMemberValue Value = DefaultMemberValue(*(it.get()));

			for (DefaultMemberValue& mem : m_DefaultMemberValues)
			{
				if (Value.m_Member.m_MemberName == mem.m_Member.m_MemberName && Value.m_Member.m_Type == mem.m_Member.m_Type)
				{
					goto skipAdd;
				}
			}

			m_DefaultMemberValues.Add(Value);
		skipAdd:;
		}

		// Sort Members by Offset
		bool bDone = false;
		while (!bDone)
		{
			bDone = true;
			for (int i = 0; i < m_DefaultMemberValues.Size() - 1; i++)
			{
				if (m_DefaultMemberValues[i].m_Member.m_MemberOffset > m_DefaultMemberValues[i + 1].m_Member.m_MemberOffset)
				{
					bDone = false;
					DefaultMemberValue temp = m_DefaultMemberValues[i];
					m_DefaultMemberValues[i] = m_DefaultMemberValues[i + 1];
					m_DefaultMemberValues[i + 1] = temp;
				}
			}
		}
	}

	void IObjectCompositionData::Serialize(Yaml::Node& root)
	{
		root = Yaml::Node();
		root["Class"] = m_Class.ToString();
		root["m_IsActorLayer"] = m_IsActorLayer ? "true" : "false";

		int i = 0;
		for (auto& value : m_DefaultMemberValues)
		{
			Yaml::Node& member = root["Member_" + std::to_string(i)];
			member["ClassMember"]["m_MemberName"] = value.m_Member.m_MemberName;
			member["m_ValueChanged"] = value.m_ValueChanged ? "true" : "false";
			member["ClassMember"]["m_MemberOffset"] = std::to_string(value.m_Member.m_MemberOffset);
			member["ClassMember"]["m_TypeSize"] = std::to_string(value.m_Member.m_TypeSize);
			member["ClassMember"]["m_Type"] = std::to_string((int)value.m_Member.m_Type);

			/*member["Value"]["Int"] = std::to_string(value.m_ValueInt);
			member["Value"]["Float"] = std::to_string(value.m_ValueFloat);
			member["Value"]["Bool"] = value.m_ValueBool ? "true" : "false";
			member["Value"]["AssetPtr"] = value.m_ValueAssetPtr ? value.m_ValueAssetPtr->m_UUID.GetString() : "0";*/

			Asset* asset = nullptr;
			if (value.m_Member.m_Type == ClassMember::Type::AssetPtr)
				asset = *ClassMember::AccessMember<Asset*>(GetRootObject(), value.m_Member.m_MemberOffset);

			switch (value.m_Member.m_Type)
			{
			case ClassMember::Type::Integer32: 
				member["Value"]["Int"] = std::to_string(value.m_ValueInt);//std::to_string(*ClassMember::AccessMember<int>(GetRootObject(), value.m_Member.m_MemberOffset));
				break;
			case ClassMember::Type::Float:
				member["Value"]["Float"] = std::to_string(value.m_ValueFloat);//std::to_string(*ClassMember::AccessMember<float>(GetRootObject(), value.m_Member.m_MemberOffset));
				break;
			case ClassMember::Type::Bool:
				member["Value"]["Bool"] = value.m_ValueBool ? "true" : "false";// (*ClassMember::AccessMember<bool>(GetRootObject(), value.m_Member.m_MemberOffset)) ? "true" : "false";
				break;
			case ClassMember::Type::Vector3:
				member["Value"]["Vec3"] = Vec::ToString<Vec3>(value.m_ValueVec3); //Vec::ToString<Vec3>((*ClassMember::AccessMember<Vec3>(GetRootObject(), value.m_Member.m_MemberOffset)));
				break;
			case ClassMember::Type::Vector4:
				member["Value"]["Vec4"] = Vec::ToString<Vec4>(value.m_ValueVec4); //Vec::ToString<Vec4>((*ClassMember::AccessMember<Vec4>(GetRootObject(), value.m_Member.m_MemberOffset)));
				break;
			case ClassMember::Type::AssetPtr:
				member["Value"]["AssetPtr"] = (value.m_ValueAssetPtr) ? value.m_ValueAssetPtr->m_UUID.GetString() : "0"; // (asset) ? asset->m_UUID.GetString() : "0";
				break;
			case ClassMember::Type::MaterialSlots:
			{
				MaterialSlots* slots = &value.m_ValueMaterialSlots;// (ClassMember::AccessMember<MaterialSlots>(GetRootObject(), value.m_Member.m_MemberOffset));
				member["Value"]["MaterialSlots"]["Overwrite"] = slots->OverwritteMaterials ? "true" : "false";
				if (slots->OverwritteMaterials) for (int i = 0; i < slots->Materials.Size(); i++) member["Value"]["MaterialSlots"][std::to_string(i)] = (slots->Materials[i]) ? slots->Materials[i]->m_UUID.GetString() : "0";
			} break;
			case ClassMember::Type::Class:
				member["Value"]["Class"] = value.m_ValueClass.ToString();// ClassMember::AccessMember<Class>(GetRootObject(), value.m_Member.m_MemberOffset)->ToString();
				break;
			case ClassMember::Type::SubclassOf:
				member["Value"]["SubclassOf"] = value.m_ValueClass.ToString();// ClassMember::AccessMember<TSubclassOf>(GetRootObject(), value.m_Member.m_MemberOffset)->GetClass().ToString();
				break;
			case ClassMember::Type::Delegate: /* Nothing */
				break;
			default: SuoraError("{0}, ReflectionType missing!", __FUNCTION__); break;
			}

			i++;
		}

	}

	void IObjectCompositionData::Deserialize(Yaml::Node& root)
	{
		m_Class = Class::FromString(root["Class"].As<std::string>());
		m_IsActorLayer = root["m_IsActorLayer"].As<std::string>() == "true";

		int i = 0;
		while (true)
		{
			Yaml::Node& member = root["Member_" + std::to_string(i++)];
			if (member.IsNone()) break;
			m_DefaultMemberValues.Add(DefaultMemberValue());
			auto& value = m_DefaultMemberValues[m_DefaultMemberValues.Size() - 1];

			value.m_Member.m_MemberName = member["ClassMember"]["m_MemberName"].As<std::string>();
			value.m_ValueChanged = member["m_ValueChanged"].As<std::string>() == "true";
			value.m_Member.m_MemberOffset = (size_t) std::stoi(member["ClassMember"]["m_MemberOffset"].As<std::string>());
			value.m_Member.m_TypeSize = (size_t) std::stoi(member["ClassMember"]["m_TypeSize"].As<std::string>());
			value.m_Member.m_Type = (ClassMember::Type) std::stoi(member["ClassMember"]["m_Type"].As<std::string>());


			switch (value.m_Member.m_Type)
			{
			case ClassMember::Type::Integer32:
				value.m_ValueInt = std::stoi(member["Value"]["Int"].As<std::string>());
				break;
			case ClassMember::Type::Float:
				value.m_ValueFloat = std::stof(member["Value"]["Float"].As<std::string>());
				break;
			case ClassMember::Type::Bool:
				value.m_ValueBool = member["Value"]["Bool"].As<std::string>() == "true";
				break;
			case ClassMember::Type::Vector3:
				value.m_ValueVec3 = Vec::FromString<Vec3>(member["Value"]["Vec3"].As<std::string>());
				break;
			case ClassMember::Type::Vector4:
				value.m_ValueVec4 = Vec::FromString<Vec4>(member["Value"]["Vec4"].As<std::string>());
				break;
			case ClassMember::Type::AssetPtr:
				value.m_ValueAssetPtr = (member["Value"]["AssetPtr"].As<std::string>() != "0")
					? AssetManager::GetAsset<Asset>(member["Value"]["AssetPtr"].As<std::string>()) 
					: nullptr;
				break;
			case ClassMember::Type::MaterialSlots:
			{
				value.m_ValueMaterialSlots.OverwritteMaterials = member["Value"]["MaterialSlots"]["Overwrite"].As<std::string>() == "true";
				value.m_ValueMaterialSlots.Materials.Clear();
				int i = 0;
				while (true)
				{
					Yaml::Node& material = member["Value"]["MaterialSlots"][std::to_string(i)];
					if (material.IsNone()) break;
					value.m_ValueMaterialSlots.Materials.Add((material.As<std::string>() != "0")
															? AssetManager::GetAsset<Material>(material.As<std::string>())
															: nullptr);
					i++;
				}
			} break;
			case ClassMember::Type::Class:
				value.m_ValueClass = Class::FromString(member["Value"]["Class"].As<std::string>());
				break;
			case ClassMember::Type::SubclassOf:
				value.m_ValueClass = Class::FromString(member["Value"]["SubclassOf"].As<std::string>());
				break;
			case ClassMember::Type::Delegate: /* Nothing */
				break;
			default: SuoraError("{0}, ReflectionType missing!", __FUNCTION__); break;
			}
		}
	}

	/** Apply the m_Value... to the m_Member on the Object* <obj> */
	void IObjectCompositionData::DefaultMemberValue::Apply(Object* obj)
	{
		const ClassReflector& refl = ClassReflector::GetByClass(obj->GetNativeClass());
		const auto allMembers = refl.GetAllClassMember();

		for (Ref<ClassMember> mem : allMembers)
		{
			if (m_Member.m_MemberName == mem->m_MemberName && m_Member.m_Type == mem->m_Type)
			{
				switch (m_Member.m_Type)
				{
				case ClassMember::Type::Integer32:
					*ClassMember::AccessMember<int>(obj, mem->m_MemberOffset) = m_ValueInt;
					break;
				case ClassMember::Type::Float:
					*ClassMember::AccessMember<float>(obj, mem->m_MemberOffset) = m_ValueFloat;
					break;
				case ClassMember::Type::Bool:
					*ClassMember::AccessMember<bool>(obj, mem->m_MemberOffset) = m_ValueBool;
					break;
				case ClassMember::Type::Vector3:
					*ClassMember::AccessMember<Vec3>(obj, mem->m_MemberOffset) = m_ValueVec3;
					break;
				case ClassMember::Type::Vector4:
					*ClassMember::AccessMember<Vec4>(obj, mem->m_MemberOffset) = m_ValueVec4;
					break;
				case ClassMember::Type::AssetPtr:
					*ClassMember::AccessMember<Asset*>(obj, mem->m_MemberOffset) = m_ValueAssetPtr;
					break;
				case ClassMember::Type::MaterialSlots:
					*ClassMember::AccessMember<MaterialSlots>(obj, mem->m_MemberOffset) = m_ValueMaterialSlots;
					break;
				case ClassMember::Type::Class:
					*ClassMember::AccessMember<Class>(obj, mem->m_MemberOffset) = m_ValueClass;
					break;
				case ClassMember::Type::SubclassOf:
					*ClassMember::AccessMember<TSubclassOf>(obj, mem->m_MemberOffset) = m_ValueClass;
					break;
				default: SuoraError("{0}, ReflectionType missing!", __FUNCTION__); break;
				}

				break;
			}
		}

	}
	/** Reset the Member of the Object* <obj> to the Value of the CompositeParent */
	void IObjectCompositionData::DefaultMemberValue::Reset(Object* obj)
	{
		SuoraVerify(obj->IsA<Node>());
		Node* DefaultRoot = obj->As<Node>()->GetCompositionNode();
		Node* DefaultComp = obj->As<Node>()->GetCompositeClass().GetClassDefaultObject()->As<Node>();
		// Traverse down hierarchy
		while (obj->As<Node>()->IsChildOf(DefaultRoot))
		{
			for (int i = 0; i < DefaultRoot->GetChildCount(); i++)
			{
				if (obj->As<Node>()->IsChildOf(DefaultRoot->GetChild(i)) || obj->As<Node>() == DefaultRoot->GetChild(i))
				{
					DefaultRoot = DefaultRoot->GetChild(i);
					DefaultComp = DefaultComp->GetChild(i);
					break;
				}
			}
		}

		const ClassReflector& refl = ClassReflector::GetByClass(obj->GetClass());
		const ClassReflector& reflParent = ClassReflector::GetByClass(DefaultComp->GetClass());
		const auto allMembers = refl.GetAllClassMember();
		const auto allParentMembers = reflParent.GetAllClassMember();
		bool parentHasMember = false;

		for (Ref<ClassMember> mem : allParentMembers)
		{
			if (m_Member.m_MemberName == mem->m_MemberName && m_Member.m_Type == mem->m_Type)
			{
				parentHasMember = true;
				break;
			}
		}

		if (true) // isMembernative
		{
			for (const Ref<ClassMember>& mem : allMembers)
			{
				if (m_Member.m_MemberName == mem->m_MemberName && m_Member.m_Type == mem->m_Type)
				{
					switch (m_Member.m_Type)
					{
					case ClassMember::Type::Integer32:
						*ClassMember::AccessMember<int>(obj, mem->m_MemberOffset) = parentHasMember ? *ClassMember::AccessMember<int>(DefaultComp, mem->m_MemberOffset) : 0;
						break;
					case ClassMember::Type::Float:
						*ClassMember::AccessMember<float>(obj, mem->m_MemberOffset) = parentHasMember ? *ClassMember::AccessMember<float>(DefaultComp, mem->m_MemberOffset) : 0.0f;
						break;
					case ClassMember::Type::Bool:
						*ClassMember::AccessMember<bool>(obj, mem->m_MemberOffset) = parentHasMember ? *ClassMember::AccessMember<bool>(DefaultComp, mem->m_MemberOffset) : false;
						break;
					case ClassMember::Type::Vector3:
						*ClassMember::AccessMember<Vec3>(obj, mem->m_MemberOffset) = parentHasMember ? *ClassMember::AccessMember<Vec3>(DefaultComp, mem->m_MemberOffset) : Vec3();
						break;
					case ClassMember::Type::Vector4:
						*ClassMember::AccessMember<Vec4>(obj, mem->m_MemberOffset) = parentHasMember ? *ClassMember::AccessMember<Vec4>(DefaultComp, mem->m_MemberOffset) : Vec4();
						break;
					case ClassMember::Type::AssetPtr:
						*ClassMember::AccessMember<Asset*>(obj, mem->m_MemberOffset) = parentHasMember ? *ClassMember::AccessMember<Asset*>(DefaultComp, mem->m_MemberOffset) : nullptr;
						break;
					case ClassMember::Type::MaterialSlots:
						*ClassMember::AccessMember<MaterialSlots>(obj, mem->m_MemberOffset) = parentHasMember ? *ClassMember::AccessMember<MaterialSlots>(DefaultComp, mem->m_MemberOffset) : MaterialSlots();
						break;
					case ClassMember::Type::Class:
						*ClassMember::AccessMember<Class>(obj, mem->m_MemberOffset) = parentHasMember ? *ClassMember::AccessMember<Class>(DefaultComp, mem->m_MemberOffset) : Class::None;
						break;
					case ClassMember::Type::SubclassOf:
						*ClassMember::AccessMember<TSubclassOf>(obj, mem->m_MemberOffset) = parentHasMember ? ClassMember::AccessMember<TSubclassOf>(DefaultComp, mem->m_MemberOffset)->GetClass() : ClassMember::AccessMember<TSubclassOf>(obj, mem->m_MemberOffset)->GetBase();
						break;
					default: SuoraError("{0}, ReflectionType missing!", __FUNCTION__); break;
					}

					break;
				}
			}
		}

		m_ValueChanged = false;
	}

	/*template<>
	int IObjectCompositionData::DefaultMemberValue::GetDefaultValueOfType()
	{
		return 0;
	}
	template<>
	float IObjectCompositionData::DefaultMemberValue::GetDefaultValueOfType()
	{
		return 0.0f;
	}
	template<>
	bool IObjectCompositionData::DefaultMemberValue::GetDefaultValueOfType()
	{
		return false;
	}
	template<>
	Vec3 IObjectCompositionData::DefaultMemberValue::GetDefaultValueOfType()
	{
		return Vec3();
	}
	template<>
	Vec4 IObjectCompositionData::DefaultMemberValue::GetDefaultValueOfType()
	{
		return Vec4();
	}
	template<>
	Asset* IObjectCompositionData::DefaultMemberValue::GetDefaultValueOfType()
	{
		return nullptr;
	}
	template<>
	MaterialSlots IObjectCompositionData::DefaultMemberValue::GetDefaultValueOfType()
	{
		return MaterialSlots();
	}
	template<>
	Class IObjectCompositionData::DefaultMemberValue::GetDefaultValueOfType()
	{
		return Class::None;
	}
	template<>
	TSubclassOf IObjectCompositionData::DefaultMemberValue::GetDefaultValueOfType()
	{
		return TSubclassOf();
	}*/

	void CompositionLayer::SerializeCompositionLayer(Suora::CompositionLayer& layer, Yaml::Node& yaml)
	{
		// This Layer
		layer.m_RootCompositionData.Serialize(yaml["m_RootCompositionData"]);
		yaml["m_Name"] = layer.m_Name;
		// Transform
		yaml["m_Transform"]["Pos"] = Vec::ToString(layer.Transform_Pos);
		yaml["m_Transform"]["Rot"] = Vec::ToString(layer.Transform_Rot);
		yaml["m_Transform"]["Scale"] = Vec::ToString(layer.Transform_Scale);
		//yaml["m_Transform"] = std::string(Vec::ToString(layer.Transform_Pos) + "%" + Vec::ToString(layer.Transform_Rot) + "%" + Vec::ToString(layer.Transform_Scale));

		// SubLayers
		for (int i = 0; i < layer.m_SubCompositionLayers.Size(); i++)
		{
			SerializeCompositionLayer(layer.m_SubCompositionLayers[i], yaml[std::to_string(i)]);
			std::string indentation;
			for (int32_t j = 0; j < layer.m_SubCompositionIndentation[i].Size(); j++)
			{
				indentation += "/" + std::to_string(layer.m_SubCompositionIndentation[i][j]);
			}
			yaml[std::to_string(i)]["Indentation"] = indentation;
		}
	}

	void CompositionLayer::DeserializeCompositionLayer(Suora::CompositionLayer& layer, Yaml::Node& yaml)
	{
		// This Layer
		{
			layer.m_RootCompositionData.Deserialize(yaml["m_RootCompositionData"]);
			layer.m_Name = yaml["m_Name"].As<std::string>();
			//std::vector<std::string> tranform = Suora::Util::SplitString(yaml["m_Transform"].As<std::string>(), '%');
			layer.Transform_Pos = Vec::FromString<Vec3>(yaml["m_Transform"]["Pos"].As<std::string>().size() != 0 ? yaml["m_Transform"]["Pos"].As<std::string>() : "");
			layer.Transform_Rot = Vec::FromString<Vec3>(yaml["m_Transform"]["Rot"].As<std::string>().size() != 0 ? yaml["m_Transform"]["Rot"].As<std::string>() : "");
			layer.Transform_Scale = Vec::FromString<Vec3>(yaml["m_Transform"]["Scale"].As<std::string>().size() != 0 ? yaml["m_Transform"]["Scale"].As<std::string>() : "1.0/1.0/1.0");
		}
		// SubLayers
		{
			int i = 0;
			while (true)
			{
				Yaml::Node& sub = yaml[std::to_string(i++)];
				if (sub.IsNone()) break;
				layer.m_SubCompositionLayers.Add(Suora::CompositionLayer());
				layer.m_SubCompositionIndentation.Add(Array<int32_t>());
				DeserializeCompositionLayer(layer.m_SubCompositionLayers[layer.m_SubCompositionLayers.Size() - 1], sub);
				std::string indentation = sub["Indentation"].As<std::string>();
				std::vector<std::string> split = Suora::Util::SplitString(indentation, '/');
				for (int32_t j = 1; j < split.size(); j++)
				{
					layer.m_SubCompositionIndentation[layer.m_SubCompositionIndentation.Last()].Add(std::stoi(split[j]));
				}
			}
		}
	}

}