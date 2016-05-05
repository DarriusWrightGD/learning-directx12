#pragma once
#include <Export.h>

class GameTimer
{
public:
	ENGINE_SHARED GameTimer();
	ENGINE_SHARED ~GameTimer();

	ENGINE_SHARED float TotalTime()const;
	ENGINE_SHARED float DeltaTime()const;
	
	ENGINE_SHARED void Reset();
	ENGINE_SHARED void Start();
	ENGINE_SHARED void Stop();
	ENGINE_SHARED void Tick();
	


private:
	double mSecondsPerCount;
	double mDeltaTime;

	__int64 mBaseTime;
	__int64 mPausedTime;
	__int64 mStopTime;
	__int64 mPreviousTime;
	__int64 mCurrentTime;

	bool mStopped;
};

