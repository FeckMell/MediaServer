#pragma once
#include "stdafx.h"

class TIME
{
public:
	TIME();

	chrono::steady_clock::time_point GetLast();
	chrono::steady_clock::time_point GetStart();
	chrono::duration<int> GetFullDuration();
	chrono::duration<int> GetActiveDuration();
	chrono::duration<int> GetInactiveTime();

	void Update();

private:

	chrono::steady_clock::time_point start;
	chrono::steady_clock::time_point last;

};