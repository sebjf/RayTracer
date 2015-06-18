#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>

#include "Maxfiles.h"
#include "MaxSLiCInterface.h"

struct vector3
{
	float x;
	float y;
	float z;
};

struct triangle_t
{
	struct vector3 v0;
	struct vector3 v1;
	struct vector3 v2;
	char padding[12];
};

struct ray_t
{
	struct vector3 origin;
	struct vector3 direction;
	u_int64_t padding;
};

struct result_t
{
	u_int32_t triangle;
};


int main(void)
{
	int N = 16; //5 intersection tests


	/* prepare some rays */

	int rays_size = (sizeof(struct ray_t) * N);
	struct ray_t* rays = malloc(rays_size);
	memset(rays,0,rays_size);

	for(int i = 0; i < N; i++)
	{
		rays[i].direction.z = 1;
	}


	/* prepare some triangles */

	int triangles_size = (sizeof(struct triangle_t) * N);
	struct triangle_t* triangles = malloc(triangles_size);
	memset(triangles,0,triangles_size);

	for(int i = 0; i < N; i++)
	{
		triangles[i].v0.z = 1;
		triangles[i].v1.z = 1;
		triangles[i].v2.z = 1;

		triangles[i].v0.x = -1;
		triangles[i].v2.x = 1;

		triangles[i].v0.y = -1;
		triangles[i].v1.y = 1;
		triangles[i].v2.y = -1;

	}

	/* prepare the kernel */

	int results_size = (sizeof(struct result_t) * N);
	struct result_t* results = malloc(results_size);
	memset(results,0,results_size);

	max_file_t *maxfile = RayTracer_init();
	max_engine_t *engine = max_load(maxfile, "*");

	max_actions_t* act = max_actions_init(maxfile, NULL);
	
	max_set_ticks(act, "RayTracerKernel", N);
	max_queue_input(act, "triangles_in", triangles, triangles_size);
	max_queue_input(act, "rays_in", rays, rays_size);
	max_queue_output(act, "results_out", results, results_size);

	printf("Running on DFE...\n");

	max_run(engine, act);

	max_unload(engine);
	
	// TODO Use result data
	for(int i = 0; i < N; ++i)
	{
		printf("%i: %i\n", i, results[i].triangle);
	}

	printf("Done.\n");
	return 0;
}
