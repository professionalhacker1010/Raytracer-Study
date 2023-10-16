#pragma once
#include "Math.h"
#include "util.h"
#include <string>

struct Vec3Test {
	static void ScopeTest1(Vec3* v) {
		v->Set(3., 2., 1.);
	}

	static void ScopeTest2(Vec3* v) {
		Vec3 temp = Vec3(4., 5., 6.);
		v = &temp;
	}

	static void ScopeTest3(Vec3* v) {
		Vec3 temp = Vec3(4., 5., 6.);
		*v = temp;
	}

	static void Run() {
		Vec3 v1(1., 2., 3.);
		Util::Print("component constructor " + (std::string)v1);

		Vec3 v2(v1);
		Util::Print("copy constructor " + (std::string)v2);

		Vec3 v3;
		Util::Print("default constructor " + (std::string)v3);

		v2 = Vec3(3., 2., 1.);
		Util::Print("= operator " + (std::string)v2);

		Util::Print("+ operator " + (std::string)(v2 + v1));

		Util::Print("- operator " + (std::string)(v2 - v1));

		Util::Print("* operator Vec3" + (std::string)(v2 * v1));

		Util::Print("* operator scalar" + (std::string)(v2 * 6.));

		Util::Print("[] operator " + std::to_string(v2[0]));

		v1.Normalize();
		Util::Print("normalize " + (std::string)v1);

		Util::Print("Magnitude " + std::to_string(v1.Length()));

		//pointers
		Vec3* p1 = new Vec3(1., 2., 3.);
		Util::Print("allocated " + (std::string)*p1);

		ScopeTest1(p1); //321
		Util::Print("Modify pass by pointer " + (std::string)*p1);

		ScopeTest2(p1); //should fail
		Util::Print("bad modify pass by pointer " + (std::string)*p1);

		ScopeTest3(p1); //456
		Util::Print("Modify pass by pointer " + (std::string)*p1);
	}
};

