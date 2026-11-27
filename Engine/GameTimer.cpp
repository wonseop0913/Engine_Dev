#include "pch.h"
#include "GameTimer.h"

GameTimer* GameTimer::GetInstance()
{
	if (s_instance == nullptr)
		s_instance = new GameTimer();
	return s_instance;
}

Bulb::ProcessResult GameTimer::Delete()
{
	if (s_instance != nullptr) {
		delete s_instance;
		s_instance = nullptr;
		return Bulb::ProcessResult::SUCCESS;
	}
	return Bulb::ProcessResult::FAILED_INSTANCE_NOT_FOUND;
}

bool GameTimer::Init()
{
	_secondsPerCount = 0.0;
	_deltaTime = -1.0;
	_baseTime = 0;
	_stopTime = 0;
	_pausedTime = 0;
	_prevTime = 0;
	_currTime = 0;
	_stopped = false;

	__int64 countsPerSec;
	if (!QueryPerformanceFrequency((LARGE_INTEGER*)&countsPerSec))
		return false;
	_secondsPerCount = 1.0 / (double)countsPerSec;

	return true;
}

float GameTimer::TotalTime()const
{
	if( _stopped )
	{
		return (float)(((_stopTime - _pausedTime)-_baseTime)*_secondsPerCount);
	}
	
	else
	{
		return (float)(((_currTime-_pausedTime)-_baseTime)*_secondsPerCount);
	}
}

float GameTimer::DeltaTime()const
{
	return (float)_deltaTime;
}

void GameTimer::Reset()
{
	__int64 currTime;
	QueryPerformanceCounter((LARGE_INTEGER*)&currTime);

	_baseTime = currTime;
	_prevTime = currTime;
	_currTime = currTime;
	_stopTime = 0;
	_stopped  = false;
}

void GameTimer::Start()
{
	__int64 startTime;
	QueryPerformanceCounter((LARGE_INTEGER*)&startTime);

	if( _stopped )
	{
		_pausedTime += (startTime - _stopTime);	

		_prevTime = startTime;
		_stopTime = 0;
		_stopped  = false;
	}
}

void GameTimer::Stop()
{
	if( !_stopped )
	{
		__int64 currTime;
		QueryPerformanceCounter((LARGE_INTEGER*)&currTime);

		_stopTime = currTime;
		_stopped  = true;
	}
}

void GameTimer::Tick()
{
	if( _stopped )
	{
		_deltaTime = 0.0;
		return;
	}

	__int64 currTime;
	QueryPerformanceCounter((LARGE_INTEGER*)&currTime);
	_currTime = currTime;

	_deltaTime = (_currTime - _prevTime)*_secondsPerCount;

	_prevTime = _currTime;

	if(_deltaTime < 0.0)
	{
		_deltaTime = 0.0;
	}
}

GameTimer* GameTimer::s_instance = nullptr;

