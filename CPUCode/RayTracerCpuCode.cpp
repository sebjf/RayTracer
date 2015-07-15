#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <unistd.h>

#include "Maxfiles.h"
#include "MaxSLiCInterface.h"
#include "Triangles.hpp"
#include "Results.hpp"
#include "Status.hpp"
#include "Verification/TestManager.hpp"


/* For optimum performance, the rays word width should always be a multiple of the PCIe word width, and therefore the main function of this class
 * is to ensure that the rays provided are a multiple of the rays word size in rays, so there is no stalling waiting on data. */
class Rays
{
public:
	ray_t* m_rays;
	size_t m_num_rays;

private:
	int m_rays_width_in_bytes;
	int m_rays_width_in_rays;


public:
	Rays(max_file_t* maxfile)
	{
		m_rays_width_in_bytes = max_get_constant_uint64t(maxfile,"RaysWordWidthInBits") / 8;
		m_rays_width_in_rays = max_get_constant_uint64t(maxfile,"RaysPerWord");

		if(m_rays_width_in_bytes != (m_rays_width_in_rays * sizeof(ray_t)))
		{
			printf("ERROR: rays word width is not a multiple of the ray data structure width. This is not currently supported.\n");
		}
	}

	void SetRays(ray_t* rays, size_t num_rays)
	{
		m_rays = rays;
		m_num_rays = num_rays;

		//for rays, only a simple check if we need to pad the input to make the ray count a multiple of the rays word width (in rays)
		if((num_rays % m_rays_width_in_rays) != 0)
		{
			m_num_rays = m_num_rays + (m_rays_width_in_rays - (num_rays % m_rays_width_in_rays));
			m_rays = (ray_t*)malloc(m_num_rays * sizeof(ray_t));
			memset(m_rays, 0, m_num_rays * sizeof(ray_t));
			memcpy(m_rays, rays, num_rays * sizeof(ray_t));
		}
	}

	void QueueRays(max_actions_t* actions)
	{
		max_queue_input(actions, "rays_in", m_rays, m_num_rays * sizeof(ray_t));
	}


};

int main(void)
{
	max_file_t *maxfile = RayTracer_init();
	max_engine_t *engine = max_load(maxfile, "*");

	TestManager test_manager;
	test_manager.Initiliase();

	/* initialise triangles */

	Triangles* tris = new Triangles(maxfile, 16);
	tris->SetTriangles(test_manager.m_triangles, test_manager.m_triangle_count);
	tris->IntialiseTriangles(engine,0);

	/* Queue the rays */

	Rays rays(maxfile);
	rays.SetRays(test_manager.m_rays, test_manager.m_rays_count);

	max_actions_t* act = max_actions_init(maxfile, NULL);
	
	int triangles_per_tick = max_get_constant_uint64t(maxfile, "TrianglesPerTick");
	int rays_per_tick = max_get_constant_uint64t(maxfile, "RaysPerTick");
	int rays_in_set = rays.m_num_rays;
	int triangles_in_set = tris->m_total_triangles;

	int intersection_ticks = (triangles_in_set / triangles_per_tick) * (rays_in_set / rays_per_tick);
	int memory_command_ticks = (rays_in_set / rays_per_tick);

	max_set_ticks(act, "MemoryCommandGenerator", memory_command_ticks);
	max_set_uint64t(act,"MemoryCommandGenerator","triangles_to_read_in_bursts",tris->m_total_bursts);

	max_ignore_lmem(act,"triangles_to_mem");

	max_set_ticks(act, "RayTracerKernel", intersection_ticks);
	max_set_uint64t(act,"RayTracerKernel","total_triangles",triangles_in_set);
	max_set_uint64t(act,"RayTracerKernel","total_rays",rays_in_set);

	rays.QueueRays(act);


	/* prepare the output */

	Results results(maxfile, engine);
	Status status(maxfile, engine);

	printf("Running on DFE...\n");

	max_run_t* max_run = max_run_nonblock(engine, act);

	while(true)
	{
		results.ReadResults();

		if(status.ReadStatus()){
			status.PrintSummary();
			break;
		}
	}

	test_manager.CheckResults(results);

	max_wait(max_run);
	max_unload(engine);

	printf("Done.\n");
	
	return 0;
}
