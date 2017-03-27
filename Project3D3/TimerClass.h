#ifndef TIMERCLASS_H
#define TIMERCLASS_H

class TimerClass
{
private:
	double secondsPerCount;
	double currentDeltaTime;

	__int64 baseTime;
	__int64 pausedTime;
	__int64 previousTime;
	__int64 stoppedTime;
	__int64 currentTime;

	bool stopped;

public:
	//Initializing the Timer.
	TimerClass();


	float time();
	float deltaTime();

	void Tick();
	void Reset();
	void Start();
	void Stop();

};



#endif