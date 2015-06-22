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
	u_int64_t result;
	u_int64_t padding;
};

class Triangles
{
private:
	triangle_t* m_triangles;

	max_file_t* m_maxfile;

	int m_triangles_size_in_bytes;

public:
	int m_total_triangles;
	int m_total_bursts;
	int m_total_words;

	float m_triangles_per_word;
	float m_word_width_in_bytes;
	float m_burst_size_in_bytes;

	Triangles(max_file_t* maxfile, int triangle_count)
	{
		m_maxfile = maxfile;

		/* compute how many triangles will fit in one word. words may not be a multiple of the triangle width so will include padding at the msb */

		int word_width_in_bits = max_get_constant_uint64t(maxfile, "TrianglesInWidthInBits");
		m_word_width_in_bytes = word_width_in_bits / 8;
		float triangle_size_in_bytes = sizeof(triangle_t);

		m_triangles_per_word = floor(m_word_width_in_bytes / triangle_size_in_bytes);

		/* calculate the minimum number of words required to represent the triangle set size we desire. this may be changed later on to accomodate the number of bursts
		 * required to read all the triangles */

		m_total_words = ceil((float)triangle_count / m_triangles_per_word);

		/* compute the number of bursts required to read all the words, and round it up. the word width may be a fraction, or multiple, of the burst width. the burst count
		 * must always be an integer however, so if it is a fraction, more triangles will be read than are required */

		m_burst_size_in_bytes = max_get_burst_size(maxfile, NULL);
		float words_per_burst = m_burst_size_in_bytes / m_word_width_in_bytes;

		m_total_bursts = (int)ceil((float)m_total_words / words_per_burst);

		/* update the total number of bursts, words and triangles, to match the actual number that will be read by the burst count we have decided on */

		m_total_words = m_total_bursts * words_per_burst;
		m_total_triangles = m_total_words * m_triangles_per_word;

		m_triangles_size_in_bytes = m_total_bursts * m_burst_size_in_bytes;

		/* and finally allocate space for the actual triangles. triangles will be stored in this array with the same layout as they have on the dfe */

		m_triangles = (triangle_t*)malloc(m_triangles_size_in_bytes);
	}

	/* returns a pointer into the triangles array, at which point m_triangles_per_word triangles should be copied in */
	triangle_t* GetTrianglesWord(int word)
	{
		return (triangle_t*)(((char*)m_triangles) + ((int)m_word_width_in_bytes * word));
	}

	triangle_t* GetTriangle(int triangle)
	{
		int word = floor((float)triangle/(float)m_triangles_per_word);
		int offset = triangle % (int)m_triangles_per_word;
		return (GetTrianglesWord(word) + offset);
	}

	void SetTriangles(triangle_t* triangles_src, int triangles_src_count)
	{
		for(int i = 0; i < triangles_src_count; i++)
		{
			*GetTriangle(i) = triangles_src[i];
		}
	}

	void IntialiseTriangles(max_engine_t* engine, int offset_in_bursts)
	{
		max_actions_t* init_act = max_actions_init(m_maxfile, "memoryInitialisation");
		max_set_param_uint64t(init_act, "address", offset_in_bursts * m_burst_size_in_bytes);
		max_set_param_uint64t(init_act, "size", m_triangles_size_in_bytes);
		max_queue_input(init_act,"triangles_in",m_triangles,m_triangles_size_in_bytes);

		max_run(engine, init_act);
	}

};

int main(void)
{
	max_file_t *maxfile = RayTracer_init();
	max_engine_t *engine = max_load(maxfile, "*");

	int num_rays = 16; //5 intersection tests

	/* prepare some triangles */

	/* the triangles data must be a multiple of the burst width of the memory, any spare capacity should be filled with invalid triangles */

	int total_triangles = 16;

	triangle_t* triangles = (triangle_t*)malloc(sizeof(triangle_t) * total_triangles);
	memset(triangles,0,sizeof(triangle_t) * total_triangles);

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

	Triangles* tris = new Triangles(maxfile, total_triangles);
	tris->SetTriangles(triangles, total_triangles);

	/* prepare some rays */

	int rays_size = (sizeof(struct ray_t) * num_rays);
	ray_t* rays = (ray_t*)malloc(rays_size);
	memset(rays,0,rays_size);

	for(int i = 0; i < num_rays; i++)
	{
		if(i > 10){
			rays[i].direction.z = 1;
		}
	}

	/* prepare the output */

	//for now one pcie word per result...

	int total_intersections = num_rays * total_triangles;
	int results_size = (sizeof(result_t) * total_intersections);
	result_t* results = (result_t*)malloc(results_size);
	memset(results,0,results_size);



	/* initialise triangles */
	tris->IntialiseTriangles(engine,0);

	max_actions_t* act = max_actions_init(maxfile, NULL);
	
	int triangles_per_tick = 1;
	int rays_per_tick = 1;
	int rays_in_set = num_rays;
	int triangles_in_set = tris->m_total_triangles;

	int intersection_ticks = (triangles_in_set / triangles_per_tick) * (rays_in_set / rays_per_tick);
	int memory_command_ticks = (rays_in_set / rays_per_tick);

	max_set_ticks(act, "MemoryCommandGenerator", memory_command_ticks);
	max_set_uint64t(act,"MemoryCommandGenerator","triangles_to_read_in_bursts",tris->m_total_bursts);

	max_ignore_lmem(act,"triangles_to_mem");
	max_set_ticks(act, "RayTracerKernel", intersection_ticks);
	max_set_uint64t(act,"RayTracerKernel","total_triangles",triangles_in_set);
	max_queue_input(act, "rays_in", rays, rays_size);
	max_queue_output(act, "results_out", results, results_size);



	printf("Running on DFE...\n");

	max_run(engine, act);

	max_unload(engine);
	
	// TODO Use result data
	for(int i = 0; i < total_intersections; ++i)
	{
		printf("%i: %i\n", i, results[i].result > 0 || results[i].padding > 0);
	}

	printf("Done.\n");
	return 0;
}
