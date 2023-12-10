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
		case ClassMember::Type::ObjectPtr:
			{
				const Class objClass = ((ClassMember_ObjectPtr*)(&member))->m_ObjectClass;
				if (objClass.Inherits(Asset::StaticClass()))
				{
					const Asset* asset = *ClassMember::AccessMember<Asset*>(node, member.m_MemberOffset);
					property["Value"]["AssetPtr"] = (asset) ? asset->m_UUID.GetString() : "0";
				}
				else
				{
					SuoraError("Cannot serialize ObjectPtr");
				}
				break;
			}
		case ClassMember::Type::MaterialSlots:
		{
			MaterialSlots* slots = (ClassMember::AccessMember<MaterialSlots>(node, member.m_MemberOffset));
			property["Value"]["MaterialSlots"]["Overwrite"] = slots->OverwritteMaterials ? "true" : "false";
			if (slots->OverwritteMaterials)
			{
				for (int i = 0; i < slots->Materials.Size(); i++)
				{
					property["Value"]["MaterialSlots"][std::to_string(i)] = (slots->Materials[i]) ? slots->Materials[i]->m_UUID.GetString() : "0";
				}
			}
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
			*ClassMember::AccessMember<int32_t>(node, member.m_MemberOffset) = std::stoi(property["Value"]["Int32"].As<String>());
			break;
		case ClassMember::Type::Float:
			*ClassMember::AccessMember<float>(node, member.m_MemberOffset) = std::stof(property["Value"]["Float"].As<String>());
			break;
		case ClassMember::Type::Bool:
			*ClassMember::AccessMember<bool>(node, member.m_MemberOffset) = property["Value"]["Bool"].As<String>() == "true";
			break;
		case ClassMember::Type::Vector3:
			*ClassMember::AccessMember<Vec3>(node, member.m_MemberOffset) = Vec::FromString<Vec3>(property["Value"]["Vec3"].As<String>());
			break;
		case ClassMember::Type::Vector4:
			*ClassMember::AccessMember<Vec4>(node, member.m_MemberOffset) = Vec::FromString<Vec4>(property["Value"]["Vec4"].As<String>());
			break;
		case ClassMember::Type::ObjectPtr:
		{
			const Class objClass = ((ClassMember_ObjectPtr*)(&member))->m_ObjectClass;
			if (objClass.Inherits(Asset::StaticClass()))
			{
				Asset*& assetRef = (Asset*&)*ClassMember::AccessMember<Asset*>(node, member.m_MemberOffset);
				const String assetUUID = property["Value"]["AssetPtr"].As<String>();
				Asset* asset = (assetUUID != "0" && assetUUID != "") ? AssetManager::GetAsset(objClass, SuoraID(assetUUID)) : nullptr;

				if (asset)
				{
					SuoraAssert(asset->GetClass().Inherits(objClass));
				}
				assetRef = asset;
			}
			else
			{
				SuoraError("Cannot deserialize ObjectPtr");
			}
			break;
		}
		case ClassMember::Type::MaterialSlots:
		{
			MaterialSlots ValueMaterialSlots;
			ValueMaterialSlots.OverwritteMaterials = property["Value"]["MaterialSlots"]["Overwrite"].As<String>() == "true";
			ValueMaterialSlots.Materials.Clear();
			int i = 0;
			while (true)
			{
				Yaml::Node& material = property["Value"]["MaterialSlots"][std::to_string(i)];
				if (material.IsNone()) break;
				ValueMaterialSlots.Materials.Add((material.As<String>() != "0")
					? AssetManager::GetAsset<Material>(material.As<String>())
					: nullptr);
				i++;
			}
			*ClassMember::AccessMember<MaterialSlots>(node, member.m_MemberOffset) = ValueMaterialSlots;
		} break;
		case ClassMember::Type::Class:
			*ClassMember::AccessMember<Class>(node, member.m_MemberOffset) = Class::FromString(property["Value"]["Class"].As<String>());
			break;
		case ClassMember::Type::SubclassOf:
			*ClassMember::AccessMember<TSubclassOf>(node, member.m_MemberOffset) = Class::FromString(property["Value"]["SubclassOf"].As<String>());
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
			child["Node3D"] = IsA<Node3D>() ? Vec::ToString<Mat4>(this->As<Node3D>()->GetTransformMatrix()) : "";
			if (this->IsA<UINode>()) this->As<UINode>()->TransformToYaml(child["UINode"]);
			child["Class"] = GetClass().ToString();
			child["SocketName"] = GetParent()->GetName();
		}
		else // Serialize Inherited Children
		{
			Yaml::Node& child = root["InheretedChildren"][std::to_string(serializer.m_InheretedChildCount++)];
			child["Name"] = GetName();
			child["Enabled"] = IsEnabled() ? "true" : "false";
			child["Node3D"] = IsA<Node3D>() ? Vec::ToString<Mat4>(this->As<Node3D>()->GetTransformMatrix()) : "";
			if (this->IsA<UINode>()) this->As<UINode>()->TransformToYaml(child["UINode"]);
			String indicies;
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
		root["Enabled"] = IsEnabled() ? "true" : "false";
		root["Node3D"] = IsA<Node3D>() ? Vec::ToString<Mat4>(this->As<Node3D>()->GetTransformMatrix()) : "";
		if (this->IsA<UINode>()) this->As<UINode>()->TransformToYaml(root["UINode"]);

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
			Array<String> Name;
			Array<String> Indicies;
			Array<String> Enabled;
			Array<String> Node3DTransform;
			Array<Yaml::Node> NodeUITransform;

			void Apply(Node* affetedNode)
			{
				for (int32_t i = 0; i < Name.Size(); i++)
				{
					std::vector<String> indicies = StringUtil::SplitString(Indicies[i], '/');
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
							node->As<Node3D>()->SetTransformMatrix(Vec::FromString<Mat4>(Node3DTransform[i]));
						}
						if (node->IsA<UINode>())
						{
							node->As<UINode>()->TransformFromYaml(NodeUITransform[i]);
						}
					}
				}
			}
		};

		NodeDeserializer(Yaml::Node& root)
		{
			if (!root["InheretedChildCount"].IsNone())
			{
				const int32_t inheretedChildCount = std::stoi(root["InheretedChildCount"].As<String>());
				for (int32_t i = 0; i < inheretedChildCount; i++)
				{
					Yaml::Node& yamlChild = root["InheretedChildren"][std::to_string(i)];
					const String childOwner = yamlChild["Owner"].As<String>();
					if (m_InheretedChildren.find(childOwner) == m_InheretedChildren.end())
						m_InheretedChildren[childOwner] = CreateRef<InheretedChildNodes>();

					m_InheretedChildren[childOwner]->Name.Add(yamlChild["Name"].As<String>());
					m_InheretedChildren[childOwner]->Indicies.Add(yamlChild["Indicies"].As<String>());
					m_InheretedChildren[childOwner]->Enabled.Add(yamlChild["Enabled"].As<String>());
					m_InheretedChildren[childOwner]->Node3DTransform.Add(yamlChild["Node3D"].As<String>());
					m_InheretedChildren[childOwner]->NodeUITransform.Add(yamlChild["UINode"]);
				}
			}
		}
		std::unordered_map<String, Ref<InheretedChildNodes>> m_InheretedChildren;
	};

	Node* Node::Deserialize(Yaml::Node& root, const bool isRootNode)
	{
		Node* node = New(Class::FromString(root["RootParentClass"].As<String>()), false)->As<Node>();
		SUORA_ASSERT(node);
		node->m_Name = root["RootName"].As<String>();

		// Rename all Inherited Children
		NodeDeserializer deserializer = NodeDeserializer(root);
		if (deserializer.m_InheretedChildren.find(node->GetName()) != deserializer.m_InheretedChildren.end())
		{
			deserializer.m_InheretedChildren[node->GetName()]->Apply(node);
		}

		node->SetEnabled(true);
		if (!root["Node3D"].IsNone() && node->IsA<Node3D>())
		{
			node->As<Node3D>()->SetTransformMatrix(Vec::FromString<Mat4>(root["Node3D"].As<String>()));
		}
		if (!root["UINode"].IsNone() && node->IsA<UINode>())
		{
			node->As<UINode>()->TransformFromYaml(root["UINode"]);
		}

		const int32_t childCount = std::stoi(root["ChildCount"].As<String>());
		for (int32_t i = 0; i < childCount; i++)
		{
			Yaml::Node& yamlChild = root["Children"][std::to_string(i)];
			const Class childClass = Class::FromString(yamlChild["Class"].As<String>());
			const String childSocket = yamlChild["SocketName"].As<String>();

			Node* child = New(childClass, false)->As<Node>();
			child->SetName(yamlChild["Name"].As<String>());
			child->SetEnabled(yamlChild["Enabled"].As<String>() == "true");
			if (child->IsA<Node3D>())
			{
				if (!yamlChild["Node3D"].IsNone())
				{
					child->As<Node3D>()->SetTransformMatrix(Vec::FromString<Mat4>(yamlChild["Node3D"].As<String>()));
				}
			}
			if (child->IsA<UINode>())
			{
				child->As<UINode>()->TransformFromYaml(yamlChild["UINode"]);
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
			const int32_t propertyCount = std::stoi(root["PropertyCount"].As<String>());
			for (int32_t i = 0; i < propertyCount; i++)
			{
				Yaml::Node& yamlProperty = root["Properties"][std::to_string(i)];
				const String nodeName = yamlProperty["NodeName"].As<String>();
				const String propertyName = yamlProperty["PropertyName"].As<String>();
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