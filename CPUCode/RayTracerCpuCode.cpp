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


	max_actions_t* act = max_actions_init(maxfile, NULL);
	
	int triangles_per_tick = max_get_constant_uint64t(maxfile, "TrianglesPerTick");
	int rays_per_tick = 1;
	int rays_in_set = test_manager.m_rays_count;
	int triangles_in_set = tris->m_total_triangles;

	int intersection_ticks = (triangles_in_set / triangles_per_tick) * (rays_in_set / rays_per_tick);
	int memory_command_ticks = (rays_in_set / rays_per_tick);

	max_set_ticks(act, "MemoryCommandGenerator", memory_command_ticks);
	max_set_uint64t(act,"MemoryCommandGenerator","triangles_to_read_in_bursts",tris->m_total_bursts);

	max_ignore_lmem(act,"triangles_to_mem");

	max_set_ticks(act, "RayTracerKernel", intersection_ticks);
	max_set_uint64t(act,"RayTracerKernel","total_triangles",triangles_in_set);
	max_set_uint64t(act,"RayTracerKernel","total_rays",rays_in_set);

	/* Queue the rays */

	max_queue_input(act, "rays_in", test_manager.m_rays, test_manager.m_rays_size);



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
