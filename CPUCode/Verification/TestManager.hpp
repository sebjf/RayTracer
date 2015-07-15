/*
 * TestManager.hpp
 *
 *  Created on: 15 Jul 2015
 *      Author: sfriston
 */

#ifndef TESTMANAGER_HPP_
#define TESTMANAGER_HPP_

#include "CPUIntersectionEngine.hpp"
#include <algorithm>    // std::find

class TestManager
{
public:
	triangle_t* m_triangles;
	size_t m_triangles_size;
	size_t m_triangle_count;

	ray_t* m_rays;
	size_t m_rays_count;
	size_t m_rays_size;

public:
	void Initiliase()
	{
		m_triangle_count = 16;

		m_triangles_size = sizeof(triangle_t) * m_triangle_count;
	    m_triangles = (triangle_t*)malloc(m_triangles_size);
		memset(m_triangles,0,m_triangles_size);

		for(uint i = 0; i < m_triangle_count; i++)
		{
			m_triangles[i].v0.z = 1;
			m_triangles[i].v1.z = 1;
			m_triangles[i].v2.z = 1;

			m_triangles[i].v0.x = -1;
			m_triangles[i].v2.x =  1;

			m_triangles[i].v0.y = -1;
			m_triangles[i].v1.y =  1;
			m_triangles[i].v2.y = -1;
		}


		/* prepare some rays */

		m_rays_count = 16; //5 intersection tests

		m_rays_size = (sizeof(struct ray_t) * m_rays_count);
		m_rays = (ray_t*)malloc(m_rays_size);
		memset(m_rays,0,m_rays_size);

		for(uint i = 0; i < m_rays_count; i++)
		{
			if(i > 10){
				m_rays[i].direction.z = 1;
			}
			else
			{
				m_rays[i].direction.z = 1;
				m_rays[i].direction.x = 100;
			}
		}

	}

	bool CheckResults(Results& results)
	{
		CPUIntersectionEngine cpu_engine;
		cpu_engine.m_num_rays = m_rays_count;
		cpu_engine.m_rays = m_rays;
		cpu_engine.m_num_triangles = m_triangle_count;
		cpu_engine.m_triangles = m_triangles;

		printf("Running CPU intersection tests...");
		cpu_engine.DoIntersectionTests();
		printf("Done.\n");

		if(results.m_intersections.size() == cpu_engine.m_intersections.size())
		{
			printf("1. Counts match.\n");
		}else
		{
			printf("1. ERROR: Counts do not match.\n");
			return false;
		}

		for(uint i = 0; i < cpu_engine.m_intersections.size(); i++)
		{
			if(std::find(results.m_intersections.begin(), results.m_intersections.end(), cpu_engine.m_intersections[i])==results.m_intersections.end())
			{
				printf("2. ERROR: Result not found (%i, %i)!\n", cpu_engine.m_intersections[i].ray, cpu_engine.m_intersections[i].triangle);
			}
		}

		printf("2. All results accounted for.\n");


		return false;
	}

};

#endif /* TESTMANAGER_HPP_ */
