#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <unistd.h>

#include "Maxfiles.h"
#include "MaxSLiCInterface.h"
#include <errno.h>

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
};

struct ray_t
{
	struct vector3 origin;
	struct vector3 direction;
	u_int64_t padding;
};

struct result_t
{
	u_int32_t ray_1;
	u_int32_t triangle_1;
	u_int32_t ray_2;
	u_int32_t triangle_2;
};

struct report_t
{
	u_int32_t ticks;
	u_int32_t intersections;
	u_int64_t reserved;
};

class Status
{
private:
	int m_slotSize;
	int m_numSlots;

	max_llstream_t* m_status_stream;

public:

	report_t status_report;

	Status(max_file_t* maxfile, max_engine_t* engine)
	{
		m_slotSize = 16;
		m_numSlots = 64;

		int results_size = m_slotSize * m_numSlots;
		void* results_buffer = NULL;
		if(posix_memalign(&results_buffer, 4096, results_size) == ENOMEM){
			printf("Could not allocate memory.");
		}

		memset(results_buffer, 0, results_size);

		m_status_stream = max_llstream_setup(engine, "status_out", m_numSlots, m_slotSize, results_buffer);
	}

	bool ReadStatus()
	{
		int slots_to_get = 1;
		void* results_data;
		int num_slots_read = max_llstream_read(m_status_stream, slots_to_get, &results_data);

		for(int i = 0; i < num_slots_read; i++)
		{
			status_report = *((report_t*)results_data);
		}

		max_llstream_read_discard(m_status_stream, num_slots_read);

		return (num_slots_read > 0);
	}

};

class Results
{
private:

	int m_slotSize; 	//one pcie word width
	int m_numSlots;

	int m_results_size;
	void* m_results_buffer;

	max_file_t* m_maxfile;

	max_llstream_t* m_results_stream;

public:
	Results(max_file_t* maxfile, max_engine_t* engine)
	{
		m_maxfile = maxfile;

		m_slotSize = 16;
		m_numSlots = 512;

		m_results_size = m_slotSize * m_numSlots;
		m_results_buffer = NULL;
		if(posix_memalign(&m_results_buffer, 4096, m_results_size) == ENOMEM){
			printf("Could not allocate memory.");
		}

		memset(m_results_buffer, 0, m_results_size);

		if(m_maxfile != NULL)
		{
			if(!max_has_handle_stream(m_maxfile, "results_out"))
			{
				printf("Maxfile does not have a results_out stream.\n");
				return;
			}
		}

		m_results_stream = max_llstream_setup(engine, "results_out", m_numSlots, m_slotSize, m_results_buffer);
	}

	void ReadResults()
	{
		int slots_to_get = 1;
		void* results_data;
		int num_slots_read = max_llstream_read(m_results_stream, slots_to_get, &results_data);

		for(int i = 0; i < num_slots_read; i++)
		{
			result_t result = *((result_t*)results_data);
			printf("Ray %i Triangle %i\n", result.ray_1, result.triangle_1);
		}

		max_llstream_read_discard(m_results_stream, num_slots_read);
	}

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

		/* some sanity checks */

		if(!(max_get_constant_uint64t(maxfile,"TriangleWidthInBytes") == sizeof(triangle_t)))
		{
			printf("Mismatch between typedef triangle_t on CPU and DFE");
		}

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

void initialiseTestTriangles(Triangles* tris)
{
	/* prepare some triangles */
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

	tris->SetTriangles(triangles, total_triangles);
}

int main(void)
{
	max_file_t *maxfile = RayTracer_init();
	max_engine_t *engine = max_load(maxfile, "*");



	Triangles* tris = new Triangles(maxfile, 16);
	initialiseTestTriangles(tris);


	/* prepare some rays */

	int num_rays = 16; //5 intersection tests

	int rays_size = (sizeof(struct ray_t) * num_rays);
	ray_t* rays = (ray_t*)malloc(rays_size);
	memset(rays,0,rays_size);

	for(int i = 0; i < num_rays; i++)
	{
		if(i > 10){
			rays[i].direction.z = 1;
		}
		else
		{
			rays[i].direction.z = 1;
			rays[i].direction.x = 100;
		}
	}





	/* initialise triangles */
	tris->IntialiseTriangles(engine,0);

	max_actions_t* act = max_actions_init(maxfile, NULL);
	
	int triangles_per_tick = max_get_constant_uint64t(maxfile, "TrianglesPerTick");
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
	max_set_uint64t(act,"RayTracerKernel","total_rays",rays_in_set);

	max_queue_input(act, "rays_in", rays, rays_size);



	/* prepare the output */

	Results results(maxfile, engine);
	Status status(maxfile, engine);

	printf("Running on DFE...\n");

	max_run_t* max_run = max_run_nonblock(engine, act);

	while(true)
	{
		results.ReadResults();

		if(status.ReadStatus()){
			break;
		}
	}

	max_wait(max_run);
	max_unload(engine);

	printf("Done. Printing results...\n");

	//print the results



	

	printf("Done.\n");
	return 0;
}
