#pragma once
#include "Suora/Common/VectorUtils.h"
#include "PhysicsEngine.h"
#include <array>
#include <vector>
#include <initializer_list>
#include <utility>

namespace Suora::Physics
{
	// Inspired by: https://github.com/IainWinter/IwEngine/tree/master/IwEngine/src/physics (MIT)
	// TODO: Replace this with JoltPhysics!

	struct ManifoldPoints;

	struct Simplex 
	{
		std::array<Vec3, 4> Points;
		size_t m_size;

		Simplex()
			: Points()
			, m_size(0)
		{
		}

		Simplex& operator=(std::initializer_list<Vec3> list)
		{
			for (auto v = list.begin(); v != list.end(); v++) 
			{
					Points[std::distance(list.begin(), v)] = *v;
			}
			m_size = list.size();

			return *this;
		}

		void push_front(const Vec3& point)
		{

			Points = { point, Points[0], Points[1], Points[2] };


			m_size = glm::min<size_t>(m_size + 1, 4);
		}

		Vec3& operator[](unsigned i) { return Points[i]; }
		size_t size() const { return m_size; }

		auto begin() const { return Points.begin(); }
		auto end()   const { return Points.end() - (4 - m_size); }
	};

#ifndef GJK_EPA_MAX_ITER
#	define GJK_EPA_MAX_ITER 64
#endif

	inline extern std::pair<bool, Simplex> GJK(const ConvexShape& colliderA, const glm::mat4& transformA, const ConvexShape& colliderB, const glm::mat4& transformB);

	bool NextSimplex(Simplex& points, Vec3& direction);
	bool Line(Simplex& points, Vec3& direction);
	bool Triangle(Simplex& points, Vec3& direction);
	bool Tetrahedron(Simplex& points, Vec3& direction);


	ManifoldPoints EPA(const Simplex& simplex,
		const ConvexShape& colliderA, const glm::mat4& transformA,
		const ConvexShape& colliderB, const glm::mat4& transformB);

	
	static Vec3 Support(const ConvexShape& colliderA, const glm::mat4& transformA, const ConvexShape& colliderB, const glm::mat4& transformB, const Vec3& direction)
	{
		return colliderA.FindFurthestPoint(transformA, direction) - colliderB.FindFurthestPoint(transformB, -direction);
	}

	
	static bool SameDirection(const Vec3& direction, const Vec3& ao)
	{
		return glm::dot(direction, ao) > 0;
	}

	// For d3 EPA

	std::pair<std::vector<glm::vec4>, size_t> GetFaceNormals(const std::vector<glm::vec3>& polytope, const std::vector<size_t>& faces);
	void AddIfUniqueEdge(std::vector<std::pair<size_t, size_t>>& edges, const std::vector<size_t>& faces, size_t a, size_t b);

}