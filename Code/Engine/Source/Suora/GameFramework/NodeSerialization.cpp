#include "Precompiled.h"
#include "Node.h"
#include "Suora/Common/Common.h"
#include "Suora/Serialization/Yaml.h"
#include "Suora/Assets/AssetManager.h"
#include "Suora/Assets/Material.h"

namespace Suora
{

	static void WritePropertyValue(Yaml::Node& property, const ClassMember& member, Node* node)
	{
		switch (member.m_Type)
		{
		case ClassMember::Type::Integer32:
			property["Value"]["Int32"] = std::to_string(*ClassMember::AccessMember<int32_t>(node, member.m_MemberOffset));
			break;
		case ClassMember::Type::Float:
			property["Value"]["Float"] = std::to_string(*ClassMember::AccessMember<float>(node, member.m_MemberOffset));
			break;
		case ClassMember::Type::Bool:
			property["Value"]["Bool"] = (*ClassMember::AccessMember<bool>(node, member.m_MemberOffset)) ? "true" : "false";
			break;
		case ClassMember::Type::Vector3:
			property["Value"]["Vec3"] = Vec::ToString<Vec3>((*ClassMember::AccessMember<Vec3>(node, member.m_MemberOffset)));
			break;
		case ClassMember::Type::Vector4:
			property["Value"]["Vec4"] = Vec::ToString<Vec4>((*ClassMember::AccessMember<Vec4>(node, member.m_MemberOffset)));
			break;
		case ClassMember::Type::AssetPtr:
			{
			const Asset* asset = *ClassMember::AccessMember<Asset*>(node, member.m_MemberOffset);
			property["Value"]["AssetPtr"] = (asset) ? asset->m_UUID.GetString() : "0";
			break;
			}
		case ClassMember::Type::MaterialSlots:
		{
			MaterialSlots* slots = (ClassMember::AccessMember<MaterialSlots>(node, member.m_MemberOffset));
			property["Value"]["MaterialSlots"]["Overwrite"] = slots->OverwritteMaterials ? "true" : "false";
			if (slots->OverwritteMaterials) for (int i = 0; i < slots->Materials.Size(); i++) property["Value"]["MaterialSlots"][std::to_string(i)] = (slots->Materials[i]) ? slots->Materials[i]->m_UUID.GetString() : "0";
		} break;
		case ClassMember::Type::Class:
			property["Value"]["Class"] = ClassMember::AccessMember<Class>(node, member.m_MemberOffset)->ToString();
			break;
		case ClassMember::Type::SubclassOf:
			property["Value"]["SubclassOf"] = ClassMember::AccessMember<TSubclassOf>(node, member.m_MemberOffset)->GetClass().ToString();
			break;
		case ClassMember::Type::Delegate: /* Nothing */
			break;
		default: SuoraError("{0}, ReflectionType missing!", __FUNCTION__); break;
		}
	}
	static void ReadPropertyValue(Yaml::Node& property, const ClassMember& member, Node* node)
	{
		switch (member.m_Type)
		{
		case ClassMember::Type::Integer32:
			*ClassMember::AccessMember<int32_t>(node, member.m_MemberOffset) = std::stoi(property["Value"]["Int32"].As<std::string>());
			break;
		case ClassMember::Type::Float:
			*ClassMember::AccessMember<float>(node, member.m_MemberOffset) = std::stof(property["Value"]["Float"].As<std::string>());
			break;
		case ClassMember::Type::Bool:
			*ClassMember::AccessMember<bool>(node, member.m_MemberOffset) = property["Value"]["Bool"].As<std::string>() == "true";
			break;
		case ClassMember::Type::Vector3:
			*ClassMember::AccessMember<Vec3>(node, member.m_MemberOffset) = Vec::FromString<Vec3>(property["Value"]["Vec3"].As<std::string>());
			break;
		case ClassMember::Type::Vector4:
			*ClassMember::AccessMember<Vec4>(node, member.m_MemberOffset) = Vec::FromString<Vec4>(property["Value"]["Vec4"].As<std::string>());
			break;
		case ClassMember::Type::AssetPtr:
			*ClassMember::AccessMember<Asset*>(node, member.m_MemberOffset) = (property["Value"]["AssetPtr"].As<std::string>() != "0")
						? AssetManager::GetAsset<Asset>(property["Value"]["AssetPtr"].As<std::string>())
						: nullptr;
			break;
		case ClassMember::Type::MaterialSlots:
		{
			MaterialSlots ValueMaterialSlots;
			ValueMaterialSlots.OverwritteMaterials = property["Value"]["MaterialSlots"]["Overwrite"].As<std::string>() == "true";
			ValueMaterialSlots.Materials.Clear();
			int i = 0;
			while (true)
			{
				Yaml::Node& material = property["Value"]["MaterialSlots"][std::to_string(i)];
				if (material.IsNone()) break;
				ValueMaterialSlots.Materials.Add((material.As<std::string>() != "0")
					? AssetManager::GetAsset<Material>(material.As<std::string>())
					: nullptr);
				i++;
			}
			*ClassMember::AccessMember<MaterialSlots>(node, member.m_MemberOffset) = ValueMaterialSlots;
		} break;
		case ClassMember::Type::Class:
			*ClassMember::AccessMember<Class>(node, member.m_MemberOffset) = Class::FromString(property["Value"]["Class"].As<std::string>());
			break;
		case ClassMember::Type::SubclassOf:
			*ClassMember::AccessMember<TSubclassOf>(node, member.m_MemberOffset) = Class::FromString(property["Value"]["SubclassOf"].As<std::string>());
			break;
		case ClassMember::Type::Delegate: /* Nothing */
			break;
		default: SuoraError("{0}, ReflectionType missing!", __FUNCTION__); break;
		}
	}

	struct NodeSerializer
	{
		int32_t m_ChildCount = 0;
		int32_t m_InheretedChildCount = 0;
		int32_t m_PropertyCount = 0;
		Node* m_RootNode = nullptr;
	};

	static void SerializeProperties(Yaml::Node& root, NodeSerializer& serializer, Node* node)
	{
		const ClassReflector& refl = ClassReflector::GetByClass(node->GetNativeClass());
		const auto allMembers = refl.GetAllClassMember();
		for (const Ref<ClassMember>& mem : allMembers)
		{
			if (node->m_OverwrittenProperties.Contains(mem->m_MemberName))
			{
				Yaml::Node& property = root["Properties"][std::to_string(serializer.m_PropertyCount++)];
				property["NodeName"] = node->GetName();
				property["PropertyName"] = mem->m_MemberName;
				WritePropertyValue(property, *mem, node);
			}
		}
	}

	void Node::SerializeAsChildNode(Yaml::Node& root, NodeSerializer& serializer)
	{
		// Serialize Non-Inherited Children
		if (m_IsActorLayer)
		{
			Yaml::Node& child = root["Children"][std::to_string(serializer.m_ChildCount++)];
			child["Name"] = GetName();
			child["Enabled"] = IsEnabled() ? "true" : "false";
			child["Node3D"] = IsA<Node3D>() ? Vec::ToString<glm::mat4>(this->As<Node3D>()->GetTransformMatrix()) : "";
			child["Class"] = GetClass().ToString();
			child["SocketName"] = GetParent()->GetName();
		}
		else // Serialize Inherited Children
		{
			Yaml::Node& child = root["InheretedChildren"][std::to_string(serializer.m_InheretedChildCount++)];
			child["Name"] = GetName();
			child["Enabled"] = IsEnabled() ? "true" : "false";
			child["Node3D"] = IsA<Node3D>() ? Vec::ToString<glm::mat4>(this->As<Node3D>()->GetTransformMatrix()) : "";
			std::string indicies;
			Node* node = this;
			bool firstIndex = true;
			while (!node->m_IsActorLayer && node != serializer.m_RootNode)
			{
				indicies.insert(0, std::to_string(node->GetChildIndex()) + (firstIndex ? "" : "/"));
				node = node->GetParent();
				firstIndex = false;
			}
			child["Owner"] = node->GetName();
			child["Indicies"] = indicies;
		}

		// Serialize Properties
		SerializeProperties(root, serializer, this);

		// Serialize my Children
		for (Node* child : m_Children)
		{
			child->SerializeAsChildNode(root, serializer);
		}
	}
	void Node::Serialize(Yaml::Node& root)
	{
		root = Yaml::Node();
		root["RootName"] = GetName();
		root["RootClass"] = GetClass().ToString();
		root["RootParentClass"] = GetClass().GetParentClass().ToString();
		root["RootEnabled"] = m_Enabled ? "true" : "false";

		NodeSerializer serializer;
		serializer.m_RootNode = this;

		// Serialize Properties
		SerializeProperties(root, serializer, this);

		for (Node* child : m_Children)
		{
			child->SerializeAsChildNode(root, serializer);
		}

		root["ChildCount"] = std::to_string(serializer.m_ChildCount);
		root["InheretedChildCount"] = std::to_string(serializer.m_InheretedChildCount);
		root["PropertyCount"] = std::to_string(serializer.m_PropertyCount);
	}


	struct NodeDeserializer
	{
		struct InheretedChildNodes
		{
			Array<std::string> Name;
			Array<std::string> Indicies;
			Array<std::string> Enabled;
			Array<std::string> Node3DTransform;

			void Apply(Node* affetedNode)
			{
				for (int32_t i = 0; i < Name.Size(); i++)
				{
					std::vector<std::string> indicies = Util::SplitString(Indicies[i], '/');
					Node* node = affetedNode;
					while (node && !indicies.empty())
					{
						node = node->GetChild(std::stoi(indicies[0]));
						indicies.erase(indicies.begin());
					}
					if (node)
					{
						node->SetName(Name[i]);
						node->SetEnabled(Enabled[i] == "true");
						if (node->IsA<Node3D>() && !Node3DTransform[i].empty())
						{
							node->As<Node3D>()->SetTransformMatrix(Vec::FromString<glm::mat4>(Node3DTransform[i]));
						}
					}
				}
			}
		};

		NodeDeserializer(Yaml::Node& root)
		{
			if (!root["InheretedChildCount"].IsNone())
			{
				const int32_t inheretedChildCount = std::stoi(root["InheretedChildCount"].As<std::string>());
				for (int32_t i = 0; i < inheretedChildCount; i++)
				{
					Yaml::Node& yamlChild = root["InheretedChildren"][std::to_string(i)];
					const std::string childOwner = yamlChild["Owner"].As<std::string>();
					if (m_InheretedChildren.find(childOwner) == m_InheretedChildren.end())
						m_InheretedChildren[childOwner] = CreateRef<InheretedChildNodes>();

					m_InheretedChildren[childOwner]->Name.Add(yamlChild["Name"].As<std::string>());
					m_InheretedChildren[childOwner]->Indicies.Add(yamlChild["Indicies"].As<std::string>());
					m_InheretedChildren[childOwner]->Enabled.Add(yamlChild["Enabled"].As<std::string>());
					m_InheretedChildren[childOwner]->Node3DTransform.Add(yamlChild["Node3D"].As<std::string>());
				}
			}
		}
		std::unordered_map<std::string, Ref<InheretedChildNodes>> m_InheretedChildren;
	};

	Node* Node::Deserialize(Yaml::Node& root, const bool isRootNode)
	{
		Node* node = New(Class::FromString(root["RootParentClass"].As<std::string>()), false)->As<Node>();
		SUORA_ASSERT(node);
		node->m_Name = root["RootName"].As<std::string>();

		// Rename all Inherited Children
		NodeDeserializer deserializer = NodeDeserializer(root);
		if (deserializer.m_InheretedChildren.find(node->GetName()) != deserializer.m_InheretedChildren.end())
		{
			deserializer.m_InheretedChildren[node->GetName()]->Apply(node);
		}

		const int32_t childCount = std::stoi(root["ChildCount"].As<std::string>());
		for (int32_t i = 0; i < childCount; i++)
		{
			Yaml::Node& yamlChild = root["Children"][std::to_string(i)];
			const Class childClass = Class::FromString(yamlChild["Class"].As<std::string>());
			const std::string childSocket = yamlChild["SocketName"].As<std::string>();

			Node* child = New(childClass, false)->As<Node>();
			child->SetName(yamlChild["Name"].As<std::string>());
			child->SetEnabled(yamlChild["Enabled"].As<std::string>() == "true");
			if (child->IsA<Node3D>())
			{
				if (!yamlChild["Node3D"].IsNone())
				{
					child->As<Node3D>()->SetTransformMatrix(Vec::FromString<glm::mat4>(yamlChild["Node3D"].As<std::string>()));
				}
			}

			// Attach Child to Socket (if Socket exists)
			Node* socket = node;
			if (!childSocket.empty())
			{
				Node* newSocket = node->GetChildByName(childSocket);
				if (newSocket)
				{
					socket = newSocket;
				}
			}
			child->ForceSetParent(socket, true, false);

			// Rename all Inherited Children
			if (deserializer.m_InheretedChildren.find(child->GetName()) != deserializer.m_InheretedChildren.end())
			{
				deserializer.m_InheretedChildren[child->GetName()]->Apply(child);
			}

			if (isRootNode)
			{
				child->m_IsActorLayer = true;
			}

		}

		if (!root["PropertyCount"].IsNone())
		{
			const int32_t propertyCount = std::stoi(root["PropertyCount"].As<std::string>());
			for (int32_t i = 0; i < propertyCount; i++)
			{
				Yaml::Node& yamlProperty = root["Properties"][std::to_string(i)];
				const std::string nodeName = yamlProperty["NodeName"].As<std::string>();
				const std::string propertyName = yamlProperty["PropertyName"].As<std::string>();
				Node* applyPropertyTo = node->GetChildByName(nodeName);

				if (applyPropertyTo == nullptr)
				{
					continue;
				}

				const ClassReflector& refl = ClassReflector::GetByClass(applyPropertyTo->GetNativeClass());
				const auto allMembers = refl.GetAllClassMember();
				for (const Ref<ClassMember>& mem : allMembers)
				{
					if (mem->m_MemberName == propertyName)
					{
						ReadPropertyValue(yamlProperty, *mem, applyPropertyTo);

						if (isRootNode)
						{
							applyPropertyTo->m_OverwrittenProperties.Add(propertyName);
						}

						break;
					}
				}
			}
		}

		if (isRootNode)
		{
			node->m_IsActorLayer = true;
		}

		return node;
	}

	void Node::ResetProperty(const ClassMember& member)
	{
		Ref<Node> root;
		Node* original = nullptr;

		if (m_IsActorLayer)
		{
			root = Ref<Node>(New(GetClass(), false)->As<Node>());
			original = root.get();
		}
		else
		{
			Array<int32_t> indicies;
			Node* node = this;
			while (!node->m_IsActorLayer && node != nullptr)
			{
				indicies.Add(node->GetChildIndex());
				node = node->GetParent();
			}
			root = Ref<Node>(New(node->GetClass(), false)->As<Node>());
			original = root.get();
			for (int32_t i = indicies.Last(); i >= 0; i--)
			{
				if (original)
				{
					original = original->GetChild(indicies[i]);
				}
			}
		}

		if (original)
		{
			Yaml::Node temp;

			WritePropertyValue(temp, member, original);
			ReadPropertyValue(temp, member, this);

			if (m_OverwrittenProperties.Contains(member.m_MemberName))
			{
				m_OverwrittenProperties.Remove(member.m_MemberName);
			}
		}
	}


}