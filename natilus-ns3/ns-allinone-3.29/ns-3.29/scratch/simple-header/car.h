#ifndef CAR_H
#define CAR_H

#include "t-object.h"
#include "function.h"
#include "struct.h" 

#include <vector>

using std::vector;

namespace ns3 {

typedef struct Car
{
	uint32_t startCell; // cell id
	uint32_t endCell;
	uint32_t curCell;
}CAR;

class CarContain
{
public:
	std::string navFunc;
	
	// LOG Setting 
	bool carInfo;

	uint32_t carN;
	
	// Car Direction Info
	uint32_t horizontal;
	uint32_t vertical; 

	// Car Evaluation Info
	double threshTmp;
	double reward;
	double totalDecision;
	double wrongDecision;
	
	Time lastTime;

	// Object Linking
	ObjectContain *oc;

	// Car Object
	vector<CAR> car;
	
	// Function
	CarContain ();
	~CarContain ();
	void Start (void);
	void CarInit (uint32_t cN);

	void Moving (STATE *state);
	void MovFunc (STATE *state);
	void CalReward (uint32_t oldPos, uint32_t newPos);
	void PrintFunc (STATE *state, CAR t_c);

	uint32_t Navigate (STATE *state, CAR t_c);
	
	
	/*********************
	* Modification Below *
	*********************/
	// Algorithm
	// 0. random
	uint32_t RandomNav (CAR t_c);
	// 1. greedy
	uint32_t GreedyNav (STATE *state, CAR t_c);
	// 2. Dijkstra
	// 3. Astar
		
	// Retrieve Function 
	double GetReward (void);
	double GetAccuracy (void);
};

}

#endif
