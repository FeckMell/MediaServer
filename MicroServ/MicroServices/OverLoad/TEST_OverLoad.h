#pragma once
#include "../../stdafx.h"
#include "../../SharedLib.h"
#include <random>

namespace OverLoad
{
	class TO
	{
	public:

		TO(int id_);
		SHP_IPL GetIPL();

	private:

		map<string, string> mapData;

	}; typedef shared_ptr<TO> SHP_TO;
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
	class TESTCLASS
	{
	public:

		static void STARTTEST();
		static void Proceed(boost::any param_);
		static void Init();

	private:

		
		static void Start();

		static void AddUser();
		static void AddUser(SHP_TO to_);
		static void DelUser();
		static void DelUser(SHP_IPL ipl_);

		static string modulNameThis;
		static string modulNameTest;

		static map<int, SHP_TO> mapObj;
	};
}