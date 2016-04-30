#pragma once

inline float clamp(float value, float min, float max)
{
	return value > max ? max : value < min ? min : value;
}