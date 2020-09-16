#ifndef TOBJECT_H
#define TOBJECT_H

#include "function.h"

namespace ns3{

typedef struct Tobject
{
	double x;
	double y;
	double vel = 0;
	double angle = 0;
	double avgAngle = 0;
	double temp = 1000;
	
	bool pause = false;
	bool occupy = false;
	double timeD;
}OBJECT;


class ObjectContain
{
public:
	std::string log;

	// LOG Setting
	bool trace;

	// Parameter
	uint32_t serId;
	uint32_t senN;
	uint32_t unitN;
	uint32_t objectN;
	double cellUnit;
	double bound;
	double vel;
	uint32_t objectMax;
	std::string obsMod;

	// Time
	Time lastTime;

	// Object
	OBJECT *object; // Create Array at Start() Func.
	double cFE = 0.0924; // Cal / g C
	double c02 = 0.24; // Cal / g C
	double weight = 200; // g
		
	// Counting
	int objectM = 0;
	int objectG = 0;

	// Map (Observation)
	double *trackMap;
	double *tempMap;
	double *zero; // array 0
	double *room; // array 25 (c)
	uint32_t *loc; // Location for Multi
	double *ang; // Angle for Multi	
	uint32_t *tag; // Tag for NewMulti

	// Funtion
	ObjectContain ();
	~ObjectContain ();
	void CreateObject (OBJECT *obj);
	void Start (void);
	void NewObject (bool);
	void NewMulti (bool);
	void MapUpdate (void);
	
	void Moving (void);
	void MovFunc (void);
	void MovFuncCar (void);
	double Angle (OBJECT *obj);
	double GetRandGauss (void);


};

}

#endif
