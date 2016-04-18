#pragma once
#include "Export.h"

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
	double secondsPerCount;
	double deltaTime;

	__int64 baseTime;
	__int64 pausedTime;
	__int64 stopTime;
	__int64 previousTime;
	__int64 currentTime;

	bool stopped;
};

