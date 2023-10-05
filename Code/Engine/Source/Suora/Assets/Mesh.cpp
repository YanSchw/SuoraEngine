#include "Precompiled.h"
#include "Mesh.h"
#include "Suora/Common/Common.h"
#include "Suora/Common/Random.h"
#include "Suora/Common/Filesystem.h"
#include "Suora/Serialization/Yaml.h"
#include "Suora/Renderer/VertexArray.h"
#include "Suora/Renderer/Vertex.h"
#include "Suora/Renderer/Decima.h"
#include "Suora/Core/Threading.h"
#include "Suora/Assets/AssetManager.h"
#include <fstream>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

namespace Suora
{
	
	// Primitive Shapes
	Mesh* Mesh::Quad = AssetManager::GetAsset<Mesh>(SuoraID("75f466f7-baec-4c5a-a23b-a5e3dc3d22bc"));
	Mesh* Mesh::Plane = AssetManager::GetAsset<Mesh>(SuoraID("45d16e2b-822e-44bf-a686-abde5552f67a"));
	Mesh* Mesh::Cube = AssetManager::GetAsset<Mesh>(SuoraID("33b79a6d-2f4a-40fc-93e5-3f01794c33b8"));
	Mesh* Mesh::Sphere = AssetManager::GetAsset<Mesh>(SuoraID("5c43e991-86be-48a4-8b14-39d275818ec1"));

	Mesh::Mesh()
	{
		m_Materials.Owner = this;
	}

	Mesh::~Mesh()
	{
		if (m_AsyncMeshBuffer.get())
		{
			// TextureBuffer_stbi has to be destroyed here (!)
			// We also wait for the async load task to finish
			m_AsyncMeshBuffer->get();
		}
		if (m_VertexArray)
		{
			m_VertexArray = nullptr;
		}
	}

	void Mesh::PreInitializeAsset(const std::string& str)
	{
		Super::PreInitializeAsset(str);

		Yaml::Node root;
		Yaml::Parse(root, str);
		m_UUID = root["UUID"].As<std::string>();
		m_IsDecimaMesh = root["Mesh"]["m_IsDecimaMesh"].As<std::string>() == "true";
		m_BoundingSphereRadius = root["Mesh"]["m_BoundingSphereRadius"].As<float>();
		m_NegativeY_Bounds = root["Mesh"]["m_NegativeY_Bounds"].As<float>();

	}

	void Mesh::InitializeAsset(const std::string& str)
	{
		Super::InitializeAsset(str);

		Yaml::Node root;
		Yaml::Parse(root, str);
		Yaml::Node& mesh = root["Mesh"];

		// Materials
		{
			m_Materials.OverwritteMaterials = mesh["Materials"]["Overwrite"].As<std::string>() == "true";
			m_Materials.Materials.Clear();
			int i = 0;
			while (true)
			{
				Yaml::Node& material = mesh["Materials"][std::to_string(i)];
				if (material.IsNone()) break;
				m_Materials.Materials.Add((material.As<std::string>() != "0")
					? AssetManager::GetAsset<Material>(material.As<std::string>())
					: nullptr);
				i++;
			}
		}
	}

	void Mesh::ReloadAsset()
	{
		Super::ReloadAsset();

		m_VertexArray = nullptr; 
		m_MeshBuffer = MeshBuffer();
		m_MainCluster = nullptr;
	}

	uint32_t Mesh::GetAssetFileSize()
	{
		uint32_t baseSize = Super::GetAssetFileSize();
		uint32_t meshSize = 0;

		if (IsSourceAssetPathValid())
		{
			meshSize = std::filesystem::file_size(GetSourceAssetPath());
		}

		return baseSize + meshSize;
	}
	
	MeshBuffer Mesh::Decimate(MeshBuffer buffer, uint32_t N)
	{
		if (buffer.Indices.size() == 0)
		{
			SuoraError("Mesh::Decimate(): cannot decimate Mesh!");
			return MeshBuffer();
		}
		SuoraError("Before {0}", buffer.Indices.size() / 3);

		int64_t It = 54321;
		for (uint32_t i = 0; i < N; i++)
		{
			It += buffer.Indices.size() * buffer.Vertices.size();
			It %= buffer.Indices.size();
			uint32_t VertexID = buffer.Indices[It];
			std::vector<uint32_t> HoleVertices;

			// Remove Vertex, and therefore all associated Tris, "harmed" still-alive vertices are collected in <HoleVertices>.
			for (int64_t j = buffer.Indices.size() - 3; j >= 0; j -= 3)
			{
				if (buffer.Vertices[buffer.Indices[j]] == buffer.Vertices[VertexID])
				{
					/*if (std::find(HoleVertices.begin(), HoleVertices.end(), buffer.Indices[j - 1]) != HoleVertices.end())*/ HoleVertices.push_back(buffer.Indices[j + 1]);
					/*if (std::find(HoleVertices.begin(), HoleVertices.end(), buffer.Indices[j - 2]) != HoleVertices.end())*/ HoleVertices.push_back(buffer.Indices[j + 2]);
					buffer.Indices.erase(buffer.Indices.begin() + j, buffer.Indices.begin() + j + 3);
				}
				else if (buffer.Vertices[buffer.Indices[j + 1]] == buffer.Vertices[VertexID])
				{
					/*if (std::find(HoleVertices.begin(), HoleVertices.end(), buffer.Indices[j - 0]) != HoleVertices.end())*/ HoleVertices.push_back(buffer.Indices[j + 0]);
					/*if (std::find(HoleVertices.begin(), HoleVertices.end(), buffer.Indices[j - 2]) != HoleVertices.end())*/ HoleVertices.push_back(buffer.Indices[j + 2]);
					buffer.Indices.erase(buffer.Indices.begin() + j, buffer.Indices.begin() + j + 3);
				}
				else if (buffer.Vertices[buffer.Indices[j + 2]] == buffer.Vertices[VertexID])
				{
					/*if (std::find(HoleVertices.begin(), HoleVertices.end(), buffer.Indices[j - 0]) != HoleVertices.end())*/ HoleVertices.push_back(buffer.Indices[j + 0]);
					/*if (std::find(HoleVertices.begin(), HoleVertices.end(), buffer.Indices[j - 1]) != HoleVertices.end())*/ HoleVertices.push_back(buffer.Indices[j + 1]);
					buffer.Indices.erase(buffer.Indices.begin() + j, buffer.Indices.begin() + j + 3);
				}
			}

			// Fill the hole.
			while (HoleVertices.size() >= 3 * 2) // TODO: Fix
			{
				uint32_t A = HoleVertices[0];
				uint32_t B = HoleVertices[1];
				for (int64_t j = 2; j < HoleVertices.size(); j += 2)
				{
					if (buffer.Vertices[HoleVertices[j]] == buffer.Vertices[A] || buffer.Vertices[HoleVertices[j]] == buffer.Vertices[B] || buffer.Vertices[HoleVertices[j + 1]] == buffer.Vertices[A] || buffer.Vertices[HoleVertices[j + 1]] == buffer.Vertices[B])
					{
						buffer.Indices.push_back((buffer.Vertices[HoleVertices[j]] != buffer.Vertices[A] && buffer.Vertices[HoleVertices[j]] != buffer.Vertices[B]) ? HoleVertices[j] : HoleVertices[j + 1]);
						buffer.Indices.push_back(A);
						buffer.Indices.push_back(B);

						std::vector<int64_t> v;
						v.push_back(A);
						v.push_back(B);
						v.push_back(HoleVertices[j + 0]);
						v.push_back(HoleVertices[j + 1]);

						HoleVertices.erase(HoleVertices.begin() + j, HoleVertices.begin() + j + 2);
						HoleVertices.erase(HoleVertices.begin(), HoleVertices.begin() + 2);

						// Remove duplicate Vertex
						{
							if (buffer.Vertices[v[0]] == buffer.Vertices[v[2]])
							{
								v.erase(v.begin() + 2);
								v.erase(v.begin() + 0);
							}
							else if (buffer.Vertices[v[0]] == buffer.Vertices[v[3]])
							{
								v.erase(v.begin() + 3);
								v.erase(v.begin() + 0);
							}
							else if (buffer.Vertices[v[1]] == buffer.Vertices[v[2]])
							{
								v.erase(v.begin() + 2);
								v.erase(v.begin() + 1);
							}
							else if (buffer.Vertices[v[1]] == buffer.Vertices[v[3]])
							{
								v.erase(v.begin() + 3);
								v.erase(v.begin() + 1);
							}
						}

						for (int64_t remain : v)
						{
							HoleVertices.push_back(remain);
						}

						goto _break;
					}
				}
				HoleVertices.push_back(A);
				HoleVertices.push_back(B);
				HoleVertices.erase(HoleVertices.begin(), HoleVertices.begin() + 2);

			_break:;
			}
		}

		SuoraError("After {0}", buffer.Indices.size() / 3);

		return buffer;
	}

	void Mesh::Decimate_Edge(MeshBuffer& buffer, uint32_t N)
	{
		if (buffer.Indices.size() == 0)
		{
			SuoraError("Mesh::Decimate(): cannot decimate Mesh!");
			return;
		}

		int64_t It = 54321;
		for (uint32_t i = 0; i < N; i++)
		{
			if (buffer.Indices.size() <= 6) return;
			It += buffer.Indices.size() * buffer.Vertices.size() + 123;
			It %= buffer.Indices.size() / 3;
			It *= 3;
			uint32_t VertexU = buffer.Indices[It];		
			uint32_t VertexV = buffer.Indices[It + 1];	

			float shortestDist = 1.0f;
			for (int64_t j = 0; j < buffer.Indices.size(); j += 3)
			{
				Vertex& A = buffer.Vertices[buffer.Indices[j + 0]];
				Vertex& B = buffer.Vertices[buffer.Indices[j + 1]];
				Vertex& C = buffer.Vertices[buffer.Indices[j + 2]];

				if (glm::distance(A.Position, B.Position) < shortestDist) { VertexU = buffer.Indices[j + 0]; VertexV = buffer.Indices[j + 1]; shortestDist = glm::distance(A.Position, B.Position); }
				if (glm::distance(B.Position, C.Position) < shortestDist) { VertexU = buffer.Indices[j + 1]; VertexV = buffer.Indices[j + 2]; shortestDist = glm::distance(B.Position, C.Position); }
				if (glm::distance(C.Position, A.Position) < shortestDist) { VertexU = buffer.Indices[j + 2]; VertexV = buffer.Indices[j + 0]; shortestDist = glm::distance(C.Position, A.Position); }
			}

			const Vertex& U = buffer.Vertices[VertexU];
			const Vertex& V = buffer.Vertices[VertexV];

			//std::vector<int64_t> CollapsedTris;
			for (int64_t j = 0; j < buffer.Indices.size(); j += 3)
			{
				const Vertex& A = buffer.Vertices[buffer.Indices[j + 0]];
				const Vertex& B = buffer.Vertices[buffer.Indices[j + 1]];
				const Vertex& C = buffer.Vertices[buffer.Indices[j + 2]];

				// Remove Tris on edge.
				if ((U != V) && (A == U || B == U || C == U) && (A == V || B == V || C == V))
				{
					buffer.Indices.erase(buffer.Indices.begin() + j, buffer.Indices.begin() + j + 3);
					j -= 3;
					continue;
				}
				else if ((U != V) && ((A == U || B == U || C == U) || (A == V || B == V || C == V)))
				{
					{
						if (A == U || A == V) buffer.Indices[j + 0] = VertexU;/*buffer.Vertices.size() - 0;*/ else
						if (B == U || B == V) buffer.Indices[j + 1] = VertexU;/*buffer.Vertices.size() - 0;*/ else
						if (C == U || C == V) buffer.Indices[j + 2] = VertexU;/*buffer.Vertices.size() - 0;*/
					}
					//CollapsedTris.push_back(j);
				}

			}
			//buffer.Vertices.push_back(Vertex::Edge(U, V));

		}


		return;
	}

	static int CheckSharedTris(MeshBuffer& buffer, int u, int v)
	{
		Vertex& U = buffer.Vertices[buffer.Indices[u]];
		Vertex& V = buffer.Vertices[buffer.Indices[v]];
		int sharedTris = 0;

		for (int64_t i = 0; i < buffer.Indices.size(); i += 3)
		{
			Vertex& A = buffer.Vertices[buffer.Indices[i + 0]];
			Vertex& B = buffer.Vertices[buffer.Indices[i + 1]];
			Vertex& C = buffer.Vertices[buffer.Indices[i + 2]];
			if ((U != V) && (A == U || B == U || C == U) && (A == V || B == V || C == V))
				sharedTris++;
		}

		return sharedTris;
	}

	void Mesh::Decimate_ClusterEdge(MeshBuffer& buffer)
	{
		if (buffer.Indices.size() == 0)
		{
			SuoraError("Mesh::Decimate(): cannot decimate Mesh!");
			return;
		}

		Array<Vec3> EdgeVertexPositions;
		for (int64_t i = 0; i < buffer.Indices.size(); i += 3)
		{
			if (CheckSharedTris(buffer, i + 0, i + 1) == 1)
			{
				EdgeVertexPositions.Add(buffer.Vertices[buffer.Indices[i + 0]].Position);
				EdgeVertexPositions.Add(buffer.Vertices[buffer.Indices[i + 1]].Position);
			}
			if (CheckSharedTris(buffer, i + 1, i + 2) == 1)
			{
				EdgeVertexPositions.Add(buffer.Vertices[buffer.Indices[i + 1]].Position);
				EdgeVertexPositions.Add(buffer.Vertices[buffer.Indices[i + 2]].Position);
			}
			if (CheckSharedTris(buffer, i + 0, i + 2) == 1)
			{
				EdgeVertexPositions.Add(buffer.Vertices[buffer.Indices[i + 0]].Position);
				EdgeVertexPositions.Add(buffer.Vertices[buffer.Indices[i + 2]].Position);
			}
		}

		int64_t It = 54321;
		bool bDone = false;
		float minDist = 0.0f;
		while (!bDone && buffer.Indices.size() / 3 > Decima::s_TrianglesPerCluster)
		{
			bDone = true;
			if (buffer.Indices.size() <= 6) return;
			It += buffer.Indices.size() * buffer.Vertices.size() + 123;
			It %= buffer.Indices.size() / 3;
			It *= 3;
			uint32_t VertexU = buffer.Indices[It];
			uint32_t VertexV = buffer.Indices[It + 1];

			float shortestDist = 999999999.0f;
			bool EdgeFound = false;
			for (int64_t j = 0; j < buffer.Indices.size(); j += 3)
			{
				Vertex& A = buffer.Vertices[buffer.Indices[j + 0]];
				Vertex& B = buffer.Vertices[buffer.Indices[j + 1]];
				Vertex& C = buffer.Vertices[buffer.Indices[j + 2]];

				if ((!EdgeVertexPositions.Contains(A.Position) || !EdgeVertexPositions.Contains(B.Position)) && glm::distance(A.Position, B.Position) <= shortestDist && CheckSharedTris(buffer, j + 0, j + 1) == 2) { VertexU = buffer.Indices[j + 0]; VertexV = buffer.Indices[j + 1]; shortestDist = glm::distance(A.Position, B.Position); EdgeFound = true; }
				if ((!EdgeVertexPositions.Contains(B.Position) || !EdgeVertexPositions.Contains(C.Position)) && glm::distance(B.Position, C.Position) <= shortestDist && CheckSharedTris(buffer, j + 1, j + 2) == 2) { VertexU = buffer.Indices[j + 1]; VertexV = buffer.Indices[j + 2]; shortestDist = glm::distance(B.Position, C.Position); EdgeFound = true; }
				if ((!EdgeVertexPositions.Contains(C.Position) || !EdgeVertexPositions.Contains(A.Position)) && glm::distance(C.Position, A.Position) <= shortestDist && CheckSharedTris(buffer, j + 2, j + 0) == 2) { VertexU = buffer.Indices[j + 2]; VertexV = buffer.Indices[j + 0]; shortestDist = glm::distance(C.Position, A.Position); EdgeFound = true; }
			}
			if (!EdgeFound) break;

			const Vertex& U = buffer.Vertices[VertexU];
			const Vertex& V = buffer.Vertices[VertexV];

			bool ScoreU = true;
			if (EdgeVertexPositions.Contains(U.Position))
			{
				ScoreU = false;
				/*if (EdgeVertexPositions.Contains(V.Position))
				{
					minDist = shortestDist;
					bDone = false;
					continue;
				}*/
			}
			minDist = 0.0f;

			for (int64_t j = 0; j < buffer.Indices.size(); j += 3)
			{
				const Vertex& A = buffer.Vertices[buffer.Indices[j + 0]];
				const Vertex& B = buffer.Vertices[buffer.Indices[j + 1]];
				const Vertex& C = buffer.Vertices[buffer.Indices[j + 2]];

				// Remove Tris on edge.
				if ((U != V) && (A == U || B == U || C == U) && (A == V || B == V || C == V))
				{
					bDone = false;
					buffer.Indices.erase(buffer.Indices.begin() + j, buffer.Indices.begin() + j + 3);
					j -= 3;
				}
				else if ((U != V) && ((A == U || B == U || C == U) || (A == V || B == V || C == V)))
				{
					bDone = false;
					{
						if (ScoreU)
						{
							if (A == U) buffer.Indices[j + 0] = VertexV;
							if (B == U) buffer.Indices[j + 1] = VertexV;
							if (C == U) buffer.Indices[j + 2] = VertexV;
						}
						else
						{
							if (A == V) buffer.Indices[j + 0] = VertexU;
							if (B == V) buffer.Indices[j + 1] = VertexU;
							if (C == V) buffer.Indices[j + 2] = VertexU;
						}
					}
				}

			}
		}

		return;
	}

	void Mesh::Decimate_Cluster(MeshBuffer& meshBuffer, Ref<Cluster> cluster)
	{
		MeshBuffer buffer;
		buffer.Indices = cluster->Indices;
		buffer.Vertices = meshBuffer.Vertices;
		Decimate_ClusterEdge(buffer);
		cluster->Indices = buffer.Indices;
	}

	VertexArray* Mesh::GetVertexArray()
	{
		if (IsMissing() || !IsSourceAssetPathValid() || IsMasterMesh())
		{
			return nullptr;
		}

		if (!m_VertexArray)
		{
			if (!m_AsyncMeshBuffer.get() && AssetManager::s_AssetStreamPool.Size() < ASSET_STREAM_COUNT_LIMIT)
			{
				AssetManager::s_AssetStreamPool.Add(this);
				std::string filePath = GetSourceAssetPath().string();
				
				m_AsyncMeshBuffer = CreateRef<std::future<Ref<MeshBuffer>>>(std::async(std::launch::async, &Mesh::Async_LoadMeshBuffer, this, filePath, m_MeshBuffer.Vertices, m_MeshBuffer.Indices));
			}
			else if (m_AsyncMeshBuffer.get() && IsFutureReady(*m_AsyncMeshBuffer.get()))
			{
				AssetManager::s_AssetStreamPool.Remove(this);
				if (IsSubMesh() && AssetManager::s_AssetStreamPool.Contains(m_ParentMesh)) AssetManager::s_AssetStreamPool.Remove(m_ParentMesh);
				Ref<MeshBuffer> buffer = m_AsyncMeshBuffer->get();
				if (m_MeshBuffer.Indices.size() == 0) m_MeshBuffer = *buffer.get();
				m_AsyncMeshBuffer = nullptr;
				if (!IsMasterMesh()) m_VertexArray = Ref<VertexArray>(VertexArray::Create(IsDecimaMesh() ? MeshBuffer(m_MeshBuffer.Vertices, m_MainCluster->Indices) : m_MeshBuffer));
			}
		}

		return m_VertexArray.get();
	}

	Ref<MeshBuffer> Mesh::Async_LoadMeshBuffer(const std::string& path, const std::vector<Vertex>& v, const std::vector<uint32_t>& i)
	{
		Ref<MeshBuffer> buffer = CreateRef<MeshBuffer>(v, i);

		// read file via ASSIMP
		Assimp::Importer importer;
		const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_FixInfacingNormals/*| aiProcess_FlipUVs*/ | aiProcess_CalcTangentSpace | aiProcess_JoinIdenticalVertices);
		// check for errors
		if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) // if is Not Zero
		{
			SuoraError("ASSIMP: {0}", importer.GetErrorString());
			return buffer;
		}
		if (scene->mNumMeshes <= 0)
		{
			SuoraError("ASSIMP: No Mesh in File.");
			return buffer;
		}
		if (!IsSubMesh() && scene->mNumMeshes > 1)
		{
			// Read Submeshes
			m_IsMasterMesh = true;
			for (int i = 0; i < scene->mNumMeshes; i++)
			{
				Ref<Mesh> submesh = Ref<Mesh>(new Mesh());
				submesh->m_ParentMesh = this;
				submesh->m_SubmeshIndex = i;
				submesh->m_Path = m_Path;
				submesh->m_IsDecimaMesh = m_IsDecimaMesh;
				m_Submeshes.Add(submesh);
			}
			return buffer;
		}

		aiMesh* mesh = scene->mMeshes[!IsSubMesh() ? 0 : m_SubmeshIndex];

		// VERTICIES
		for (uint32_t i = 0; i < mesh->mNumVertices; i++)
		{
			Vertex vertex;
			glm::vec3 vector;
			vector.x = mesh->mVertices[i].x;
			vector.y = mesh->mVertices[i].y;
			vector.z = mesh->mVertices[i].z;
			vertex.Position = vector;
			if (mesh->HasNormals())
			{
				vector.x = mesh->mNormals[i].x;
				vector.y = mesh->mNormals[i].y;
				vector.z = mesh->mNormals[i].z;
				vertex.Normal = vector;
			}
			if (mesh->mTextureCoords[0])
			{
				glm::vec2 vec;
				vec.x = mesh->mTextureCoords[0][i].x;
				vec.y = mesh->mTextureCoords[0][i].y;
				vertex.TexCoord = vec;
				
				vector.x = mesh->mTangents[i].x;
				vector.y = mesh->mTangents[i].y;
				vector.z = mesh->mTangents[i].z;
				if (mesh->HasTangentsAndBitangents()) vertex.Tangent = vector;
				
				vector.x = mesh->mBitangents[i].x;
				vector.y = mesh->mBitangents[i].y;
				vector.z = mesh->mBitangents[i].z;
				if (mesh->HasTangentsAndBitangents()) vertex.Bitangent = vector;
			}
			else vertex.TexCoord = glm::vec2(0.0f, 0.0f);

			buffer->Vertices.push_back(vertex);
		}

		// INDICES
		for (uint32_t i = 0; i < mesh->mNumFaces; i++)
		{
			aiFace face = mesh->mFaces[i];
			if (!m_FlipNormals)
			{
				for (uint32_t j = 0; j < face.mNumIndices; j++)
				{
					buffer->Indices.push_back(face.mIndices[j]);
				}
			}
			else
			{
				for (int32_t j = face.mNumIndices - 1; j >= 0; j--)
				{
					buffer->Indices.push_back(face.mIndices[j]);
				}
			}
			
		}

		if (IsDecimaMesh() && !m_MainCluster) Clusterfication(*buffer.get());

		return buffer;
	}


	void Mesh::Serialize(Yaml::Node& root)
	{
		Super::Serialize(root);

		Yaml::Node& mesh = root["Mesh"];
		mesh["m_IsDecimaMesh"] = m_IsDecimaMesh ? "true" : "false";
		mesh["m_BoundingSphereRadius"] = std::to_string(m_BoundingSphereRadius);
		mesh["m_NegativeY_Bounds"] = std::to_string(m_NegativeY_Bounds);

		
		mesh["Materials"]["Overwrite"] = m_Materials.OverwritteMaterials ? "true" : "false";
		for (int i = 0; i < m_Materials.Materials.Size(); i++)
		{
			mesh["Materials"][std::to_string(i)] = (m_Materials.Materials[i]) ? m_Materials.Materials[i]->m_UUID.GetString() : "0";
		}
	}

	void Mesh::PostProcessCluster(MeshBuffer& buffer, Ref<Cluster> cluster, int32_t& id)
	{
		if (cluster->Child1) PostProcessCluster(buffer, cluster->Child1, id);
		if (cluster->Child2) PostProcessCluster(buffer, cluster->Child2, id);

		// LocalPos
		{
			Vec3 sum = (cluster->Indices.size() > 0) ? buffer.Vertices[cluster->Indices[0]].Position : Vec::Zero;
			for (int i = 1; i < cluster->Indices.size(); i++)
			{
				sum += buffer.Vertices[cluster->Indices[i]].Position;
			}
			cluster->LocalPosition = sum / (float)cluster->Indices.size();
		}

		// Normal
		{
			Vec3 sum = (cluster->Indices.size() > 0) ? buffer.Vertices[cluster->Indices[0]].Normal : Vec::Zero;
			for (int i = 1; i < cluster->Indices.size(); i++)
			{
				sum += buffer.Vertices[cluster->Indices[i]].Normal;
			}
			cluster->Normal = glm::normalize(sum);
		}

		// Radius
		for (int i = 0; i < cluster->Indices.size(); i++)
		{
			float dist = glm::distance(cluster->LocalPosition, buffer.Vertices[cluster->Indices[i]].Position);
			if (dist > cluster->ClusterRadius) cluster->ClusterRadius = dist;
		}

		// Create unique Vertices
		{
			for (int i = 1; i < cluster->Indices.size(); i++)
			{
				buffer.Vertices.push_back(buffer.Vertices[cluster->Indices[i]]);
				cluster->Indices[i] = buffer.Vertices.size() - 1;
			}
		}
		// ClusterIDs
		{
			for (int i = 1; i < cluster->Indices.size(); i++)
			{
				buffer.Vertices[cluster->Indices[i]].Cluster = id;
			}
			id++;
		}
	}

	void Mesh::GroupClusters(MeshBuffer& buffer, std::vector<Ref<Cluster>> clusters)
	{
		std::vector<Ref<Cluster>> Parents;

#define Join_Clusters(K) \
		Ref<Cluster> Parent = CreateRef<Cluster>();\
		Parent->Child1 = Leaf;\
		Parent->Child2 = clusters[K];\
		Parent->Indices.insert(Parent->Indices.end(), Parent->Child1->Indices.begin(), Parent->Child1->Indices.end());\
		Parent->Indices.insert(Parent->Indices.end(), Parent->Child2->Indices.begin(), Parent->Child2->Indices.end());\
		Parents.push_back(Parent);\
\
		clusters.erase(clusters.begin() + K);\
		clusters.erase(clusters.begin() + 0);

		while (clusters.size() > 0)
		{
			Ref<Cluster> Leaf = clusters[0];

			if (clusters.size() == 1)
			{
				Parents.push_back(Leaf);
				break;
			}

			for (int k = 1; k < clusters.size(); k++)
			{
				for (int64_t i = 0; i < Leaf->Indices.size(); i += 3)
				{
					const Vertex& A = buffer.Vertices[Leaf->Indices[i + 0]];
					const Vertex& B = buffer.Vertices[Leaf->Indices[i + 1]];
					const Vertex& C = buffer.Vertices[Leaf->Indices[i + 2]];
					for (int64_t j = 0; j < clusters[k]->Indices.size(); j += 3)
					{
						const Vertex& U = buffer.Vertices[clusters[k]->Indices[j + 0]];
						const Vertex& V = buffer.Vertices[clusters[k]->Indices[j + 1]];
						const Vertex& W = buffer.Vertices[clusters[k]->Indices[j + 2]];
						int c = 0;
						if (A == U || A == V || A == W) c++;
						if (B == U || B == V || B == W) c++;
						if (C == U || C == V || C == W) c++;
						if (c >= 2)
						{
							Join_Clusters(k);
							goto Regroup;
						}
					}
				}
			}
			// No matching Cluster Found.
			{
				Join_Clusters(1);
			}
			Regroup:;
		}
		if (Parents.size() > 1)
		{
			std::vector<std::future<void>> jobs;
			for (Ref<Cluster> cluster : Parents)
			{
				//Decimate_Cluster(buffer, cluster);
				jobs.push_back(std::async(std::launch::async, &Mesh::Decimate_Cluster, this, buffer, cluster));
			}
			for (auto& job : jobs)
			{
				job.get();
			}
			GroupClusters(buffer, Parents);
		}
		else
		{
			m_MainCluster = Parents[0];
			int32_t ID = 1;
			SuoraLog("Mesh::DecimaBuilder -> PostProcess");
			PostProcessCluster(buffer, m_MainCluster, ID);
		}
	}

	/*
	void Mesh::Clusterfication(MeshBuffer& buffer)
	{
		int rawCluster = buffer.Indices.size() / 3 / Decima::s_TrianglesPerCluster;
		int nCluster = 1;
		while (nCluster < rawCluster) nCluster *= 2;
		const int64_t LocalTrianlesPerCluster = buffer.Indices.size() / 3 / nCluster;
		std::vector<Ref<Cluster>> leafCluster;
		for (int i = 0; i < nCluster; i++) leafCluster.push_back(CreateRef<Cluster>());

		std::vector<uint32_t> IndicesLeft = buffer.Indices;

		std::unordered_map<Vec3, Cluster*> Clusters_Vertices;

		for (Ref<Cluster> cluster : leafCluster)
		{
			std::vector<uint32_t> NextIndices;
			for (int64_t i = 0; i < IndicesLeft.size(); i += 3)
			{
				const Vertex& A = buffer.Vertices[IndicesLeft[i + 0]];
				const Vertex& B = buffer.Vertices[IndicesLeft[i + 1]];
				const Vertex& C = buffer.Vertices[IndicesLeft[i + 2]];

#define CONTAINS(ITEM) (Clusters_Vertices.find(ITEM) != Clusters_Vertices.end())
#define SHARE(ITEMA, ITEMB) (Clusters_Vertices[ITEMA] == Clusters_Vertices[ITEMB])

				if ((CONTAINS(A.Position) && CONTAINS(B.Position) && SHARE(A.Position, B.Position))
				||  (CONTAINS(B.Position) && CONTAINS(C.Position) && SHARE(B.Position, C.Position))
				||  (CONTAINS(C.Position) && CONTAINS(A.Position) && SHARE(C.Position, A.Position)))
				{
					Cluster* SharedCluster = nullptr;
					if ((CONTAINS(A.Position) && CONTAINS(B.Position) && SHARE(A.Position, B.Position))) SharedCluster = Clusters_Vertices[A.Position];
					if ((CONTAINS(B.Position) && CONTAINS(C.Position) && SHARE(B.Position, C.Position))) SharedCluster = Clusters_Vertices[B.Position];
					if ((CONTAINS(C.Position) && CONTAINS(A.Position) && SHARE(C.Position, A.Position))) SharedCluster = Clusters_Vertices[C.Position];
					
					if (SharedCluster->Indices.size() / 3 < LocalTrianlesPerCluster + 1)
					{
						Clusters_Vertices[A.Position] = SharedCluster; SharedCluster->Indices.push_back(IndicesLeft[i + 0]);
						Clusters_Vertices[B.Position] = SharedCluster; SharedCluster->Indices.push_back(IndicesLeft[i + 1]);
						Clusters_Vertices[C.Position] = SharedCluster; SharedCluster->Indices.push_back(IndicesLeft[i + 2]);
					}
					else if (false)//((IndicesLeft.size() - i) >= (cluster->Indices.size() - LocalTrianlesPerCluster))
					{
						Clusters_Vertices[A.Position] = cluster.get(); cluster->Indices.push_back(IndicesLeft[i + 0]);
						Clusters_Vertices[B.Position] = cluster.get(); cluster->Indices.push_back(IndicesLeft[i + 1]);
						Clusters_Vertices[C.Position] = cluster.get(); cluster->Indices.push_back(IndicesLeft[i + 2]);
					}
					else
					{
						NextIndices.push_back(IndicesLeft[i + 0]); NextIndices.push_back(IndicesLeft[i + 1]); NextIndices.push_back(IndicesLeft[i + 2]);
					}
				}
				else if (cluster->Indices.size() == 0)// || (IndicesLeft.size() - i) <= (LocalTrianlesPerCluster - cluster->Indices.size()))
				{
					Clusters_Vertices[A.Position] = cluster.get(); cluster->Indices.push_back(IndicesLeft[i + 0]);
					Clusters_Vertices[B.Position] = cluster.get(); cluster->Indices.push_back(IndicesLeft[i + 1]);
					Clusters_Vertices[C.Position] = cluster.get(); cluster->Indices.push_back(IndicesLeft[i + 2]);
				}
				else
				{
					NextIndices.push_back(IndicesLeft[i + 0]); NextIndices.push_back(IndicesLeft[i + 1]); NextIndices.push_back(IndicesLeft[i + 2]);
				}
			}
			IndicesLeft = NextIndices;
		}
		int runs = 0;
		while (IndicesLeft.size() > 0)
		{
			int size = IndicesLeft.size();
			for (int64_t i = 0; i < IndicesLeft.size(); i += 3)
			{
				const Vertex& A = buffer.Vertices[IndicesLeft[i + 0]];
				const Vertex& B = buffer.Vertices[IndicesLeft[i + 1]];
				const Vertex& C = buffer.Vertices[IndicesLeft[i + 2]];

				if ((CONTAINS(A.Position) && CONTAINS(B.Position) && SHARE(A.Position, B.Position))
				||  (CONTAINS(B.Position) && CONTAINS(C.Position) && SHARE(B.Position, C.Position))
				||  (CONTAINS(C.Position) && CONTAINS(A.Position) && SHARE(C.Position, A.Position)))
				{
					Cluster* SharedCluster = nullptr;
					if ((CONTAINS(A.Position) && CONTAINS(B.Position) && SHARE(A.Position, B.Position))) SharedCluster = Clusters_Vertices[A.Position];
					if ((CONTAINS(B.Position) && CONTAINS(C.Position) && SHARE(B.Position, C.Position))) SharedCluster = Clusters_Vertices[B.Position];
					if ((CONTAINS(C.Position) && CONTAINS(A.Position) && SHARE(C.Position, A.Position))) SharedCluster = Clusters_Vertices[C.Position];

					if (SharedCluster->Indices.size() / 3 < LocalTrianlesPerCluster * 1.5f)
					{
						Clusters_Vertices[A.Position] = SharedCluster; SharedCluster->Indices.push_back(IndicesLeft[i + 0]);
						Clusters_Vertices[B.Position] = SharedCluster; SharedCluster->Indices.push_back(IndicesLeft[i + 1]);
						Clusters_Vertices[C.Position] = SharedCluster; SharedCluster->Indices.push_back(IndicesLeft[i + 2]);
						IndicesLeft.erase(IndicesLeft.begin() + i, IndicesLeft.begin() + i + 3);
						break;
					}
				}
			}
			if (size == IndicesLeft.size()) runs++; else runs = 0;
			if (runs >= 32) break;
		}
		for (int64_t i = 0; i < IndicesLeft.size(); i += 3)
		{
			leafCluster[i % leafCluster.size()]->Indices.push_back(IndicesLeft[i + 0]);
			leafCluster[i % leafCluster.size()]->Indices.push_back(IndicesLeft[i + 1]);
			leafCluster[i % leafCluster.size()]->Indices.push_back(IndicesLeft[i + 2]);
		}
		

		if (leafCluster.size() % 2 == 0)
			GroupClusters(buffer, leafCluster);
	}*/

	void Mesh::Clusterfication(MeshBuffer& buffer)
	{
		SuoraLog("Mesh::Clusterfication(MeshBuffer&)");
		MeshBuffer TempVertexBuffer; TempVertexBuffer.Vertices = buffer.Vertices;
		std::vector<Ref<Cluster>> leafCluster;
		int TrianglesLeft = buffer.Indices.size() / 3;
		std::vector<bool> TriangleFlags;
		for (int64_t i = 0; i < buffer.Indices.size(); i += 3) TriangleFlags.push_back(false);
		
		std::unordered_map<Vec3, std::vector<uint32_t>> Tris_Vertices;

		for (int64_t i = 0; i < buffer.Indices.size(); i += 3)
		{
			if (i + 2 >= buffer.Indices.size()) continue;
			const Vertex& A = buffer.Vertices[buffer.Indices[i + 0]];
			const Vertex& B = buffer.Vertices[buffer.Indices[i + 1]];
			const Vertex& C = buffer.Vertices[buffer.Indices[i + 2]];

			if (Tris_Vertices.find(A.Position) == Tris_Vertices.end()) Tris_Vertices[A.Position] = {}; Tris_Vertices[A.Position].push_back(i + 0);
			if (Tris_Vertices.find(B.Position) == Tris_Vertices.end()) Tris_Vertices[B.Position] = {}; Tris_Vertices[B.Position].push_back(i + 1);
			if (Tris_Vertices.find(C.Position) == Tris_Vertices.end()) Tris_Vertices[C.Position] = {}; Tris_Vertices[C.Position].push_back(i + 2);
		}
		
		int64_t TriangleFlagsThreshhold = 0;
		while (TrianglesLeft > 0)
		{
			Ref<Cluster> cluster = CreateRef<Cluster>();
			leafCluster.push_back(cluster);
			for (int64_t i = TriangleFlagsThreshhold; i < TriangleFlags.size(); i++)
			{
				if (!TriangleFlags[i])
				{
					TriangleFlagsThreshhold = i;
					TriangleFlags[i] = true;
					TrianglesLeft--;
					cluster->Indices.push_back(buffer.Indices[i * 3 + 0]);
					cluster->Indices.push_back(buffer.Indices[i * 3 + 1]);
					cluster->Indices.push_back(buffer.Indices[i * 3 + 2]);
					break;
				}
			}
			bool bTrisFound = false;
			do
			{
				bTrisFound = false;
				std::vector<std::pair<uint32_t, uint32_t>> Edges;
				TempVertexBuffer.Indices = cluster->Indices;

				for (int64_t i = 0; i < cluster->Indices.size(); i += 3)
				{
					if (CheckSharedTris(TempVertexBuffer, i + 0, i + 1) == 1) Edges.push_back({ i + 0, i + 1 });
					if (CheckSharedTris(TempVertexBuffer, i + 1, i + 2) == 1) Edges.push_back({ i + 1, i + 2 });
					if (CheckSharedTris(TempVertexBuffer, i + 2, i + 0) == 1) Edges.push_back({ i + 2, i + 0 });
				}
				for (auto& Edge : Edges)
				{
					std::vector<uint32_t>& Us = Tris_Vertices[buffer.Vertices[cluster->Indices[Edge.first]].Position];
					std::vector<uint32_t>& Vs = Tris_Vertices[buffer.Vertices[cluster->Indices[Edge.second]].Position];

					for (int64_t i = 0; i < Us.size(); i++)
					{
						for (int64_t j = 0; j < Vs.size(); j++)
						{
							if (Us[i] / 3 == Vs[j] / 3 && !TriangleFlags[Us[i] / 3] && cluster->Indices.size() / 3 < Decima::s_TrianglesPerCluster)
							{
								TriangleFlags[Us[i] / 3] = true;
								TrianglesLeft--;
								cluster->Indices.push_back(buffer.Indices[(Us[i] / 3) * 3 + 0]);
								cluster->Indices.push_back(buffer.Indices[(Us[i] / 3) * 3 + 1]);
								cluster->Indices.push_back(buffer.Indices[(Us[i] / 3) * 3 + 2]);
								bTrisFound = true;
								goto _end;
							}
						}
					}
				_end:;
				
				}

			} while (cluster->Indices.size() / 3 < Decima::s_TrianglesPerCluster && bTrisFound);
		
		}
		
		GroupClusters(buffer, leafCluster);
	}

	Array<std::string> Mesh::GetSupportedSourceAssetExtensions()
	{
		return {".obj", ".fbx", ".gltf"};
	}

	Ref<VertexArray> Cluster::GetVertexArray(MeshBuffer& buffer)
	{
		if (!m_VertexArray.get())
		{
			MeshBuffer VaoBuffer;
			VaoBuffer.Vertices = buffer.Vertices;
			VaoBuffer.Indices = Indices;
			VaoBuffer.Optimize();
			m_VertexArray = Ref<VertexArray>(VertexArray::Create(VaoBuffer));
		}

		return m_VertexArray;
	}

}