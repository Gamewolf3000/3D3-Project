

#include "TimerClass.h"
#include <Windows.h>

//Initalizing
TimerClass::TimerClass()
{
	secondsPerCount = 0.0;
	currentDeltaTime = -1.0;
	baseTime = 0;
	pausedTime = 0;
	previousTime = 0;
	currentTime = 0;
	stopped = false;

	__int64 countsPerSec;
	QueryPerformanceFrequency((LARGE_INTEGER*) &countsPerSec);
	secondsPerCount = 1.0 / countsPerSec;

}

float TimerClass::time()
{
	if (stopped) //If the timer is stopped, we return the total time the application
				 //has been running without being paused
	{
		return (float)(((stoppedTime - pausedTime) - baseTime) * secondsPerCount);
	}
	else
	{
		return (float)(((currentTime - pausedTime) - baseTime) * secondsPerCount);
	}
}

float TimerClass::deltaTime()
{
	return (float)currentDeltaTime;
}

void TimerClass::Tick()
{
	if (stopped)
	{
		currentDeltaTime = 0.0;
		return;
	}
	
	//Check what time this frame happens at
	QueryPerformanceCounter((LARGE_INTEGER*)&currentTime);

	//Check the time since the last frame
	currentDeltaTime = (currentTime - previousTime)*secondsPerCount;

	//Make sure that delta isn't negative
	if (currentDeltaTime < 0)
	{
		currentDeltaTime = 0;
	}

	//Swithc "current time" to "previous time"
	previousTime = currentTime;
}

void TimerClass::Reset()
{
	QueryPerformanceCounter((LARGE_INTEGER*)&currentTime);

	baseTime = currentTime;
	previousTime = currentTime;
	pausedTime = 0.0;
	stoppedTime = 0.0;
	stopped = false;
}

void TimerClass::Start()
{
	if (stopped)
	{
		stopped = false;
		__int64 startTime;
		QueryPerformanceCounter((LARGE_INTEGER*)&startTime);

		//Update how long we have stayed paused
		pausedTime += (startTime - stoppedTime);

		//Change current and previous time to fit the unpause
		currentTime = startTime;

		stoppedTime = 0;
	}
	
}

void TimerClass::Stop()
{
	//If we're already stopped, then we do nothing. However, if that's not the case, then we set
	//the current time as the time we stopped.
	if (!stopped)
	{
		stopped = true;
		QueryPerformanceCounter((LARGE_INTEGER*)&currentTime);
		stoppedTime = currentTime;
	}
}