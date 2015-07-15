/*
 * Status.hpp
 *
 *  Created on: 15 Jul 2015
 *      Author: sfriston
 */

#ifndef STATUS_HPP_
#define STATUS_HPP_

#include "Maxfiles.h"
#include "MaxSLiCInterface.h"
#include <errno.h>
#include "Types.h"

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

		if(!max_has_handle_stream(maxfile, "status_out"))
		{
			printf("Maxfile does not have a status_out stream.\n");
			return;
		}

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

	void PrintSummary()
	{
		printf("Intersection Tests Complete\n");
		printf("\tTotal Intersections: %i\n", status_report.intersections);
	}

};

#endif /* STATUS_HPP_ */
