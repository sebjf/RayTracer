/*
 * Types.h
 *
 *  Created on: 15 Jul 2015
 *      Author: sfriston
 */

#ifndef TYPES_H_
#define TYPES_H_

struct vector3
{
	float x;
	float y;
	float z;

	vector3(float x, float y, float z)
	{
		this->x = x;
		this->y = y;
		this->z = z;
	}

	vector3()
	{

	}
};

struct ray_t
{
	struct vector3 origin;
	struct vector3 direction;
};

struct triangle_t
{
	struct vector3 v0;
	struct vector3 v1;
	struct vector3 v2;
};

struct intersection_t
{
	u_int32_t ray;
	u_int32_t triangle;

	bool operator==(const intersection_t& rhs)
	{
	    return memcmp(this, &rhs, sizeof(intersection_t)) == 0;
	}
};


#endif /* TYPES_H_ */
