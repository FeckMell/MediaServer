#pragma once
#include "stdafx.h"
#include "SharedLib.h"
#include "MSann_MusicStore.h"
#include "MSann_Ann.h"


namespace ann
{
	class Control
	{
	public:

		Control();
		void Preprocessing(string);

	private:

		void CR(SHP_IPL);
		void DL(SHP_IPL);

		SHP_Ann FindAnn(string);
		void RemoveAnn(SHP_Ann);

		vector<SHP_Ann> vecAnn;
		SHP_MusicStore musicStore;

	};
	typedef shared_ptr<Control> SHP_Control;
}