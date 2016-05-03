#pragma once
#include <cstdlib>

inline float clamp(float value, float min, float max)
{
	return value > max ? max : value < min ? min : value;
}

inline float random()
{
	return  static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
}