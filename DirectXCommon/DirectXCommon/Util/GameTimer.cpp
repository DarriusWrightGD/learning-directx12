#include "GameTimer.h"
#include <Windows.h>


GameTimer::GameTimer() : mSecondsPerCount(0.0), mDeltaTime(0.0), mBaseTime(0),
mPausedTime(0), mPreviousTime(0), mCurrentTime(0), mStopped(false)
{
	__int64 countsPerSec;
	QueryPerformanceFrequency((LARGE_INTEGER*)&countsPerSec);
	mSecondsPerCount = 1.0 / (double)countsPerSec;
}


GameTimer::~GameTimer()
{
}

float GameTimer::TotalTime() const
{

	if (mStopped)
	{
		return (float)(((mStopTime - mPausedTime) - mBaseTime)*mSecondsPerCount);
	}
	else
	{
		return (float)(((mCurrentTime - mPausedTime) - mBaseTime)* mSecondsPerCount);
	}
}

float GameTimer::DeltaTime() const
{
	return (float)mDeltaTime;
}

void GameTimer::Reset()
{
	__int64 currTime;
	QueryPerformanceCounter((LARGE_INTEGER*)&currTime);
	mBaseTime = currTime;
	mPreviousTime = currTime;
	mStopTime = 0;
	mStopped = false;
}

void GameTimer::Start()
{
	__int64 startTime;
	QueryPerformanceCounter((LARGE_INTEGER*)&startTime);

	if (mStopped)
	{
		mPausedTime += (startTime - mStopTime);
		mPreviousTime = startTime;

		mStopTime = 0;
		mStopped = false;
	}
}

void GameTimer::Stop()
{
	if (!mStopped)
	{
		__int64 currTime;
		QueryPerformanceCounter((LARGE_INTEGER*)&currTime);
		mStopTime = currTime;
		mStopped = true;
	}
}

void GameTimer::Tick()
{
	if (mStopped)
	{
		mDeltaTime = 0.0;
		return;
	}

	__int64 currentTickTime;
	QueryPerformanceCounter((LARGE_INTEGER*)&currentTickTime);
	mCurrentTime = currentTickTime;

	mDeltaTime = (mCurrentTime - mPreviousTime)* mSecondsPerCount;

	mPreviousTime = mCurrentTime;

	if (mDeltaTime < 0.0)
	{
		mDeltaTime = 0.0;
	}

}
