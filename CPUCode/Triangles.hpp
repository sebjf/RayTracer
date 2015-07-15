/*
 * Triangles.hpp
 *
 *  Created on: 15 Jul 2015
 *      Author: sfriston
 */

#ifndef TRIANGLES_HPP_
#define TRIANGLES_HPP_

#include "Maxfiles.h"
#include "MaxSLiCInterface.h"
#include <errno.h>
#include "Types.h"


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

#endif /* TRIANGLES_HPP_ */
