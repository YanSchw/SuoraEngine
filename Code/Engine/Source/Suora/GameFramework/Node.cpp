#include "Precompiled.h"
#include "Node.h"
#include "World.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>


#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/matrix_decompose.hpp>

#include "Suora/Common/Common.h"
#include "Suora/Core/Engine.h"
#include "Suora/GameFramework/GameInstance.h"
#include "Suora/GameFramework/InputModule.h"
#include "Suora/NodeScript/NodeScriptObject.h"
#include "Suora/Core/Object/Pointer.h"

namespace Suora
{

	/**   Node   **/

	Node::Node()
	{
	}
	Node::~Node()
	{
		// Unparent Children - Before actually destroying the Parent --- if the World exits
		if (GetWorld())
		{
			while (HasChildren())
			{
				GetChild(0)->ForceSetParent(GetParent(), true, false);
			}
		}
		else
		{
			// There is no World, the Node likely exits on the Stack and not the Heap.
			// Manually delete this Nodes children...
			while (HasChildren())
			{
				Node* child = GetChild(0);
				child->ForceSetParent(GetParent(), true, false);
				delete child;
			}
		}

		ForceSetParent(nullptr, true, false);

		if (GetWorld())
		{
			GetWorld()->UnregisterNode(this);
		}
	}

	void Node::WorldUpdate(float deltaTime)
	{
		NODESCRIPT_EVENT_DISPATCH("Node::WorldUpdate(float)", deltaTime);
	}
	void Node::LocalUpdate(float deltaTime)
	{
		NODESCRIPT_EVENT_DISPATCH("Node::LocalUpdate(float)", deltaTime);
	}

	void Node::SetUpdateFlag(UpdateFlag flag)
	{
		m_UpdateFlags |= flag;

		if (!IsInitialized())
		{
			return;
		}

		GetWorld()->ReregisterNode(this);
	}

	bool Node::IsUpdateFlagSet(UpdateFlag flag) const
	{
		return 0 != ((int32_t)m_UpdateFlags & (int32_t)flag);
	}

	bool Node::ShouldUpdateInCurrentContext() const
	{
		return IsEnabled() && m_WasBeginCalled && !m_IsPendingKill;
	}

	bool Node::IsPendingKill() const
	{
		return m_IsPendingKill;
	}

	World* Node::GetWorld() const
	{
		LOCAL_UPDATE_LOCK();
		return m_World;
	}

	bool Node::DoesNameExistInHierarchy(const String& name, Node* root, Node* exclude)
	{
		if (!root) return false;
		if (root->GetName() == name && root != exclude) return true;

		for (int i = 0; i < root->GetChildCount(); i++)
		{
			if (DoesNameExistInHierarchy(name, root->GetChild(i), exclude)) return true;
		}

		return false;
	}
	void Node::IncrementNameIndex()
	{
		if (m_Name[m_Name.size() - 1] != ']')
		{
			m_Name += " [1]";
		}
		else
		{
			int32_t currentIndex = 0;
			int32_t currentFactor = 1;
			for (int32_t i = m_Name.size() - 2; i >= 0; i--)
			{
				char number = m_Name[i];
				if ((number >= '1' && number <= '9') || number == '0')
				{
					currentIndex += (number - '0') * currentFactor;
					currentFactor *= 10;
				}
				else if (number == '[')
				{
					m_Name = m_Name.substr(0, i + 1);
					m_Name += std::to_string(++currentIndex);
					m_Name += "]";
					return;
				}
				else
				{
					m_Name += " [1]";
					return;
				}
			}
		}
	}

	void Node::SetName(const String& name)
	{
		if (name.empty())
		{
			SetName(GetClass().GetClassName());
			return;
		}

		if (m_WasBeginCalled)
		{
			m_Name = name;
		}
		else
		{
			// Duplicate names are not allowed, in a non-gameplay context!
			bool alreadyExists = DoesNameExistInHierarchy(name, GetRootNode(), this);
			if (!alreadyExists)
			{
				m_Name = name;
			}
			else
			{
				m_Name = name;
				IncrementNameIndex();
			}
		}
	}

	String Node::GetName() const
	{
		return m_Name;
	}

	Node* Node::Duplicate()
	{
		Yaml::Node Serialized;
		Serialize(Serialized);
		Serialized["RootParentClass"] = GetClass().ToString();

		if (!GetWorld())
		{
			SuoraError("Node::Duplicate(): Cannot duplicate Node. No World context.");
			return nullptr;
		}

		Node* duplicate = Deserialize(Serialized, true);
		if (IsInitialized())
		{
			duplicate->InitializeNode(*GetWorld());
		}
		duplicate->SetParent(GetParent());
		if (duplicate->IsA<Node3D>())
		{
			duplicate->As<Node3D>()->TickTransform();
		}

		// Still bad....
		// TODO: Fix -> Some ChildNodes dont get properly deserialized
		duplicate->m_IsActorLayer = true;

		if (!m_WasBeginCalled)
		{
			while (DoesNameExistInHierarchy(duplicate->GetName(), duplicate->GetRootNode(), duplicate))
			{
				duplicate->SetName(duplicate->GetName());
			}
		}

		return duplicate;
	}

	void Node::Destroy()
	{
		if (m_IsPendingKill)
		{
			// The Node will already die at the end of the frame!
			return;
		}
		m_IsPendingKill = true;

		if (m_WasBeginCalled)
		{
			OnNodeDestroy();
		}

		Array<Node*> ChildrenToKill = m_Children;
		for (Node* child : ChildrenToKill)
		{
			SuoraAssert(child);
			child->Destroy();
		}

		SuoraAssert(GetWorld());
		SuoraAssert(GetWorld()->m_PendingKills.Contains(this) == false);
		GetWorld()->m_PendingKills.Add(this);
	}

	void Node::Possess()
	{
		GetWorld()->SetPawn(this);
	}

	bool Node::IsPlayerPawn()
	{
		return GetWorld()->GetPlayerPawn() == this;
	}

	bool Node::IsPossessed()
	{
		if (IsPlayerPawn()) return true;
		if (GetParent() == nullptr) return false;

		return GetParent()->IsPossessed();
	}

	void Node::OnPossess()
	{
		NODESCRIPT_EVENT_DISPATCH("Node::OnPossess()");

		Array<Ptr<Node>> children;
		for (Node* child : m_Children) children.Add(child);
		for (auto& It : children)
		{
			if (It)
			{
				It->OnPossess();
			}
		}
	}

	Node* Node::GetActorNode()
	{
		return this->IsA<LevelNode>() ? 
			nullptr 
			: (m_IsActorLayer ? 
				this 
				: (GetParent() ? GetParent()->GetActorNode() : nullptr));
	}
	bool Node::IsTheActorNode()
	{
		return GetActorNode() == this;
	}

	Node* Node::GetRootNode()
	{
		return GetParent() ? GetParent()->GetRootNode() : this;
	}

	void Node::Replicate(bool b)
	{
		if (IsInitialized())
		{
			SuoraWarn("You may not change replication status after initialization.");
			return;
		}

		m_Replicated = b;
	}

	bool Node::IsReplicated() const
	{
		return m_Replicated;
	}

	Node* Node::CreateChild(const Class& cls)
	{
		Node* node = (Node*)New(cls);
		SUORA_ASSERT(node);
		SUORA_ASSERT(node->IsA<Node>(), "You can only add Nodes as Children.");

		if (node->IsA<Component>())
		{
			if (!GetClass().Inherits(node->As<Component>()->GetRequiredBaseClass()))
			{
				SUORA_ERROR(LogCategory::Gameplay, "Cannot create Component, without given BaseClass '" + node->As<Component>()->GetRequiredBaseClass().GetClassName() + "'");
				delete node;
				return nullptr;
			}
		}
		
		node->SetParent(this);

		if (IsInitialized())
		{
			node->InitializeNode(*GetWorld());
		}

		return node;
	}

	Node* Node::GetChildByName(const String& name)
	{
		if (GetName() == name) return this;

		for (int i = 0; i < GetChildCount(); i++)
		{
			Node* node = GetChild(i)->GetChildByName(name);
			if (node) return node;
		}

		return nullptr;
	}

	Node* Node::GetChildNodeOfClass(const Class& cls, bool includeSelf)
	{
		for (int i = 0; i < GetChildCount(); i++)
		{
			Node* node = GetChild(i)->GetChildNodeOfClass(cls, true);
			if (node)
			{
				return node;
			}
		}

		return includeSelf ? Cast(this, cls) : nullptr;
	}

	void Node::GetChildNodesOfClass(const Class& cls, Array<Node*>& OutArray, bool includeSelf)
	{
		for (int i = 0; i < GetChildCount(); i++)
		{
			GetChild(i)->GetChildNodesOfClass(cls, OutArray, true);
		}

		if (includeSelf && Cast(this, cls))
		{
			OutArray.Add(this);
		}

	}
	int32_t Node::GetChildIndex() const
	{
		if (!GetParent())
		{
			return -1;
		}

		int32_t idx = 0;
		for (const Node* child : GetParent()->m_Children)
		{
			if (child == this)
			{
				return idx;
			}
			idx++;
		}

		return -1;
	}

	Node* Node::GetParentNodeOfClass(const Class& cls, bool includeSelf)
	{
		return includeSelf ? (Cast(this, cls) ? this : (GetParent() ? GetParent()->GetParentNodeOfClass(cls, true) : nullptr)) 
						   : (GetParent() ? GetParent()->GetParentNodeOfClass(cls, true) : nullptr);
	}

	void Node::InitializeNode(World& world)
	{
		SUORA_ASSERT(GetWorld() == nullptr);
		if (GetName() == "New Node") SetName(GetClass().GetClassName());
		world.m_WorldNodes.Add(this);
		m_World = &world;
		m_Initialized = true;

		// Create Issue
		world.m_BeginPlayIssues.Add(this);

		// UpdateFlags; ... are now set, after the BeginPlay() Issue was resolved by the World.
		// REASON: Update() could be called before BeginPlay() was called
		/*GetWorld()->ReregisterNode(this);*/

		if (Implements<INodeScriptObject>())
		{
			GetInterface<INodeScriptObject>()->InitializeBlueprintInstance(world);
		}

		for (int32_t i = 0; i < GetChildCount(); i++)
		{
			GetChild(i)->InitializeNode(world);
		}
	}

	void Node::UnInitializeNode(World& world)
	{
	}

	bool Node::IsInitialized() const
	{
		return m_Initialized;
	}

	bool Node::IsEnabled() const
	{
		return m_EnabledInHierarchy;
	}

	void Node::SetEnabled(bool enabled)
	{
		m_Enabled = enabled;
		m_EnabledInHierarchy = enabled ? (GetParent() ? GetParent()->m_EnabledInHierarchy : enabled) : false;

		TickTransform();
	}

	Node3D* Node::GetTransform()
	{
		return GetParent() ? GetParent()->GetTransform() : nullptr;
	}

	Node3D* Node::GetParentTransform()
	{
		return GetParent() ? GetParent()->GetTransform() : nullptr;
	}

	UINode* Node::GetUITransform()
	{
		return GetParent() ? GetParent()->GetUITransform() : nullptr;
	}

	UINode* Node::GetParentUITransform()
	{
		return GetParent() ? GetParent()->GetUITransform() : nullptr;
	}

	void Node::TickTransform(bool inWorldSpace)
	{
		for (Node* child : m_Children)
		{
			if (child->m_Enabled)
			{
				child->m_EnabledInHierarchy = m_EnabledInHierarchy;
			}

			child->TickTransform(false);
		}
	}

	void Node::OnParentChange(Node* prev, Node* next)
	{
	}

	void Node::ForceSetParent(Node* parent, bool keepWorldTransform, bool inGameplayContext)
	{
		if (parent && parent->IsChildOf(this))
		{
			return;
		}
		if (inGameplayContext)
		{
			OnParentChange(GetParent(), parent);
		}

		if (GetParent())
		{
			GetParent()->m_Children.Remove(this);
		}

		m_Parent = parent;

		if (GetParent())
		{
			GetParent()->m_Children.Add(this);
		}

		// Reenable or Disable
		if (GetParent())
		{
			if (m_Enabled && m_EnabledInHierarchy != GetParent()->m_EnabledInHierarchy)
			{
				m_EnabledInHierarchy = GetParent()->m_EnabledInHierarchy;
			}
		}
		else
		{
			m_EnabledInHierarchy = m_Enabled;
		}


		if (Node3D* transform = GetTransform())
		{
			transform->RecalculateTransformMatrix();
		}
	}
	void Node::SetParent(Node* parent, bool keepWorldTransform)
	{
		ForceSetParent(parent, keepWorldTransform, true);
	}

	// Serialization in NodeSerialization.cpp

	/**   Node3D   **/

	Node3D::Node3D()
	{
		const auto& scaled = glm::scale(Mat4(1), Vec3(1));
		const auto& rotated = glm::toMat4(glm::identity<Quat>()) * scaled;
		m_WorldTransformMatrix = glm::translate(Mat4(1), Vec3(0)) * rotated;
		TickTransform(false);
	}
	Node3D::~Node3D()
	{
		/*DetachAllChildren(GetParent());
		SetParent(nullptr);*/
	}

	void Node3D::SetPosition(const Vec3& position)
	{
		//GetTransformMatrix()[3] = Vec4(position, 1.0);

		const auto& scaled = glm::scale(Mat4(1), GetScale());

		const auto& rotated = glm::toMat4(GetRotation()) * scaled;
		m_WorldTransformMatrix = glm::translate(Mat4(1), position) * rotated;

		TickTransform(true);
	}
	
	void Node3D::AddWorldOffset(const Vec3& offset)
	{
		SetPosition(GetPosition() + offset);
	}
	
	Vec3 Node3D::GetPosition() const
	{
		return Vec3(GetTransformMatrix()[3]);
	}
	
	Vec3 Node3D::GetLocalPosition() const
	{
		return Vec3(m_LocalTransformMatrix[3]);
	}

	void Node3D::SetLocalPosition(const Vec3& position)
	{
		const auto& scaled = glm::scale(Mat4(1), GetLocalScale());

		const auto& rotated = glm::toMat4(GetLocalRotation()) * scaled;
		m_LocalTransformMatrix = glm::translate(Mat4(1), position) * rotated;

		ReprojectLocalMatrixToWorld();
		TickTransform(true);
	}

	Quat Node3D::GetRotation() const
	{
		Vec3 scale;
		glm::quat rotation;
		Vec3 translation;
		Vec3 skew;
		Vec4 perspective;
		glm::decompose(m_WorldTransformMatrix, scale, rotation, translation, skew, perspective);
		return rotation;
	}
	Quat Node3D::GetLocalRotation() const
	{
		Vec3 scale;
		glm::quat rotation;
		Vec3 translation;
		Vec3 skew;
		Vec4 perspective;
		glm::decompose(m_LocalTransformMatrix, scale, rotation, translation, skew, perspective);
		return rotation;
	}
	Vec3 Node3D::GetEulerRotation() const
	{
		return glm::degrees(glm::eulerAngles(GetRotation()));
	}
	Vec3 Node3D::GetLocalEulerRotation() const
	{
		return glm::degrees(glm::eulerAngles(GetLocalRotation()));
	}
	void Node3D::SetRotation(const Quat& rot)
	{
		const auto& scaled = glm::scale(Mat4(1), GetScale());
		const auto& rotated = glm::toMat4(rot) * scaled;
		m_WorldTransformMatrix = glm::translate(Mat4(1), GetPosition()) * rotated;
		TickTransform(true);
	}
	void Node3D::SetLocalRotation(const Quat& rot)
	{
		const auto& scaled = glm::scale(Mat4(1), GetLocalScale());
		const auto& rotated = glm::toMat4(rot) * scaled;
		m_LocalTransformMatrix = glm::translate(Mat4(1), GetLocalPosition()) * rotated;
		ReprojectLocalMatrixToWorld();
		TickTransform(true);
	}
	void Node3D::SetEulerRotation(const Vec3& eulerAngles)
	{
		SetRotation(Quat(glm::radians(eulerAngles)));
	}

	void Node3D::SetLocalEulerRotation(const Vec3& eulerAngles)
	{
		SetLocalRotation(Quat(glm::radians(eulerAngles)));
	}

	void Node3D::Rotate(const Quat& quat)
	{
		// TODO: Double check this 
		SetRotation(GetRotation() * quat);
	}

	void Node3D::RotateEuler(const Vec3& eulerAngles)
	{
		SetRotation(glm::normalize(GetRotation() * glm::quat(eulerAngles)));
	}
	void Node3D::RotateWithAxis(const Vec3& axis, float angle)
	{
		m_WorldTransformMatrix = glm::rotate(m_WorldTransformMatrix, glm::radians(angle), axis);
		TickTransform(true);
	}

	void Node3D::SetLookDirection(const Vec3& direction, const Vec3& up)
	{
		SetRotation(glm::quatLookAtLH(direction, up));
	}
	Vec3 Node3D::GetScale() const
	{
		Vec3 scale;
		glm::quat rotation;
		Vec3 translation;
		Vec3 skew;
		Vec4 perspective;
		glm::decompose(m_WorldTransformMatrix, scale, rotation, translation, skew, perspective);
		return scale;
	}
	Vec3 Node3D::GetLocalScale() const
	{
		Vec3 scale;
		glm::quat rotation;
		Vec3 translation;
		Vec3 skew;
		Vec4 perspective;
		glm::decompose(m_LocalTransformMatrix, scale, rotation, translation, skew, perspective);
		return scale;
	}
	void Node3D::SetScale(const Vec3& scale)
	{
		if (scale.x <= 0.0f || scale.y <= 0.0f || scale.z <= 0.0f) return;
		const auto& scaled = glm::scale(Mat4(1), scale);

		const auto& rotated = glm::toMat4(GetRotation()) * scaled;
		m_WorldTransformMatrix = glm::translate(Mat4(1), GetPosition()) * rotated;
		TickTransform(true);
	}
	void Node3D::SetLocalScale(const Vec3& scale)
	{
		if (scale.x <= 0.0f || scale.y <= 0.0f || scale.z <= 0.0f) return;
		const auto& scaled = glm::scale(Mat4(1), scale);

		const auto& rotated = glm::toMat4(GetLocalRotation()) * scaled;
		m_LocalTransformMatrix = glm::translate(Mat4(1), GetLocalPosition()) * rotated;
		ReprojectLocalMatrixToWorld();
		TickTransform(true);
	}

	Vec3 Node3D::GetRightVector() const
	{
		//const Mat4 inverted = glm::inverse(GetTransformMatrix());
		const Vec3 right = glm::normalize(Vec3(GetTransformMatrix()[0]));
		return right;
	}
	Vec3 Node3D::GetUpVector() const
	{
		//const Mat4 inverted = glm::inverse(GetTransformMatrix());
		const Vec3 up = glm::normalize(Vec3(GetTransformMatrix()[1]));
		return up;
	}
	Vec3 Node3D::GetForwardVector() const
	{
		//const Mat4 inverted = glm::inverse(GetTransformMatrix());
		const Vec3 forward = glm::normalize(Vec3(GetTransformMatrix()[2]));
		return forward;
	}

	Mat4 Node3D::GetTransformMatrix() const
	{
		return m_WorldTransformMatrix;
	}

	void Node3D::RecalculateTransformMatrix()
	{
		bool inverseParentTransform = false;

		/// Source: https://stackoverflow.com/questions/11920866/global-transform-to-local-transform
		m_LocalTransformMatrix = GetParentTransform() ?
			((!inverseParentTransform ? glm::inverse(GetParentTransform()->GetTransformMatrix()) : GetParentTransform()->GetTransformMatrix()) * m_WorldTransformMatrix) :
			m_WorldTransformMatrix;

	}

	void Node3D::ReprojectLocalMatrixToWorld()
	{
		if (!GetParentTransform())
		{
			m_WorldTransformMatrix = m_LocalTransformMatrix;
			return;
		}

		m_WorldTransformMatrix = GetParentTransform()->m_WorldTransformMatrix * m_LocalTransformMatrix;
	}

	Mat4 Node3D::CalculateTransformMatrix(const Vec3& position, const Vec3& eulerAngles, const Vec3& scale)
	{
		const Mat4 rotation = glm::toMat4(Quat({ glm::radians(eulerAngles.x), glm::radians(eulerAngles.y), glm::radians(eulerAngles.z) }));

		return glm::translate(Mat4(1.0f), { position.x, position.y, position.z })
			* rotation
			* glm::scale(Mat4(1.0f), { scale.x, scale.y, scale.z });
	}

	Mat4 Node3D::CalculateTransformMatrix(const Vec3& position, const Quat& rotation, const Vec3& scale)
	{
		const Mat4 rot = glm::toMat4(rotation);

		return glm::translate(Mat4(1.0f), { position.x, position.y, position.z })
			* rot
			* glm::scale(Mat4(1.0f), { scale.x, scale.y, scale.z });
	}

	void Node3D::TickTransform(bool inWorldSpace)
	{
		if (!inWorldSpace)
		{
			ReprojectLocalMatrixToWorld();
		}
		else
		{
			RecalculateTransformMatrix();
		}

		Super::TickTransform(inWorldSpace);

		RecalculateTransformMatrix();
	}

	void Node3D::InitializeNode(World& world)
	{
		Super::InitializeNode(world);
	}



	void UINode::TransformToYaml(Yaml::Node& root) const
	{
		root["m_Anchor"] = Vec::ToString(m_Anchor);
		root["m_IsWidthRelative"] = m_IsWidthRelative ? "true" : "false";
		root["m_Width"] = std::to_string(m_Width);
		root["m_IsHeightRelative"] = m_IsHeightRelative ? "true" : "false";
		root["m_Height"] = std::to_string(m_Height);
		root["m_Pivot"] = Vec::ToString(m_Pivot);
		root["m_AbsolutePixelOffset"] = Vec::ToString(m_AbsolutePixelOffset);
		root["m_EulerRotationAroundAnchor"] = Vec::ToString(m_EulerRotationAroundAnchor);
	}

	void UINode::TransformFromYaml(Yaml::Node& root)
	{
		if (root.IsNone())
		{
			return;
		}

		m_Anchor = Vec::FromString<Vec2>(root["m_Anchor"].As<String>());
		m_IsWidthRelative = root["m_IsWidthRelative"].As<String>() == "true";
		m_Width = std::stof(root["m_Width"].As<String>());
		m_IsHeightRelative = root["m_IsHeightRelative"].As<String>() == "true";
		m_Height = std::stof(root["m_Height"].As<String>());
		m_Pivot = Vec::FromString<Vec2>(root["m_Pivot"].As<String>());
		m_AbsolutePixelOffset = Vec::FromString<Vec3>(root["m_AbsolutePixelOffset"].As<String>());
		m_EulerRotationAroundAnchor = Vec::FromString<Vec3>(root["m_EulerRotationAroundAnchor"].As<String>());
	}

	UINode* UINode::GetUITransform()
	{
		return this;
	}

	UINode::RectTransform UINode::GetRectTransform()
	{
		UINode* parentNode = GetParentUITransform();
		RectTransform parentTransform = parentNode ? parentNode->GetRectTransform() : RectTransform();

		Vec3 anchorPos = parentTransform.UpperLeft + parentTransform.GetHalfRight() + parentTransform.GetHalfDown();
		anchorPos += parentTransform.GetHalfRight() * m_Anchor.x;
		anchorPos -= parentTransform.GetHalfDown() * m_Anchor.y;

		// Collapse RectTransform into Anchor
		RectTransform transform = parentTransform;
		transform.UpperLeft   = anchorPos;
		transform.UpperRight  = anchorPos;
		transform.BottomLeft  = anchorPos;

		// Now, blow up the RectTransform again
		transform.UpperLeft  -= parentTransform.GetHalfRight() * m_Width * (m_IsWidthRelative ? 1.0f : GetViewportPixelScale().x);
		transform.UpperRight += parentTransform.GetHalfRight() * m_Width * (m_IsWidthRelative ? 1.0f : GetViewportPixelScale().x);
		transform.BottomLeft -= parentTransform.GetHalfRight() * m_Width * (m_IsWidthRelative ? 1.0f : GetViewportPixelScale().x);

		transform.UpperLeft  -= parentTransform.GetHalfDown() * m_Height * (m_IsHeightRelative ? 1.0f : GetViewportPixelScale().y);
		transform.UpperRight -= parentTransform.GetHalfDown() * m_Height * (m_IsHeightRelative ? 1.0f : GetViewportPixelScale().y);
		transform.BottomLeft += parentTransform.GetHalfDown() * m_Height * (m_IsHeightRelative ? 1.0f : GetViewportPixelScale().y);

		// Apply Pivot
		{
			const Vec3 halfRight = transform.GetHalfRight();
			transform.UpperLeft -= halfRight * m_Pivot.x;
			transform.UpperRight -= halfRight * m_Pivot.x;
			transform.BottomLeft -= halfRight * m_Pivot.x;

			const Vec3 halfDown = transform.GetHalfDown();
			transform.UpperLeft -= halfDown * m_Pivot.y;
			transform.UpperRight -= halfDown * m_Pivot.y;
			transform.BottomLeft -= halfDown * m_Pivot.y;
		}

		// TODO:
		// 
		// Apply Offset

		// Apply Rotation


		return transform;
	}

	Vec2 UINode::GetViewportPixelScale()
	{
		return Vec2(1.0f / (float)s_UIViewportWidth, 1.0f / (float)s_UIViewportHeight);
	}



	void Component::Begin()
	{
		Super::Begin();

		m_IsActorLayer = false;
	}

	void Component::SetParent(Node* parent, bool keepWorldTransform)
	{
		if (!GetParent())
		{
			Super::SetParent(parent, keepWorldTransform);
		}
		else
		{
			SuoraWarn("A Component may never be reparented!");
		}
	}

	void Component::SetRequiredBaseClass(const Class& baseClass)
	{
		m_ParentBaseclass = baseClass;
	}

	Class Component::GetRequiredBaseClass() const
	{
		return m_ParentBaseclass;
	}

}