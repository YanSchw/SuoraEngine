#pragma once
#include <future>
#include "Suora/Common/Array.h"
#include "Suora/Assets/Blueprint.h"
#include "Suora/Core/Update.h"
#include "Node.h"
#include "World.generated.h"

namespace Suora::Physics
{
	class PhysicsWorld;
}

namespace Suora
{
	class Engine;
	class CameraNode;
	class Node;
	class Level;
	class Ilum;
	class GameInstance;
	
	struct HitResult
	{
		Vec3 Normal;
		Vec3 Point;
		class CollisionNode* Collider = nullptr;
		class PhysicsNode* PhysicsBody = nullptr;
	};
	struct RaycastParams
	{
		Array<class CollisionNode*> IgnoredCollisionNodes;
	};

	struct LocalUpdateChunk
	{
		std::thread m_Thread;
		Array<Node*> m_Nodes;
	};

	/** Container for all Nodes during Gameplay */
	class World : public Object
	{
		SUORA_CLASS(9867384674);

	private:
		GameInstance* m_GameInstance = nullptr;
		Array<Node*> m_WorldNodes;

		/** Array of all nodes, that need to have Begin() called. */
		Array<Ptr<Node>> m_BeginPlayIssues;

		/** Array of all nodes, that will be killed at the end of the frame. 
		*   Kill happens outside of Gameplay Context.                        */
		Array<Node*> m_PendingKills;

		Ptr<Node> m_Pawn = nullptr;
		Ptr<CameraNode> m_MainCamera;
		Level* m_SourceLevel = nullptr;
		Ref<Physics::PhysicsWorld> m_PhysicsWorld;
		Ref<Ilum> m_IlumContext;

	public:
		World();
		~World();

		void Update(float deltaTime);

		float GetDeltaTime() const;
		Level* GetSourceLevel() const;
		Physics::PhysicsWorld* GetPhysicsWorld();
		GameInstance* GetGameInstance() const;
		Ilum* GetIlumContext() const;

		Node* Spawn(const Class& cls);
		Node* Spawn(const Class& cls, const Vec3& position, const Quat& rotation);

		template<class T>
		T* Spawn()
		{
			return Spawn(T::StaticClass())->As<T>();
		}

		bool Raycast(const Vec3& start, const Vec3& end, HitResult& result, const RaycastParams& params = RaycastParams());

		void SetPawn(Node* pawn);
		Node* GetPlayerPawn() const;
		void SetMainCamera(CameraNode* camera);
		CameraNode* GetMainCamera() const;

		Array<Node*> GetAllNodes() const;

		Array<Node*> FindNodesByClass(const Class& cls);
		template<class T>
		Array<T*> FindNodesByClass()
		{
			Array<T*> nodes;
			for (Node* node : m_WorldNodes)
			{
				if (node->IsA<T>())
				{
					nodes.Add(node->As<T>());
				}
			}
			return nodes;
		}

	private:
		void UnregisterNode(Node* node);
		void ReregisterNode(Node* node);

		Array<Node*> m_WorldUpdateNodes;
		Array<Node*> m_LocalUpdateNodes;

		void ResolveAllBeginPlayIssues();
		void ResolvePendingKills();

		void WorldUpdate(float deltaTime);
		void LocalUpdate(float deltaTime, LocalUpdateChunk* chunk);
		void PrepareLocalUpdate();

		std::thread m_WorldUpdateThread;
		std::thread m_PrepareLocalUpdateThread;
		Array<LocalUpdateChunk> m_LocalUpdateChunks;

		float m_DeltaTime = 0.0f;

		friend class Engine;
		friend class Level;
		friend class Node;
		friend class GameInstance;
		friend class ViewportPanel;
		friend class NodeClassEditor;
		friend class CameraNode;
		friend class RenderPipeline;
	};
}