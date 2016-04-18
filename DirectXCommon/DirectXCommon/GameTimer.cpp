#include "GameTimer.h"
#include <Windows.h>


GameTimer::GameTimer() : secondsPerCount(0.0), deltaTime(0.0), baseTime(0),
pausedTime(0), previousTime(0), currentTime(0), stopped(false)
{
	__int64 countsPerSec;
	QueryPerformanceFrequency((LARGE_INTEGER*)&countsPerSec);
	secondsPerCount = 1.0 / (double)countsPerSec;
}


GameTimer::~GameTimer()
{
}

float GameTimer::TotalTime() const
{

	if (stopped)
	{
		return (float)(((stopTime - pausedTime) - baseTime)*secondsPerCount);
	}
	else
	{
		return (float)(((currentTime - pausedTime) - baseTime)* secondsPerCount);
	}
}

float GameTimer::DeltaTime() const
{
	return (float)deltaTime;
}

void GameTimer::Reset()
{
	__int64 currTime;
	QueryPerformanceCounter((LARGE_INTEGER*)&currTime);
	baseTime = currTime;
	previousTime = currTime;
	stopTime = 0;
	stopped = false;
}

void GameTimer::Start()
{
	__int64 startTime;
	QueryPerformanceCounter((LARGE_INTEGER*)&startTime);

	if (stopped)
	{
		pausedTime += (startTime - stopTime);
		previousTime = startTime;

		stopTime = 0;
		stopped = false;
	}
}

void GameTimer::Stop()
{
	if (!stopped)
	{
		__int64 currTime;
		QueryPerformanceCounter((LARGE_INTEGER*)&currTime);
		stopTime = currTime;
		stopped = true;
	}
}

void GameTimer::Tick()
{
	if (stopped)
	{
		deltaTime = 0.0;
		return;
	}

	__int64 currentTickTime;
	QueryPerformanceCounter((LARGE_INTEGER*)&currentTickTime);
	currentTime = currentTickTime;

	deltaTime = (currentTime - previousTime)* secondsPerCount;

	previousTime = currentTime;

	if (deltaTime < 0.0)
	{
		deltaTime = 0.0;
	}

}
