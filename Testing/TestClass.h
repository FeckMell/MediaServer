#include "stdafx.h"

class TESTDATA
{
public:
	string q = "param q";
};
typedef shared_ptr<TESTDATA> SHP_TD;

class Test1// mgcp proceed
{
public:

	boost::function<void(string, int, boost::any)> Register()
	{
		return boost::bind(&Test1::Proceed, this, _1, _2, _3);
	}


	void Proceed(string mess_, int num_, boost::any td_)
	{
		SHP_TD td = boost::any_cast<SHP_TD>(td_);
		cout << "\nname=Test1 mess=" << mess_ << " num=" << num_ << " q=" << q;
		cout << "\ntd_=" << td->q;
		q++;
	}
private:

	static int q;

};
int Test1::q = 0;
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
class Test2// mgcp proceed
{
public:

	boost::function<void(string, int, boost::any)> Register()
	{
		return boost::bind(&Test2::Proceed, this, _1, _2,_3);
	}

	void Proceed(string mess_, int num_, boost::any td_)
	{
		SHP_TD td = boost::any_cast<SHP_TD>(td_);
		cout << "\nname=Test2 mess=" << mess_ << " num=" << num_ << " q=" << q;
		cout << "\ntd_=" << td->q;
		q++;
	}
private:

	static int q;
};
int Test2::q = 0;
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
class HOLDER
{
public:

	static void AddModul(string name_, boost::any obj_, boost::function<void(string, int, boost::any)> bindfunc_)
	{
		moduls[name_] = obj_;
		funcs[name_] = bindfunc_;
	}

	static void Execute(string name_, string mess_, int num_, SHP_TD td_)
	{
			funcs[name_](mess_, num_, boost::any(td_));
	}

private:

	static map<string, boost::any> moduls;
	static map<string, boost::function<void(string, int, boost::any)>> funcs;
};
map<string, boost::any> HOLDER::moduls = {};
map<string, boost::function<void(string, int, boost::any)>> HOLDER::funcs = {};
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------