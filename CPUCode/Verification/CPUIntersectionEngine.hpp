/*
 * CPUIntersectionEngine.hpp
 *
 *  Created on: 15 Jul 2015
 *      Author: sfriston
 */

#ifndef CPUINTERSECTIONENGINE_HPP_
#define CPUINTERSECTIONENGINE_HPP_

#include "Types.h"

#define EPSILON 0.000001

class CPUIntersectionEngine
{
public:
	triangle_t* m_triangles;
	size_t m_num_triangles;

	ray_t* m_rays;
	size_t m_num_rays;

	std::vector<intersection_t> m_intersections;

public:
	void DoIntersectionTests()
	{
		for(uint r = 0; r < m_num_rays; r++)
		{
			for(uint t = 0; t < m_num_triangles; t++)
			{
				if(CheckIntersection(m_triangles[t], m_rays[r]))
				{
					intersection_t result;
					result.ray = r;
					result.triangle = t;
					m_intersections.push_back(result);
				}
			}
		}
	}

private:
	bool CheckIntersection(triangle_t t, ray_t r)
	{
		return triangle_intersection(t.v0, t.v1, t.v2, r.origin, r.direction) > 0;
	}

	//Thanks: http://rosettacode.org/wiki/Vector_products#C.2B.2B
	float DOT(vector3 lhs,  vector3 rhs )
	{
		float scalar = lhs.x * rhs.x + lhs.y * rhs.y + lhs.z * rhs.z ;
		return scalar ;
	}

	vector3 CROSS (vector3 lhs, vector3 rhs )
	{
		float a = lhs.y * rhs.z - lhs.z * rhs.y ;
		float b = lhs.z * rhs.x - lhs.x * rhs.z ;
		float c = lhs.x * rhs.y - lhs.y * rhs.x ;
		vector3 product( a , b , c ) ;
		return product ;
	}

	vector3 SUB (vector3 lhs, vector3 rhs )
	{
		float a = lhs.x - rhs.x;
		float b = lhs.y - rhs.y;
		float c = lhs.z - rhs.z;
		vector3 product( a , b , c ) ;
		return product ;
	}

	//Thankshttps://en.wikipedia.org/wiki/M%C3%B6ller%E2%80%93Trumbore_intersection_algorithm
	int triangle_intersection( const vector3   V1,  // Triangle vertices
							   const vector3   V2,
							   const vector3   V3,
							   const vector3    O,  //Ray origin
							   const vector3    D)  //Ray direction
	{
	  vector3 e1, e2;  //Edge1, Edge2
	  vector3 P, Q, T;
	  float det, inv_det, u, v;
	  float t;

	  //Find vectors for two edges sharing V1
	  e1 = SUB(V2, V1);
	  e2 = SUB(V3, V1);
	  //Begin calculating determinant - also used to calculate u parameter
	  P = CROSS(D, e2);
	  //if determinant is near zero, ray lies in plane of triangle
	  det = DOT(e1, P);
	  //NOT CULLING
	  if(det > -EPSILON && det < EPSILON) return 0;
	  inv_det = 1.f / det;

	  //calculate distance from V1 to ray origin
	  T = SUB(O, V1);

	  //Calculate u parameter and test bound
	  u = DOT(T, P) * inv_det;
	  //The intersection lies outside of the triangle
	  if(u < 0.f || u > 1.f) return 0;

	  //Prepare to test v parameter
	  Q = CROSS(T, e1);

	  //Calculate V parameter and test bound
	  v = DOT(D, Q) * inv_det;
	  //The intersection lies outside of the triangle
	  if(v < 0.f || u + v  > 1.f) return 0;

	  t = DOT(e2, Q) * inv_det;

	  if(t > EPSILON) { //ray intersection
		return 1;
	  }

	  // No hit, no win
	  return 0;
	}

};

#endif /* CPUINTERSECTIONENGINE_HPP_ */
