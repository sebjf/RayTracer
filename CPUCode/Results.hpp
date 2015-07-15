/*
 * Results.hpp
 *
 *  Created on: 15 Jul 2015
 *      Author: sfriston
 */

#ifndef RESULTS_HPP_
#define RESULTS_HPP_

#include "Maxfiles.h"
#include "MaxSLiCInterface.h"
#include <errno.h>
#include "Types.h"
#include <vector>

struct result_t
{
	u_int32_t ray_1;
	u_int32_t triangle_1;
	u_int32_t ray_2;
	u_int32_t triangle_2;
};

class Results
{
public:
	std::vector<intersection_t> m_intersections;

private:

	int m_slotSize; 	//one pcie word width
	int m_numSlots;

	int m_results_buffer_size;
	void* m_results_buffer;

	max_file_t* m_maxfile;

	max_llstream_t* m_results_stream;

public:
	Results(max_file_t* maxfile, max_engine_t* engine)
	{
		m_maxfile = maxfile;

		m_slotSize = 16;
		m_numSlots = 512;

		m_results_buffer_size = m_slotSize * m_numSlots;
		m_results_buffer = NULL;
		if(posix_memalign(&m_results_buffer, 4096, m_results_buffer_size) == ENOMEM){
			printf("Could not allocate memory.");
		}

		memset(m_results_buffer, 0, m_results_buffer_size);

		if(!max_has_handle_stream(m_maxfile, "results_out"))
		{
			printf("Maxfile does not have a results_out stream.\n");
			return;
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

			intersection_t intersection_1;
			intersection_1.ray = result.ray_1;
			intersection_1.triangle = result.triangle_1;
			intersection_t intersection_2;
			intersection_2.ray = result.ray_2;
			intersection_2.triangle = result.triangle_2;

			m_intersections.push_back(intersection_1);
			m_intersections.push_back(intersection_2);

		}

		max_llstream_read_discard(m_results_stream, num_slots_read);
	}

	void PrintResults()
	{
		for(uint i = 0; i < m_intersections.size(); i++)
		{
			printf("Ray: %i Triangle: %i\n", m_intersections[i].ray, m_intersections[i].triangle);
		}
	}

};

#endif /* RESULTS_HPP_ */
