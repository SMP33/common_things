#pragma once
#include <time.h>
#include <ctime>
#include <ratio>
#include <chrono>
#include <unistd.h>
#include <pthread.h> 
#include <functional>

namespace common_things
{
	// Timer
	class Time
	{
	private:
		unsigned long long                    point;
		std::chrono::steady_clock::time_point start;

	public:
		Time();
		void               reset();
		/*
		 *first time point
		 **/
		  void               p1();
		/*
		 second time point, returns passed interval
		 **/
		unsigned long long p2();
		/*
		 *currint time in mcs
		 **/
		unsigned long long micros();
		/*
		 *currint time in ms
		 **/
		unsigned long long millis();
	}
	;

	
	// Waiting timeout
	class Timeout
	{
	private:
		Time              time;
		unsigned int       delay;
		unsigned long long lastTick;

	public:
		/*
		 *Restart
		 **/
		void restart();
		/*
		 *d - delay in milliseconds
		 **/
		Timeout(unsigned int d);
		/*
		 *Check if the timeout has come or not
		 **/
		bool         check();
		/*
		 *Time left before timeout. Returns negative value if time is out
		 **/
		long int more();
	}
	;


	class Alarm
	{
	public:
		Alarm();
		~Alarm();		
		/*
		Ставит таймер, который будет вызывать функцию threadFunc с интервалом delay в отдельном потоке.
		threadFunc должна принимать 1 аргумент типа int.
		state - аргумент, который будет передан функции при каждой итерации цикла
		*/
		typedef std::function<void(int)> T;
		void start(T threadFunc, unsigned long long delay, int state);
		/*
		Совершает последний вызов функции с аргументом state
		*/
		void stop(int state);
		/*
		Устанавливает состояние state
		*/
		void setState(int state);
	private:
		Time time;
		pthread_mutex_t mutex;
		pthread_t thread;
		T threadFunc_new;
		T threadFunc;
		void* alarmFunc();
		unsigned long long delay_new;
		unsigned long long delay;
		bool stopFlag;
		int state_new;
		int state;
		void startNew();
		bool funcFlag;
		bool threadFlag;
		bool newThread;
	};
	
}




common_things::Time::Time()
{
	reset();
}

void
common_things::Time::reset()
{

	start = std::chrono::steady_clock::now();
}

void
common_things::Time::p1()
{
	point = micros();
}

unsigned long long
common_things::Time::p2()
{
	return micros() - point;
}

unsigned long long
common_things::Time::micros()
{
	std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();
	return std::chrono::duration_cast<std::chrono::microseconds>(now - start)
	  .count();
}

unsigned long long
common_things::Time::millis()
{
	auto now = std::chrono::steady_clock::now();
	return std::chrono::duration_cast<std::chrono::milliseconds>(
	         std::chrono::steady_clock::now() - start)
	  .count();
}

void
common_things::Timeout::restart()
{
	lastTick = time.millis();
}
common_things::Timeout::Timeout(unsigned int d)
	: delay(d)
{
	
	restart();
}
bool
common_things::Timeout::check()
{
	unsigned long long nowTick = time.millis();
	if (nowTick - lastTick >= delay)
	{
		lastTick = nowTick;
		return true;
	}
	return false;
}

long int
common_things::Timeout::more()
{
	return delay - (time.millis() - lastTick);
}

	
void 
common_things::Alarm::setState(int state)
{
		
	pthread_mutex_lock(&mutex);
	this->state = state;
	pthread_mutex_unlock(&mutex);
}
	
void*
common_things::Alarm::alarmFunc()
{
	pthread_mutex_lock(&mutex);
	threadFlag = true;
	pthread_mutex_unlock(&mutex);
	while (true)
	{
		time.p1();
		pthread_mutex_lock(&mutex);
		bool stopFlag = this->stopFlag;
		funcFlag = true;
		int state = this->state;
		pthread_mutex_unlock(&mutex);
		if (stopFlag)
			break;
			
		threadFunc(state);
			
		pthread_mutex_lock(&mutex);
		stopFlag = this->stopFlag;
		funcFlag = false;
		pthread_mutex_unlock(&mutex);
			
		unsigned long long passed = time.p2();
			
		if (stopFlag)
			break;
		if (passed < delay)
		{
			usleep(delay - passed);
		}
		if (newThread)
			startNew();
	}
	pthread_mutex_lock(&mutex);
	stopFlag = false;
	threadFlag = false;
	pthread_mutex_unlock(&mutex);
}
void
common_things::Alarm::start(T threadFunc, unsigned long long delay, int state)
{
	newThread = true;
	this->threadFunc_new = threadFunc;
	this->delay_new = delay;
	this->state_new = state;
		
	pthread_mutex_lock(&mutex);
	if (!threadFlag)
		startNew();
	pthread_mutex_unlock(&mutex);
}
void
common_things::Alarm::startNew()
{
		
	delay = delay_new;
	threadFunc = threadFunc_new;
	state = state_new;
	auto f =[](void* context)->void * { ((Alarm*)context)->alarmFunc() ; };
	pthread_create(&thread, NULL, f, this);
	newThread = false;
}
	
void
common_things::Alarm::stop(int state)
{
	pthread_mutex_lock(&mutex);
	stopFlag = true;
	this->state = state;
	pthread_mutex_unlock(&mutex);
}
	
common_things::Alarm::Alarm()
{
	pthread_mutex_init(&mutex, NULL);
	state = 1;
	stopFlag = false;
	funcFlag = false;
	threadFlag = false;
	newThread = false;
}

common_things::Alarm::~Alarm()
{
}
