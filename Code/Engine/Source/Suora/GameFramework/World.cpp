#include "Precompiled.h"
#include "World.h"
#include "Suora/GameFramework/Node.h"
#include "New.h"
#include "Suora/Renderer/Ilum.h"
#include "Suora/Physics/PhysicsEngine.h"

#define LOCAL_UPDATE_CHUNKS_PER_THREAD 128

namespace Suora
{

	World::World()
	{
		m_IlumContext = Ref<Ilum>(new Ilum(this));
		m_PhysicsWorld = Ref<Physics::PhysicsWorld>(new Physics::PhysicsWorld());
	}

	World::~World()
	{
		for (Node* node : m_WorldNodes)
		{
			node->ForceSetParent(nullptr, true, false);
		}
		while (m_WorldNodes.Size() > 0)
		{
			SuoraError(m_WorldNodes[0]->GetName());
			delete m_WorldNodes[0];
		}
	}

	Node* World::Spawn(const Class& cls)
	{
		LOCAL_UPDATE_LOCK();
		SuoraAssert(!cls.Inherits(Component::StaticClass()), "A RootNode may not be a Component!");

		Node* node = Cast<Node>(New(cls, false, false));
		SuoraVerify(node);

		node->Implement<IObjectCompositionData>();
		node->GetInterface<IObjectCompositionData>()->m_IsActorLayer = true;
		node->MakeAllChildrenPuppets();

		node->InitializeNode(*this);

		return node;
	}

	Node* World::Spawn(const Class& cls, const Vec3& position, const Quat& rotation)
	{
		Node* node = Spawn(cls);

		if (Node3D* node3D = node->As<Node3D>())
		{
			node3D->SetPosition(position);
			node3D->SetRotation(rotation);
		}

		return node;
	}

	float World::GetDeltaTime() const
	{
		return m_DeltaTime;
	}
	Level* World::GetSourceLevel() const
	{
		return m_SourceLevel;
	}
	Physics::PhysicsWorld* World::GetPhysicsWorld() const
	{
		return m_PhysicsWorld.get();
	}
	GameInstance* World::GetGameInstance() const
	{
		return m_GameInstance;
	}

	Ilum* World::GetIlumContext() const
	{
		return m_IlumContext.get();
	}

	void World::SetPawn(Node* pawn)
	{
		m_Pawn = pawn;
		m_Pawn->OnPossess();
	}
	Node* World::GetPlayerPawn() const
	{
		return m_Pawn;
	}
	void World::SetMainCamera(CameraNode* camera)
	{
		m_MainCamera = camera;
	}
	CameraNode* World::GetMainCamera() const
	{
		return m_MainCamera.Get();
	}

	Array<Node*> World::GetAllNodes() const
	{
		return m_WorldNodes;
	}

	bool World::Raycast(const Vec3& start, const Vec3& end, HitResult& result, const RaycastParams& params)
	{
		return GetPhysicsWorld()->Raycast(start, end, result, params);
	}

	void World::Update(float deltaTime)
	{
		m_DeltaTime = deltaTime;

		ResolveAllBeginPlayIssues();

		GetPhysicsWorld()->Update(deltaTime);

		// Node::PawnUpdate()
		if (m_Pawn)
		{
			m_Pawn->PawnUpdate(deltaTime);
		}

		// WorldUpdate + PrepareLocalUpdate
		/*m_WorldUpdateThread = std::thread(&World::WorldUpdate, this, deltaTime);
		m_PrepareLocalUpdateThread = std::thread(&World::PrepareLocalUpdate, this);

		m_WorldUpdateThread.join();
		m_PrepareLocalUpdateThread.join();*/
		WorldUpdate(deltaTime);
		PrepareLocalUpdate();

		// LocalUpdate
		UpdateRules::s_LocalUpdate = true;
		for (LocalUpdateChunk& chunk : m_LocalUpdateChunks)
		{
			chunk.m_Thread = std::thread(&World::LocalUpdate, this, deltaTime, &chunk);
		}
		for (LocalUpdateChunk& chunk : m_LocalUpdateChunks)
		{
			chunk.m_Thread.join();
		}
		UpdateRules::s_LocalUpdate = false;

		ResolvePendingKills();
	}

	Array<Node*> World::FindNodesByClass(const Class& cls)
	{
		Array<Node*> nodes;
		for (Node* node : m_WorldNodes)
		{
			if (node->IsA(cls))
			{
				nodes.Add(node);
			}
		}
		return nodes;
	}

	void World::UnregisterNode(Node* node)
	{
		if (m_WorldNodes.Contains(node)) m_WorldNodes.Remove(node);
		if (node->IsUpdateFlagSet(UpdateFlag::WorldUpdate) && m_WorldUpdateNodes.Contains(node)) m_WorldUpdateNodes[m_WorldUpdateNodes.IndexOf(node)] = nullptr;
		if (node->IsUpdateFlagSet(UpdateFlag::LocalUpdate) && m_LocalUpdateNodes.Contains(node)) m_LocalUpdateNodes.Remove(node);
	}
	void World::ReregisterNode(Node* node)
	{
		if (node->IsUpdateFlagSet(UpdateFlag::WorldUpdate) && !m_WorldUpdateNodes.Contains(node)) m_WorldUpdateNodes.Add(node);
		if (node->IsUpdateFlagSet(UpdateFlag::LocalUpdate) && !m_LocalUpdateNodes.Contains(node)) m_LocalUpdateNodes.Add(node);
	}

	void World::ResolveAllBeginPlayIssues()
	{
		while (m_BeginPlayIssues.Size() > 0)
		{
			if (m_BeginPlayIssues[0] != nullptr)
			{
				if (!m_BeginPlayIssues[0]->m_WasBeginCalled)
				{
					m_BeginPlayIssues[0]->m_WasBeginCalled = true;
					m_BeginPlayIssues[0]->Begin();
				}

				// Push UpdateFlags
				ReregisterNode(m_BeginPlayIssues[0]);
			}
			m_BeginPlayIssues.RemoveAt(0);
		}
	}

	void World::ResolvePendingKills()
	{
		// Kill all Nodes in m_PendingKills
		for (Node* node : m_PendingKills)
		{
			delete node;
		}
		m_PendingKills.Clear();
	}

	void World::WorldUpdate(float deltaTime)
	{
		for (int64_t i = m_WorldUpdateNodes.Last(); i >= 0; i--)
		{
			if (m_WorldUpdateNodes[i])
			{
				if (m_WorldUpdateNodes[i]->ShouldUpdateInCurrentContext())
				{
					m_WorldUpdateNodes[i]->WorldUpdate(deltaTime);
				}
			}
			else
			{
				m_WorldUpdateNodes.RemoveAt(i);
			}
		}
	}
	void World::LocalUpdate(float deltaTime, LocalUpdateChunk* chunk)
	{
		SUORA_ASSERT(chunk, "LocalUpdateChunk is invalid!");

		for (Node* node : chunk->m_Nodes)
		{
			if (node->ShouldUpdateInCurrentContext())
			{
				node->LocalUpdate(deltaTime);
			}
		}
	}
	void World::PrepareLocalUpdate()
	{
		m_LocalUpdateChunks.Clear();
	}
}