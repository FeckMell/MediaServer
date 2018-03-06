#pragma once
#include "stdafx.h"
#include "SL_Socket.h"

class COM
{
public:

	static void Init();

	static void RegisterModul(string name_, string type_, string params_);
	static void BindFunction(string name_, boost::function<void(boost::system::error_code, size_t)> function_);

	static void SendToModul(string where_, string what_);

	static string PrintAll();

private:

	static map<string, SHP_SOCKET> _mapsockets;
	static map<string, boost::signals2::signal<void(string)>> _signals;

};