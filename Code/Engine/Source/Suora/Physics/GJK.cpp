#include "Precompiled.h"
#include "GJK.h"
#include <float.h>

namespace Suora::Physics
{
	/// GJK

	inline extern std::pair<bool, Simplex> GJK(const ConvexShape& colliderA, const glm::mat4& transformA, const ConvexShape& colliderB, const glm::mat4& transformB)
	{
		using vec_t = Vec3;

		vec_t support = Support(colliderA, transformA, colliderB, transformB, vec_t(1));

		Simplex points;
		points.push_front(support);

		vec_t direction = -support;

		size_t iterations = 0;
		while (iterations++ < GJK_EPA_MAX_ITER)
		{
			support = Support(colliderA, transformA, colliderB, transformB, direction);

			if (dot(support, direction) <= 0)
			{
				break;
			}

			points.push_front(support);

			if (NextSimplex(points, direction))
			{
				return std::make_pair(true, points);
			}
		}

		return std::make_pair(false, points);
	}

	bool NextSimplex(Simplex& points, Vec3& direction)
	{
		switch (points.size()) 
		{
		case 2: return Line(points, direction);
		case 3: return Triangle(points, direction);
		case 4: return Tetrahedron(points, direction);
		}

		SUORA_VERIFY(false);
		return false;
	}

	bool Line(Simplex& points, Vec3& direction)
	{
		using vec_t = Vec3;

		vec_t a = points[0];
		vec_t b = points[1];

		vec_t ab = b - a;
		vec_t ao = -a;

		if (SameDirection(ab, ao)) 
		{
			direction = glm::cross(glm::cross(ab, ao), ab);
		}
		else 
		{
			points = { a };
			direction = ao;
		}

		return false;
	}

	bool Triangle(Simplex& points, Vec3& direction)
	{
		using vec_t = Vec3;

		vec_t a = points[0];
		vec_t b = points[1];
		vec_t c = points[2];

		vec_t ab = b - a;
		vec_t ac = c - a;
		vec_t ao = -a;

		vec_t abc = glm::cross(ab, ac);

		if (SameDirection(glm::cross(abc, ac), ao)) 
		{
			if (SameDirection(ac, ao)) 
			{
				points = { a, c };
				direction = glm::cross(glm::cross(ac, ao), ac);
			}
			else 
			{
				return Line(points = { a, b }, direction);
			}
		}
		else 
		{
			if (SameDirection(cross(ab, abc), ao)) 
			{
				return Line(points = { a, b }, direction);
			}
			else 
			{
				if (SameDirection(abc, ao)) 
				{
					direction = abc;
				}
				else 
				{
					points = { a, c, b };
					direction = -abc;
				}
			}
		}

		return false;
	}

	bool Tetrahedron(Simplex& points, Vec3& direction)
	{
		using vec_t = Vec3;

		vec_t a = points[0];
		vec_t b = points[1];
		vec_t c = points[2];
		vec_t d = points[3];

		vec_t ab = b - a;
		vec_t ac = c - a;
		vec_t ad = d - a;
		vec_t ao = -a;

		vec_t abc = glm::cross(ab, ac);
		vec_t acd = glm::cross(ac, ad);
		vec_t adb = glm::cross(ad, ab);

		if (SameDirection(abc, ao)) return Triangle(points = { a, b, c }, direction);
		if (SameDirection(acd, ao)) return Triangle(points = { a, c, d }, direction);
		if (SameDirection(adb, ao)) return Triangle(points = { a, d, b }, direction);

		return true;
	}

	/// EPA


	ManifoldPoints EPA(const Simplex& simplex, const ConvexShape& colliderA, const glm::mat4& transformA, const ConvexShape& colliderB, const glm::mat4& transformB)
	{
		using vec_t = Vec3;

		std::vector<vec_t> polytope(simplex.begin(), simplex.end());
		std::vector<size_t>  faces = 
		{
			0, 1, 2,
			0, 3, 1,
			0, 2, 3,
			1, 3, 2
		};

		auto [normals, minFace] = GetFaceNormals(polytope, faces);

		vec_t minNormal;
		double minDistance = DBL_MAX;

		size_t iterations = 0;
		while (minDistance == DBL_MAX)
		{
			minNormal = glm::vec3(normals[minFace]);
			minDistance = normals[minFace].w;

			if (iterations++ > GJK_EPA_MAX_ITER) 
			{
				break;
			}

			vec_t support = Support(colliderA, transformA, colliderB, transformB, minNormal);
			double sDistance = glm::dot(minNormal, support);

			if (abs(sDistance - minDistance) > 0.001f) 
			{
				minDistance = DBL_MAX;

				std::vector<std::pair<size_t, size_t>> uniqueEdges;

				for (size_t i = 0; i < normals.size(); i++) 
				{
					if (SameDirection(vec_t(normals[i]), support)) 
					{
						size_t f = i * 3;

						AddIfUniqueEdge(uniqueEdges, faces, f, f + 1);
						AddIfUniqueEdge(uniqueEdges, faces, f + 1, f + 2);
						AddIfUniqueEdge(uniqueEdges, faces, f + 2, f);

						faces[f + 2] = faces.back(); faces.pop_back();
						faces[f + 1] = faces.back(); faces.pop_back();
						faces[f] = faces.back(); faces.pop_back();

						normals[i] = normals.back(); normals.pop_back();

						i--;
					}
				}

				if (uniqueEdges.size() == 0) 
				{
					break;
				}

				std::vector<size_t> newFaces;
				for (auto [edge1, edge2] : uniqueEdges) 
				{
					newFaces.push_back(edge1);
					newFaces.push_back(edge2);
					newFaces.push_back(polytope.size());
				}

				polytope.push_back(support);

				auto [newNormals, newMinFace] = GetFaceNormals(polytope, newFaces);

				double newMinDistance = DBL_MAX;
				for (size_t i = 0; i < normals.size(); i++) 
				{
					if (normals[i].w < newMinDistance) 
					{
						newMinDistance = normals[i].w;
						minFace = i;
					}
				}

				if (newNormals[newMinFace].w < newMinDistance) 
				{
					minFace = newMinFace + normals.size();
				}

				faces.insert(faces.end(), newFaces.begin(), newFaces.end());
				normals.insert(normals.end(), newNormals.begin(), newNormals.end());
			}
		}

		if (minDistance == DBL_MAX)
		{
			return {};
		}

		ManifoldPoints points;

		points.Normal = minNormal;
		points.Depth = minDistance + 0.001f;
		points.HasCollision = true;

		return points;
	}

	std::pair<std::vector<glm::vec4>, size_t> GetFaceNormals(const std::vector<glm::vec3>& polytope, const std::vector<size_t>& faces)
	{
		std::vector<glm::vec4> normals;
		size_t minTriangle = 0;
		float  minDistance = FLT_MAX;

		for (size_t i = 0; i < faces.size(); i += 3) 
		{
			glm::vec3 a = polytope[faces[i]];
			glm::vec3 b = polytope[faces[i + 1]];
			glm::vec3 c = polytope[faces[i + 2]];

			glm::vec3 normal = glm::normalize(glm::cross(b - a, c - a));
			float distance = glm::dot(normal, a);

			if (distance < 0) 
			{
				normal *= -1;
				distance *= -1;
			}

			normals.emplace_back(normal, distance);

			if (distance < minDistance) 
			{
				minTriangle = i / 3;
				minDistance = distance;
			}
		}

		return { normals, minTriangle };
	}

	void AddIfUniqueEdge(std::vector<std::pair<size_t, size_t>>& edges, const std::vector<size_t>& faces, size_t a, size_t b)
	{
		auto reverse = std::find(              //      0--<--3
			edges.begin(),                     //     / \ B /   A: 2-0
			edges.end(),                       //    / A \ /    B: 0-2
			std::make_pair(faces[b], faces[a]) //   1-->--2
		);

		if (reverse != edges.end()) 
		{
			edges.erase(reverse);
		}
		else 
		{
			edges.emplace_back(faces[a], faces[b]);
		}
	}

}