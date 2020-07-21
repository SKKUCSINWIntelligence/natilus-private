#include "car.h"

namespace ns3 {

CarContain::CarContain ()
{
}

CarContain::~CarContain ()
{
	// memory free
}

void CarContain::Start (void)
{
	// Car Initial
	CarInit (carN); 

	// Eval Info Initial
	reward = 0;
	totalDecision = 0;
	wrongDecision = 0;
}

void CarContain::CarInit (uint32_t cN)
{
	for (uint32_t i=0; i<cN; i++)
	{
		CAR c;
		c.startCell = (rand () % (oc->senN));
		c.curCell = c.startCell; 

		while (true)
		{
			c.endCell = (rand  () % (oc->senN));
			if (c.endCell != c.startCell) break;
		}
		car.insert (car.begin (), c);
	}
}
void CarContain::Moving (STATE *state)
{
	MovFunc (state);
}

void CarContain::MovFunc (STATE *state)
{
	// Clear the Reward
	reward = 0; 

	for (uint32_t i=0; i<carN; i++)
	{
		CAR t_c = car.back ();
		car.pop_back ();

		uint32_t unitN = oc->unitN;

		// Get Current x,y
		uint32_t curX = t_c.curCell % unitN;
		uint32_t curY = t_c.curCell / unitN;
		
		// Choose direction
		uint32_t direction = Navigate (state, t_c);
		/* 0: stay, 1: right, 2: down, 3: left, 4: up */
		switch (direction)
		{
			case 0:
				break;
			case 1: 
				curX += 1;
				break;
			case 2:
				curY -= 1;
				break;
			case 3:
				curX -= 1;
				break;
			case 4:
				curY += 1;
				break;
		}
	
		// Check Reward 
		CalReward (t_c.curCell, curY * unitN + curX);

		// Retrieve Cell Num
		t_c.curCell = curY * unitN + curX;	
	
		// Print Car Map
		if (carInfo)
		{
			PrintFunc (state, t_c);
		}

		// Check If Car Arrived
		if (t_c.curCell == t_c.endCell)
		{	
			//carN -= 1;
			CarInit (1);

			if (carInfo)
			{
				std::cout << "CAR ARRIVED !!" << carN << std::endl;	
			}
		}
		else
		{
			car.insert(car.begin(), t_c);
		}
	}
}

void CarContain::CalReward (uint32_t oldPos, uint32_t newPos)
{
	uint32_t unitN = oc->unitN;

	// Increae Total Decision Made
	totalDecision += 1;

	if (oldPos == newPos)
	{
		uint32_t curX = oldPos % unitN;
		uint32_t curY = oldPos / unitN;
		
		double horizontalTruth = 0;
		double verticalTruth = 0;

		if (horizontal == 1)
			horizontalTruth = oc->tempMap[unitN * curY + (curX + 1)];
		else if (horizontal == 3)
			horizontalTruth = oc->tempMap[unitN * curY + (curX - 1)];

		if (vertical == 2)
			verticalTruth = oc->tempMap[unitN * (curY - 1) + curX];
		else if (vertical == 4)
			verticalTruth = oc->tempMap[unitN * (curY + 1) + curX];
		
		// If car does not move, althogh it can move, then get penalty
		// Otherwise no penalty
		if (horizontal == 0) 
		{
			if (verticalTruth <= threshTmp)
			{
				reward -= 1;
				wrongDecision += 1;
			}
			else
			{
				reward += 0;
			}
		}
		else if (vertical == 0)
		{
			if (horizontalTruth <= threshTmp)
			{
				reward -= 1;
				wrongDecision += 1;
			}
			else
			{
				reward += 0;
			}
		}
		else
		{
			if (horizontalTruth <= threshTmp || verticalTruth <= threshTmp)
			{
				reward -= 1;
				wrongDecision += 1;
			}
			else
			{
				reward += 0;
			}
		}
	}
	else
	{	
		// If car move to high temperature, then get penalty
		// Otherwise no penalty
		if (oc->tempMap[newPos] >= threshTmp)
		{
			reward -= 1;
			wrongDecision += 1;
		}
		else
		{
			reward += 0;
		}
	}
}

void CarContain::PrintFunc (STATE* state, CAR t_c)
{
	uint32_t unitN = oc->unitN;

	std::cout << "[[Observed Map]]" << std::endl;
	for (uint32_t i=0; i<unitN; i++)
	{
		for (uint32_t j=0; j<unitN; j++)
		{
			std::cout << state->sampleValue[unitN * (unitN - i - 1) + j] << " ";
		}
		std::cout << std::endl; 
	}
	std::cout << std::endl;

	std::cout << "[[Car Map]]" << std::endl;
	for (uint32_t i=unitN; i>0; i--)
	{ 
		for (uint32_t j=0; j<unitN; j++)
		{
			if ((unitN * (unitN - i) + j) == t_c.curCell)
			{
				std::cout << "1 ";
			}
			else
			{
				std::cout << "0 "; 	
			}
		}
		std::cout << std::endl;
	}
	std::cout << std::endl;
}

uint32_t CarContain::Navigate (STATE* state, CAR t_c)
{
	if (navFunc == "Random")
	{
		return RandomNav (t_c);
	}
	else if (navFunc == "Greedy")
	{
		return GreedyNav (state, t_c);
	}
	else
	{
		return 1;
	}
}

uint32_t CarContain::RandomNav (CAR t_c)
{
	uint32_t unitN = oc->unitN;
	
	// Get Current x,y
	uint32_t curX = t_c.curCell % unitN;
	uint32_t curY = t_c.curCell / unitN;
		
	// Get End x,y
	uint32_t endX = t_c.endCell % unitN;
	uint32_t endY = t_c.endCell / unitN;

	/* 0: no move, 1: right, 2: down, 3: left, 4: up */
	horizontal =  0; // 1 or 3
	vertical = 0;		// 2 or 4

	if (curX < endX)
	{
		horizontal = 1;
	}
	if (curX > endX)
	{
		horizontal = 3;
	}

	if (curY < endY)
	{
		vertical = 4;
	}
	if (curY > endY)
	{
		vertical = 2;
	}
	
	// Decide Direction
	if (horizontal == 0)
	{
		return vertical;
	}
	if (vertical == 0)
	{
		return horizontal;
	}
	if (rand()%2)
	{
		return vertical;
	}
	else
	{
		return horizontal;
	}		
}

uint32_t CarContain::GreedyNav (STATE* state, CAR t_c)
{
	uint32_t unitN = oc->unitN;
		
	// Get Current x,y
	uint32_t curX = t_c.curCell % unitN;
	uint32_t curY = t_c.curCell / unitN;
		
	// Get End x,y
	uint32_t endX = t_c.endCell % unitN;
	uint32_t endY = t_c.endCell / unitN;
	
	/* 0: no move, 1: right, 2: down, 3: left, 4: up */
	horizontal =  0; // 1 or 3
	double horizontalTmp = 0; 
	vertical = 0;		// 2 or 4
	double verticalTmp = 0;

	if (curX < endX)
	{
		horizontal = 1;
		horizontalTmp = state->sampleValue[unitN * curY + (curX + 1)];
	}
	if (curX > endX)
	{
		horizontal = 3;
		horizontalTmp = state->sampleValue[unitN * curY + (curX - 1)];
	}

	if (curY < endY)
	{
		vertical = 4;
		verticalTmp = state->sampleValue[unitN * (curY + 1) + curX];
	}
	if (curY > endY)
	{
		vertical = 2;
		verticalTmp = state->sampleValue[unitN * (curY -1) + curX];
	}

	if (horizontal == 0)
	{
		if (verticalTmp >= threshTmp) return 0;
		else return vertical;
	}
	if (vertical == 0)
	{
		if (horizontalTmp >= threshTmp) return 0;
		else return horizontal;
	}

	if (verticalTmp >= threshTmp && horizontalTmp >= threshTmp)
	{
		return 0;
	}
	if (verticalTmp >= threshTmp)
	{
		return horizontal;
	}
	if (horizontalTmp >= threshTmp)
	{
		return vertical;
	}

	if (rand() % 2)
	{
		return vertical;
	}
	else
	{
		return horizontal;
	}
}

double CarContain::GetReward (void)
{
	if (carN == 0)
		return 1;
	else
		return reward / carN;
}

double CarContain::GetAccuracy (void) 
{
	if (totalDecision != 0)
		return wrongDecision / totalDecision;
	else
		return 0;
}
}
