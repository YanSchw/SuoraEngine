#pragma once
#include "Suora/Core/Object/Object.h"
#include "Suora/Core/Update.h"
#include "Suora/Common/VectorUtils.h"
#include "Node.generated.h"

namespace Yaml
{
	class Node;
}
namespace Suora::Physics
{
	class PhysicsWorld;
}

namespace Suora
{
	class World;
	class Node3D;
	class LevelNode;
	class InputModule;

	/** Baseclass for all Nodes in the GameFramework */
	class Node : public Object
	{
		SUORA_CLASS(57842331);
	private:
		Node* m_Parent = nullptr;
		Array<Node*> m_Children;

		World* m_World = nullptr;
		UpdateFlag m_UpdateFlags = UpdateFlag::NeverUpdate;
		bool m_Initialized = false;
		bool m_Replicated = false;
		std::string m_Name = "New Node";
		bool m_Enabled = true, m_EnabledInHierarchy = true;
		bool m_WasBeginCalled = false;
		bool m_IsPendingKill = false;
	public:
		Node();
		virtual ~Node();

		FUNCTION(NodeEvent) virtual void Begin() { NODESCRIPT_EVENT_DISPATCH("Node::Begin()"); }
		FUNCTION(NodeEvent) virtual void OnNodeDestroy() { NODESCRIPT_EVENT_DISPATCH("Node::OnNodeDestroy()"); }
		FUNCTION(NodeEvent) virtual void WorldUpdate(float deltaTime);
		FUNCTION(NodeEvent) virtual void LocalUpdate(float deltaTime);
		FUNCTION(NodeEvent) virtual void PawnUpdate(float deltaTime) { NODESCRIPT_EVENT_DISPATCH("Node::PawnUpdate(float)", deltaTime); }

		void SetUpdateFlag(UpdateFlag flag);
		bool IsUpdateFlagSet(UpdateFlag flag) const;
		/** Returns 'true', if the World is allowed to Update this Node in current Context
		*   Context: IsEnabled? WasBeginPlayCalled? IsNotPendingKill? etc. */
		bool ShouldUpdateInCurrentContext() const;

		FUNCTION(Callable, Pure)
		bool IsPendingKill() const;

		FUNCTION(Callable, Pure, TypeAccessor)
		World* GetWorld() const;

	private:
		static bool DoesNameExistInHierarchy(const std::string& name, Node* root, Node* exclude = nullptr);

		/** Before: m_Name was "MyNodeName [13]  ->  After: m_Name is "MyNodeName [14]" */
		void IncrementNameIndex();
	public:
		void SetName(const std::string& name);
		std::string GetName() const;

		FUNCTION(Callable)
		Node* Duplicate();

		FUNCTION(Callable)
		void Destroy();

		// Pawn and Actor System
		FUNCTION(Callable)
		void Possess();
		FUNCTION(Callable, Pure)
		bool IsPlayerPawn();
		FUNCTION(Callable, Pure)
		bool IsPossessed();
		FUNCTION(NodeEvent)
		virtual void OnPossess();
		FUNCTION(Callable, Pure)
		Node* GetActorNode();
		FUNCTION(Callable, Pure)
		bool IsTheActorNode();
		void MakeAllChildrenPuppets(bool includeSelf = false);

		Node* GetRootNode();

		/** Replication */
		FUNCTION(Callable)
		void Replicate(bool b);
		FUNCTION(Callable, Pure)
		bool IsReplicated() const;
		/**/

		/** InputModule */
		void ProcessInput();
		virtual void SetupInputModule(InputModule* input) { }

		// Child Nodes
		Node* CreateChild(const Class& cls);
		template<class T>
		T* CreateChild()
		{
			return CreateChild(T::StaticClass())->As<T>();
		}
		template<class T>
		T* CreateChild(const std::string& name)
		{
			T* child = CreateChild(T::StaticClass())->As<T>();
			child->SetName(name);
			return child;
		}
		Node* GetChildByName(const std::string& name);
		Node* GetChildNodeOfClass(const Class& cls, bool includeSelf = false);
		template<class T>
		T* GetChildNodeOfClass(bool includeSelf = false)
		{
			return GetChildNodeOfClass(T::StaticClass(), includeSelf)->As<T>();
		}
		void GetChildNodesOfClass(const Class& cls, Array<Node*>& OutArray, bool includeSelf = false);
		template<class T>
		Array<T*> GetChildNodesOfClass(bool includeSelf = false)
		{
			Array<T*> out;
			Array<Node*> nodes;
			GetChildNodesOfClass(T::StaticClass(), nodes, includeSelf);

			for (auto& It : nodes)
			{
				out.Add(It->As<T>());
			}

			return out;
		}
		Node* GetParentNodeOfClass(const Class& cls, bool includeSelf = false);
		template<class T>
		T* GetParentNodeOfClass(bool includeSelf = false)
		{
			return GetParentNodeOfClass(T::StaticClass(), includeSelf)->As<T>();
		}
		Node* IsARecursive(const Class& cls)
		{
			return GetParentNodeOfClass(cls, true);
		}
		template<class T>
		T* IsARecursive()
		{
			return IsARecursive(T::StaticClass())->As<T>();
		}

	protected:
		virtual void InitializeNode(World& world);
	public:
		bool IsInitialized() const;
		bool IsEnabled() const;

		void SetEnabled(bool enabled);

		FUNCTION(Callable, Pure)
		virtual Node3D* GetTransform();

		FUNCTION(Callable, Pure)
		virtual Node3D* GetParentTransform();
	protected:
		virtual void TickTransform(bool inWorldSpace = true);
		virtual void OnParentChange(Node* prev, Node* next);

		/** Always sets the Parent of the node, even for Components
		*   If inGameplayContext is set to 'true', Events will fire and Gameplay code might execute.... */
		void ForceSetParent(Node* parent, bool keepWorldTransform, bool inGameplayContext);
	public:
		FUNCTION(Callable)
		virtual void SetParent(Node* parent, bool keepWorldTransform = true);

		FUNCTION(Callable, Pure)
		Node* GetParent() const
		{
			return m_Parent;
		}
		FUNCTION(Callable, Pure)
		bool HasChildren() const
		{
			return m_Children.Size() > 0;
		}
		FUNCTION(Callable, Pure)
		int GetChildCount() const
		{
			return m_Children.Size();
		}
		FUNCTION(Callable, Pure)
		Node* GetChild(int index) const
		{
			if (index >= m_Children.Size()) return nullptr;
			return m_Children[index];
		}
		FUNCTION(Callable, Pure)
		bool IsChildOf(Node* other) const
		{
			return GetParent() ? (GetParent() == other ? true : GetParent()->IsChildOf(other)) : false;
		}
		void DetachAllChildren(Node* target = nullptr)
		{
			for (int i = m_Children.Size() - 1; i >= 0; i--)
			{
				m_Children[i]->SetParent(target);
			}
		}

		/** Serialization & Deserialization */
		void Serialize(Yaml::Node& root);
		static Node* Deserialize(Yaml::Node& root, bool includeCompositionData, Node* liveNode = nullptr, bool deepestLayer = false, bool editContext = false);
		Class GetCompositeClass();
		Node* GetCompositionNode();

		friend class Node3D;
		friend class World;
		friend class GameInstance;
		friend class Level;
		friend class DetailsPanel;
		friend class LevelOutliner;
		friend class NodeClassEditor;
	};


	/** Node3D is a Node with a 3-Dimentional Transform */
	class Node3D : public Node
	{
		SUORA_CLASS(4863437);
	private:
		glm::mat4 m_WorldTransformMatrix = glm::mat4(1);
		glm::mat4 m_LocalTransformMatrix = glm::mat4(1);
	public:
		Node3D();
		virtual ~Node3D();
		Node3D* GetTransform() override
		{
			return this;
		}


		FUNCTION(Callable)
		void SetPosition(const Vec3& position);
		FUNCTION(Callable)
		void AddWorldOffset(const Vec3& offset);
		FUNCTION(Callable, Pure)
		Vec3 GetPosition() const;
		FUNCTION(Callable, Pure)
		Vec3 GetLocalPosition() const;
		void SetLocalPosition(const Vec3& position);

		Quat GetRotation() const;
		Quat GetLocalRotation() const;
		Vec3 GetEulerRotation() const;
		Vec3 GetLocalEulerRotation() const;
		void SetRotation(const Quat& rot);
		void SetLocalRotation(const Quat& rot);
		void SetEulerRotation(const Vec3& eulerAngles);
		void SetLocalEulerRotation(const Vec3& eulerAngles);

		//FUNCTION(Callable)
		void Rotate(const Quat& quat);
		FUNCTION(Callable)
		void RotateEuler(const Vec3& eulerAngles);
		FUNCTION(Callable)
		void RotateWithAxis(const Vec3& axis, float angle);

		void SetLookDirection(const Vec3& direction, const Vec3& up = Vec::Up);
		Vec3 GetScale() const;
		Vec3 GetLocalScale() const;
		void SetScale(const Vec3& scale);
		void SetLocalScale(const Vec3& scale);

		glm::mat4 GetTransformMatrix() const;
		void SetTransformMatrix(const glm::mat4& mat)
		{
			m_WorldTransformMatrix = mat;
			RecalculateTransformMatrix();
			TickTransform(true);
		}
		void RecalculateTransformMatrix();
		void ReprojectLocalMatrixToWorld();
		static glm::mat4 CalculateTransformMatrix(const glm::vec3& position, const glm::vec3& eulerAngles, const glm::vec3& scale);
		static glm::mat4 CalculateTransformMatrix(const glm::vec3& position, const glm::quat& rotation, const glm::vec3& scale);

	protected:
		void TickTransform(bool inWorldSpace = false) override;

		virtual void InitializeNode(World& world) override;

	private:

		void DecomposeTransformMatrix()
		{
			/*
			Vec3 scale;
			glm::quat rotation;
			Vec3 translation;
			glm::vec3 skew;
			glm::vec4 perspective;
			glm::decompose(GetTransformMatrix(), scale, rotation, translation, skew, perspective);
			m_Position = translation;
			m_Rotation = glm::degrees(glm::eulerAngles(glm::conjugate(rotation)));
			m_Scale = scale;
			*/
		}
	public:
		FUNCTION(Callable, Pure)
		Vec3 GetRightVector() const;
		FUNCTION(Callable, Pure)
		Vec3 GetUpVector() const;
		FUNCTION(Callable, Pure)
		Vec3 GetForwardVector() const;

		friend class Node;
		friend class DetailsPanel;
		friend class ViewportPanel;
		friend class Physics::PhysicsWorld;
	};

	/** UINode is a Node with a UserInterface ready Transform */
	class UINode : public Node
	{
		SUORA_CLASS(8569225);
	public:
	};

	/** A special Node that cannot do anything by itself, but always alters its Parent; Cannot be unparented or spawned! */
	class Component : public Node
	{
		SUORA_CLASS(678534121);
	public:
		void Begin() override;
		virtual void SetParent(Node* parent, bool keepWorldTransform = true) override;
	};

	/** Rootclass of all Levels in the Engine. Is ignored if Node::GetRootNode() was called. */
	class LevelNode : public Node
	{
		SUORA_CLASS(1863227);
	public:
	};

}

#define REQUIRE_NODE(Class) if (!GetParent() || !GetParent()->IsA<Class>()) return;