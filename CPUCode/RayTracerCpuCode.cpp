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

class Triangles
{
public:
	triangle_t* m_triangles;

	max_file_t* m_maxfile;

	int m_triangles_size_in_bytes;

	int m_total_triangles;
	int m_total_bursts;
	int m_total_words;

	int m_triangles_per_word;
	int m_word_width_in_bytes;

	Triangles(max_file_t* maxfile, int triangle_count)
	{
		m_maxfile = maxfile;

		/* first compute the number of triangles in this set - this may be larger than triangle_count because triangle arrays must be aligned on bursts */

		int burst_size_in_bytes = max_get_burst_size(maxfile, NULL);
		int triangles_per_burst = floor((float)burst_size_in_bytes/(float)sizeof(triangle_t));

		m_total_bursts = ceil((float)triangle_count / (float)triangles_per_burst);
		m_total_triangles = m_total_bursts * triangles_per_burst;

		m_triangles_size_in_bytes = m_total_triangles * sizeof(triangle_t);

		/* next compute the boundaries in memory for subsets of triangles - triangles are read into the kernel N triangles at a time, and there may be padding
		 * depending on the triangle width
		 * triangles may cross burst boundaries (though not at the beginning or end) but they will not cross word boundaries */

		int word_width_in_bits = max_get_constant_uint64t(maxfile, "TrianglesInWidthInBits");
		m_word_width_in_bytes = word_width_in_bits / 8;

		int triangle_size_in_bytes = sizeof(triangle_t);
		int triangle_size_in_bits = triangle_size_in_bytes/8;

		m_triangles_per_word = floor((float)word_width_in_bits / (float)triangle_size_in_bits);

		m_total_words = m_total_triangles / m_triangles_per_word;

	}

	triangle_t* Prepare()
	{
		triangles_data = (triangle_t*)malloc(m_triangles_size_in_bytes);

		//copy the triangles into the triangles_data array - triangles_data is laid out such that there is padding in the correct place when sets of triangles
		//are read from lmem into the kernel

		for(int i = 0; i < m_total_words; i++)
		{
			memccpy(triangles_data, m_triangles, sizeof(triangle_t), m_triangles_per_word);
			triangles_data += m_word_width_in_bytes;
			m_triangles += m_triangles_per_word;
		}

		return triangles_data;
	}

private:
	triangle_t* triangles_data;

};

int main(void)
{
	max_file_t *maxfile = RayTracer_init();
	max_engine_t *engine = max_load(maxfile, "*");

	int N = 16; //5 intersection tests

	/* prepare some triangles */

	/* the triangles data must be a multiple of the burst width of the memory, any spare capacity should be filled with invalid triangles */

	int total_triangles = 16;

	int burst_size_in_bytes = max_get_burst_size(maxfile, NULL);
	int triangles_per_burst = floor((float)burst_size_in_bytes)/(float)sizeof(struct triangle_t);
	int total_bursts = ceil((float)total_triangles / (float)triangles_per_burst);
	int triangle_size_in_bytes = total_bursts * burst_size_in_bytes;

	triangle_t* triangles = (triangle_t*)malloc(triangle_size_in_bytes);
	memset(triangles,0,triangle_size_in_bytes);

	for(int i = 0; i < total_triangles; i++)
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

	/* prepare some rays */

	int rays_size = (sizeof(struct ray_t) * N);
	ray_t* rays = (ray_t*)malloc(rays_size);
	memset(rays,0,rays_size);

	for(int i = 0; i < N; i++)
	{
		rays[i].direction.z = 1;
	}

	/* prepare the kernel */

	int results_size = (sizeof(struct result_t) * N);
	result_t* results = (result_t*)malloc(results_size);
	memset(results,0,results_size);



	/* initialise triangles */

	max_actions_t* init_act = max_actions_init(maxfile, "memoryInitialisation");
	max_set_param_uint64t(init_act, "address", 0);
	max_set_param_uint64t(init_act, "size", triangle_size_in_bytes);
	max_queue_input(init_act,"triangles_in",triangles,triangle_size_in_bytes);

	max_run(engine, init_act);

	max_actions_t* act = max_actions_init(maxfile, NULL);
	


	int triangles_per_tick = 1;
	int rays_per_tick = 1;
	int rays_in_set = N;
	int triangles_in_set = total_bursts * triangles_per_burst;

	int intersection_ticks = (triangles_in_set / triangles_per_tick) * (rays_in_set / rays_per_tick);
	int memory_command_ticks = (rays_in_set / rays_per_tick);

	max_set_ticks(act, "MemoryCommandGenerator", memory_command_ticks);
	max_set_uint64t(act,"MemoryCommandGenerator","triangles_to_read_in_bursts",total_bursts);

	max_ignore_lmem(act,"triangles_to_mem");
	max_set_ticks(act, "RayTracerKernel", intersection_ticks);
	max_set_uint64t(act,"RayTracerKernel","total_triangles",triangles_in_set);
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
