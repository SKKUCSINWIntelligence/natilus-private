#include "t-object.h"

namespace ns3{

ObjectContain::ObjectContain ()
{
}

ObjectContain::~ObjectContain ()
{
	// memory free
	delete[] object;
	delete[] trackMap;
	delete[] tempMap;
	delete[] zero;
	delete[] room;
	if (obsMod == "multi")
	{
		delete[] loc;	
		delete[] ang;
		delete[] tag;
	}
}

void ObjectContain::CreateObject (OBJECT *obj)
{
	// Object Location Create Random
	int r = rand() % 4;
	if (r == 0)
	{
		obj->x = (rand()%((uint32_t)bound*1000 + 1)) / 1000.0;
		obj->y = bound;
	}
	else if (r == 1)
	{
		obj->x = bound;
		obj->y = (rand()%((uint32_t)bound*1000 + 1)) / 1000.0; 
	}
	else if (r == 2)
	{
		obj->x = (rand()%((uint32_t)bound*1000 + 1)) / 1000.0;
		obj->y = 0;
	}
	else if (r == 3)
	{
		obj->x = 0;
		obj->y = (rand()%((uint32_t)bound*1000 + 1)) / 1000.0;
	}
	obj->vel = vel;
	obj->avgAngle = (rand () % 8 + 1) * (PI/4.0);
	obj->angle = obj->avgAngle;
	Angle (obj);

	// 0701
	obj->occupy = true;
}

void ObjectContain::Start ()
{
	// Object Create
	if (obsMod == "car" || obsMod == "multi")
	{
		object = new OBJECT[objectMax];

		for (uint32_t i=0; i<objectMax; i++)
		{
			OBJECT *obj = &object[i];
			obj->occupy = false;
		}
	}
	else
	{
		object = new OBJECT[objectN];
		objectMax = objectN;

		// Object Initial
		for (uint32_t i=0; i<objectN; i++)
		{
			OBJECT *obj = &object[i];
			// Object Location Create Random
			obj->x = (rand()%((uint32_t)bound*1000 + 1)) / 1000.0;
			obj->y = (rand()%((uint32_t)bound*1000 + 1)) / 1000.0;
			obj->vel = vel;
			obj->avgAngle = (rand () % 8 + 1) * (PI/4.0);
			obj->angle = obj->avgAngle;
			// 0701
			obj->occupy = true;
		}
	}
	
	// If Multi Init loc
	if (obsMod == "multi")
	{
		loc = new uint32_t[(unitN/2) -1];
		ang = new double[3*((unitN/2)-1)];
		tag = new uint32_t[(unitN/2) -1];

		for (uint32_t i=0; i<(unitN/2)-1; i++)
		{
			tag[i] = 0;
		}
		
		if (unitN == 4)
		{
			loc[0] = 10;
			
			ang[0] = 180.0/360.0*2*PI;
			ang[1] = 225.0/360.0*2*PI;
			ang[2] = 270.0/360.0*2*PI;
		}
		else if (unitN == 5)
		{
			loc[0] = 18;

			ang[0] = 180.0/360.0*2*PI;
			ang[1] = 225.0/360.0*2*PI;
			ang[2] = 270.0/360.0*2*PI;
		}
		else if (unitN == 6)
		{
			loc[0] = 7;
			loc[1] = 28;

			ang[0] = 30/360.0*2*PI;
			ang[1] = 45.0/360.0*2*PI;
			ang[2] = 90.0/360.0*2*PI;
			ang[3] = 330.0/360.0*2*PI;
			ang[4] = 225.0/360.0*2*PI;
			ang[5] = 270.0/360.0*2*PI;
		}
		else if (unitN == 7)
		{
			loc[0] = 14;
			loc[1] = 40;

			ang[0] = 0;
			ang[1] = 45/360*2*PI;
			ang[2] = 340/360*2*PI;
			ang[3] = 180/360*2*PI;
			ang[4] = 225/360*2*PI;
			ang[5] = 270/360*2*PI;
		}
		else if (unitN == 8)
		{
			loc[0] = 10;
			loc[1] = 38; 
			loc[2] = 49;
			
			ang[0] = 0.0/360.0*2*PI;
			ang[1] = 45.0/360.0*2*PI;
			ang[2] = 90.0/360.0*2*PI;
			ang[3] = 180.0/360.0*2*PI;
			ang[4] = 225.0/360.0*2*PI;
			ang[5] = 270.0/360.0*2*PI;
			ang[6] = 0.0/360.0*2*PI;
			ang[7] = 255.0/360.0*2*PI;
			ang[8] = 315.0/360.0*2*PI;
		}
		else if (unitN == 10)
		{
			loc[0] = 11;
			loc[1] = 48; 
			loc[2] = 55;
			loc[3] = 82;

			ang[0] = 30.0/360.0*2*PI;
			ang[1] = 90.0/360.0*2*PI;
			ang[2] = 340.0/360.0*2*PI;
			ang[3] = 90.0/360.0*2*PI;
			ang[4] = 180.0/360.0*2*PI;
			ang[5] = 2250.0/360.0*2*PI;
			ang[6] = 45.0/360.0*2*PI;
			ang[7] = 225.0/360.0*2*PI;
			ang[8] = 315.0/360.0*2*PI;
			ang[9] = 0;
			ang[10] = 260.0/360.0*2*PI;
			ang[11] = 330.0/360.0*2*PI;
		}
		else if (unitN == 12)
		{

			loc[0] = 27;
			loc[1] = 43; 
			loc[2] = 77;
			loc[3] = 105;
			loc[4] = 110;
		
			ang[0] = 0.0/360.0*2*PI;
			ang[1] = 45.0/360.0*2*PI;
			ang[2] = 135.0/360.0*2*PI;
			ang[3] = 45.0/360.0*2*PI;
			ang[4] = 90.0/360.0*2*PI;
			ang[5] = 135.0/360.0*2*PI;
			ang[6] = 50.0/360.0*2*PI;
			ang[7] = 200.0/360.0*2*PI;
			ang[8] = 340.0/360.0*2*PI;
			ang[9] = 180.0/360.0*2*PI;;
			ang[10] = 250.0/360.0*2*PI;
			ang[11] = 270.0/360.0*2*PI;	
			ang[12] = 20.0/360.0*2*PI;;
			ang[13] = 260.0/360.0*2*PI;
			ang[14] = 330.0/360.0*2*PI;	
		}
		else if (unitN == 14)
		{
			loc[0] = 46;
			loc[1] = 66; 
			loc[2] = 77;
			loc[3] = 117;
			loc[4] = 150;
			loc[5] = 157;

			ang[0] = 30.0/360.0*2*PI;
			ang[1] = 120.0/360.0*2*PI;
			ang[2] = 340.0/360.0*2*PI;
			ang[3] = 70.0/360.0*2*PI;
			ang[4] = 110.0/360.0*2*PI;
			ang[5] = 190.0/360.0*2*PI;
			ang[6] = 20.0/360.0*2*PI;
			ang[7] = 90.0/360.0*2*PI;
			ang[8] = 170.0/360.0*2*PI;
			ang[9] = 0/360.0*2*PI;;
			ang[10] = 200.0/360.0*2*PI;
			ang[11] = 270.0/360.0*2*PI;	
			ang[12] = 160.0/360.0*2*PI;
			ang[13] = 250.0/360.0*2*PI;
			ang[14] = 280.0/360.0*2*PI;
			ang[15] = 0/360.0*2*PI;
			ang[16] = 260.0/360.0*2*PI;
			ang[17] = 290.0/360.0*2*PI;
		}
		else if (unitN == 16)
		{
			loc[0] = 40;
			loc[1] = 76; 
			loc[2] = 83;
			loc[3] = 122;
			loc[4] = 150;
			loc[5] = 188;
			loc[6] = 195;

			ang[0] = 0.0/360.0*2*PI;
			ang[1] = 90.0/360.0*2*PI;
			ang[2] = 180.0/360.0*2*PI;
			ang[3] = 60.0/360.0*2*PI;
			ang[4] = 135.0/360.0*2*PI;
			ang[5] = 210.0/360.0*2*PI;
			ang[6] = 30.0/360.0*2*PI;
			ang[7] = 60.0/360.0*2*PI;
			ang[8] = 315.0/360.0*2*PI;
			ang[9] = 45.0/360.0*2*PI;;
			ang[10] = 135.0/360.0*2*PI;
			ang[11] = 225.0/360.0*2*PI;	
			ang[12] = 20.0/360.0*2*PI;
			ang[13] = 240.0/360.0*2*PI;
			ang[14] = 300.0/360.0*2*PI;
			ang[15] = 180.0/360.0*2*PI;
			ang[16] = 225.0/360.0*2*PI;
			ang[17] = 270.0/360.0*2*PI;
			ang[18] = 30.0/360.0*2*PI;
			ang[19] = 290.0/360.0*2*PI;
			ang[20] = 230.0/360.0*2*PI;
		}
		else if (unitN == 20)
		{
			loc[0] = 63;
			loc[1] = 76; 
			loc[2] = 108;
			loc[3] = 165;
			loc[4] = 193;
			loc[5] = 249;
			loc[6] = 315;
			loc[7] = 323;
			loc[8] = 351;

			ang[0] = 80.0/360.0*2*PI;
			ang[1] = 135.0/360.0*2*PI;
			ang[2] = 180.0/360.0*2*PI;
			ang[3] = 30.0/360.0*2*PI;
			ang[4] = 100.0/360.0*2*PI;
			ang[5] = 340.0/360.0*2*PI;
			ang[6] = 0.0/360.0*2*PI;
			ang[7] = 45.0/360.0*2*PI;
			ang[8] = 135.0/360.0*2*PI;
			ang[9] = 45.0/360.0*2*PI;;
			ang[10] = 120.0/360.0*2*PI;
			ang[11] = 180.0/360.0*2*PI;	
			ang[12] = 160.0/360.0*2*PI;
			ang[13] = 240.0/360.0*2*PI;
			ang[14] = 315.0/360.0*2*PI;
			ang[15] = 30.0/360.0*2*PI;
			ang[16] = 200.0/360.0*2*PI;
			ang[17] = 270.0/360.0*2*PI;
			ang[18] = 180.0/360.0*2*PI;
			ang[19] = 225.0/360.0*2*PI;
			ang[20] = 280.0/360.0*2*PI;
			ang[21] = 15.0/360.0*2*PI;	
			ang[22] = 255.0/360.0*2*PI;
			ang[23] = 315.0/360.0*2*PI;
			ang[24] = 200.0/360.0*2*PI;
			ang[25] = 270.0/360.0*2*PI;
			ang[26] = 315.0/360.0*2*PI;
		}
		else if (unitN == 24)
		{
			loc[0] = 85;
			loc[1] = 125; 
			loc[2] = 186;
			loc[3] = 249;
			loc[4] = 308;
			loc[5] = 317;
			loc[6] = 351;
			loc[7] = 415;
			loc[8] = 451;
			loc[9] = 468;
			loc[10] = 482;

			ang[0] = 60.0/360.0*2*PI;
			ang[1] = 135.0/360.0*2*PI;
			ang[2] = 180.0/360.0*2*PI;
			ang[3] = 0.0/360.0*2*PI;
			ang[4] = 45.0/360.0*2*PI;
			ang[5] = 90.0/360.0*2*PI;
			ang[6] = 135.0/360.0*2*PI;
			ang[7] = 180.0/360.0*2*PI;
			ang[8] = 210.0/360.0*2*PI;
			ang[9] = 30.0/360.0*2*PI;;
			ang[10] = 120.0/360.0*2*PI;
			ang[11] = 315.0/360.0*2*PI;	
			ang[12] = 150.0/360.0*2*PI;
			ang[13] = 210.0/360.0*2*PI;
			ang[14] = 245.0/360.0*2*PI;
			ang[15] = 15.0/360.0*2*PI;
			ang[16] = 700.0/360.0*2*PI;
			ang[17] = 340.0/360.0*2*PI;
			ang[18] = 149.0/360.0*2*PI;
			ang[19] = 180.0/360.0*2*PI;
			ang[20] = 225.0/360.0*2*PI;

			ang[21] = 200.0/360.0*2*PI;	
			ang[22] = 290.0/360.0*2*PI;
			ang[23] = 340.0/360.0*2*PI;
			ang[24] = 180.0/360.0*2*PI;
			ang[25] = 240.0/360.0*2*PI;
			ang[26] = 280.0/360.0*2*PI;
			ang[27] = 0.0/360.0*2*PI;
			ang[28] = 270.0/360.0*2*PI;
			ang[29] = 315.0/360.0*2*PI;
			ang[30] = 20.0/360.0*2*PI;
			ang[31] = 270.0/360.0*2*PI;
			ang[32] = 315.0/360.0*2*PI;
		}
	}
	// Create Map
	trackMap = new double[senN];
	tempMap = new double[senN];

	zero = new double[senN];
	room = new double[senN];
	for (uint32_t i=0; i<senN; i++)
	{
		zero[i] = 0;
		room[i] = 25;
		trackMap[i] = 0;
		tempMap[i] = 25;
	}
	
	if (envMod == "sumo")
		MapUpdateSumo (-100);
	else
		MapUpdate ();
	
	lastTime = Simulator::Now ();
	
	if (obsMod == "multi" && envMod != "sumo")
	{
		Simulator::Schedule (Seconds(1/30.0), &ObjectContain::NewMulti, this, true);
	}
}

void ObjectContain::NewObject (bool reGen)
{
	int r = rand() % objectMax;
	int d = r - (int)objectN;
	
	//std::cout << "CREATE NEW OBJECT " << d << " at " << Simulator::Now().GetSeconds() << std::endl;

	int s = rand() % 4;
	double x = 0, y = 0;
	if (s == 0)
	{
		x = (rand()%((uint32_t)bound*1000 + 1)) / 1000.0;
		y = bound;	
	}
	else if (s == 1)
	{
		x = bound;
		y = (rand()%((uint32_t)bound*1000 + 1)) / 1000.0; 
	}
	else if (s == 2)
	{
		x = (rand()%((uint32_t)bound*1000 + 1)) / 1000.0;
		y = 0;
	}
	else if (s == 3)
	{
		x = 0;
		y = (rand()%((uint32_t)bound*1000 + 1)) / 1000.0;
	}
	
	double min = 0;
	double max = 2.0*PI;
	double min2 = 0;
	double max2 = 2.0*PI;

	uint32_t count = 0;
	int sign = rand()%2;
	if (sign == 0)
		sign = -1;

	double aNew = 0;
	double aAvg = (rand () % 8 + 1) * (PI/4.0);

	while (1)
	{
		bool flag = false;
		if (x == 0 && y == 0)
		{
			aAvg = PI/4.0;
			min = 0;
			max = PI/2.0;
			aNew = aAvg + (sign*rand()%112/10.0)/360.0*2*PI; // (0730) 225 -> 112

		}
		else if (x==0 && y == bound)
		{
			aAvg = 2.0*PI - PI/4.0;
			min = PI + PI/2.0;
			max = 2.0*PI;
			aNew = aAvg + (sign*rand()%112/10.0)/360.0*2*PI;
		}
		else if (x==bound && y==bound)
		{
			aAvg = PI + PI/4.0;
			min = PI;
			max = PI + PI/2.0;
			aNew = aAvg + (sign*rand()%112/10.0)/360.0*2*PI;
		}
		else if (x==bound && y==0)
		{
			aAvg = PI - PI/4.0;
			min = PI/2.0;
			max = PI;
			aNew = aAvg + (sign*rand()%112/10.0)/360.0*2*PI;
		}
		else if (x==0)
		{
			flag = true;
			aAvg = 2.0*PI;
			min = 0;
			max = PI/2.0;
			min2 = PI + PI/2.0;
			max2 = 2.0*PI;
			aNew = aAvg + (sign*rand()%225/10.0)/360.0*2*PI; // (0730) 450 -> 225
		}
		else if (x==bound)
		{
			aAvg = PI;
			min = PI/2.0;
			max = PI + PI/2.0;
			aNew = aAvg + (sign*rand()%225/10.0)/360.0*2*PI;		
		}
		else if (y==0)
		{
			aAvg = PI/2.0;
			min = 0;
			max = PI;
			aNew = aAvg + (sign*rand()%225/10.0)/360.0*2*PI;
		}
		else if (y==bound)
		{
			aAvg = PI + PI/2.0;
			min = PI;
			max = 2.0*PI;
			aNew = aAvg + (sign*rand()%225/10.0)/360.0*2*PI;
		}
		else
		{
			std::cout << "tobject.cc::Angle Error!" << std::endl;
			exit(1);
		}

		if (aNew > 2.0*PI)
			aNew -= 2.0*PI;

		
		if (!flag && (min<aNew && aNew<max))
			break;
		else if (flag && ((min<=aNew && aNew<max) || (min2<aNew && aNew<=max2)))
			break;
		
		if (count++ > 100)
		{
			printf("Object MovFunc Loop Error !\n");
			std::cout << "Count: " << count << std::endl;
			exit (1);
		}
	}
	

	if (d > 0)
	{
		if (d >= 10)
			d = rand() % 5 + 5;
		for (int i=0; i < d; i++)
		{
			if (i >= 10) 
				break;

			for (uint32_t j=0; j<objectMax; j++)
			{
				OBJECT *obj = &object[j];
				if (!obj->occupy)
				{
					//std::cout << "Create New Object" << std::endl;
					obj->x = x;
					obj->y = y;
					obj->vel = vel;
					obj->avgAngle = aAvg; 
					obj->angle = aNew;
					//Angle(obj);

					obj->occupy = true;
					objectN += 1;
					break;
				}
			}
		}
	}

	MapUpdate ();
	if (reGen)
		Simulator::Schedule (Seconds(1/30.0), &ObjectContain::NewObject, this, reGen);
}
/*
void ObjectContain::NewMulti (bool reGen)
{	
	// (0731)
	// Generation Loc is fixed. Should Angle also fixed?
	uint32_t r = rand() % (int) ((unitN/2) - 1);
	uint32_t c = loc[r]; 
	uint32_t xid = c % unitN;
	uint32_t yid = c / unitN;
	double x = cellUnit*xid + cellUnit/2;
	double y = cellUnit*yid + cellUnit/2;
	//double a = (rand() % 360)/360.0*2*PI;
	uint32_t ar = rand() % 3;
	double a = ang[r*3+ar];
		
	int o = rand() % objectMax / 4;
	int d = o - (int) (objectN / 4);
	if (objectN == 0)
	{
		while(true)
		{
			if(d >= 5)
				break;
			o = rand() % objectMax / 4;
			d = o - (int) (objectN / 4);
		}
	}

	if (d >= 5)
	{
		tag[r] = c;
		for (int i=0; i<d; i++)
		{
			if (i >= 10)
			{
				d = 10;
				break;
			}

			for (uint32_t j=0; j<objectMax; j++)
			{
				OBJECT *obj = &object[j];
				if (!obj->occupy)
				{
					//std::cout << "Create New Object" << std::endl;
					obj->x = x;
					obj->y = y;
					obj->vel = vel;
					obj->avgAngle = a; 
					obj->angle = a;

					obj->occupy = true;
					objectN += 1;
					break;
				}
			}
		}

		// Make a	cluster
		uint32_t p = 0; //rand()%4;
		uint32_t cell[3] = {0};

		switch(p) 
		{
			case 0: // only works on this case
				cell[0] = (yid+1)*unitN + xid;
				cell[1] = (yid+1)*unitN + (xid+1);
				cell[2] = yid*unitN + (xid+1);
				break;
			case 1:
				cell[0] = yid*unitN + (xid+1);
				cell[1] = (yid-1)*unitN + (xid+1);
				cell[2] = (yid-1)*unitN + xid;
				break;
			case 2: 
				cell[0] = (yid-1)*unitN + xid;
				cell[1] = (yid-1)*unitN + (xid-1);
				cell[2] = yid*unitN + (xid-1);
				break;
			case 3:
				cell[0] = yid*unitN + (xid-1);
				cell[1] = (yid+1)*unitN + (xid-1);
				cell[2] = (yid+1)*unitN + xid;
				break;
		}

		for (int i=0; i<3; i++)
		{
			uint32_t _xid = cell[i] % unitN;
			uint32_t _yid = cell[i] / unitN;
			double _x = cellUnit*_xid + cellUnit/2;
			double _y = cellUnit*_yid + cellUnit/2;
			uint32_t _d = (rand() % (d-2)) + 1;

			if (objectN >= objectMax)
				break;
			if (objectN+_d >= objectMax)
				break;

			for (uint32_t j=0; j<_d; j++)
			{
				for (uint32_t k=0; k<objectMax; k++)
				{
					OBJECT *obj = &object[k];
					if (!obj->occupy)
					{
						obj->x = _x;
						obj->y = _y;
						obj->vel = vel;
						obj->avgAngle = a; 
						obj->angle = a;
	
						obj->occupy = true;
						objectN += 1;
						break;
					}
				}
			}
		}
	}

	MapUpdate ();	
	if (reGen)
		Simulator::Schedule (Seconds(1/60.0), &ObjectContain::NewMulti, this, reGen);
}*/

void
ObjectContain::NewMulti (bool reGen)
{

	// (0731)
	// Generation Loc is fixed. Should Angle also fixed?
	
	// Select a location
	for (uint32_t k=0; k<2; k++)
	{
		uint32_t r = rand() % (int) ((unitN/2) - 1); 
		uint32_t c = loc[r];

		uint32_t xid = c % unitN;
		uint32_t yid = c / unitN;
		double x = cellUnit*xid + cellUnit/2;
		double y = cellUnit*yid + cellUnit/2;
		
		// Select an Angle
		uint32_t ar = rand() % 3;
		double a = ang[r*3+ar];	

		int d = rand()%7+3;
		if (objectN == 0)
		{
			while(true)
			{
				if(d >= 5)
					break;
				d = (rand()%5)+5;
			}
		}

		//if (d+(d-2)*3 <= (int)(objectMax - objectN))
		if ((int) (objectMax - objectN) >= 30 && d >= 5)
		{
			tag[r] = c;
			objectM += d;
			objectG += 1;
			for (int i=0; i<d; i++)
			{
				for (uint32_t j=0; j<objectMax; j++)
				{
					OBJECT *obj = &object[j];
					if (!obj->occupy)
					{
						//std::cout << "Create New Object" << std::endl;
						obj->x = x;
						obj->y = y;
						obj->vel = vel;
						obj->avgAngle = a; 
						obj->angle = a;

						obj->occupy = true;
						objectN += 1;
						break;
					}
				}
			}

			// Make a	cluster
			uint32_t p = k; //0; //rand()%4;
			if (k==1)
				p = 2;
			uint32_t cell[3] = {0};

			switch(p) 
			{
				case 0: // only works on this case
					cell[0] = (yid+1)*unitN + xid;
					cell[1] = (yid+1)*unitN + (xid+1);
					cell[2] = yid*unitN + (xid+1);
					break;
				case 1:
					cell[0] = yid*unitN + (xid+1);
					cell[1] = (yid-1)*unitN + (xid+1);
					cell[2] = (yid-1)*unitN + xid;
					break;
				case 2: 
					cell[0] = (yid-1)*unitN + xid;
					cell[1] = (yid-1)*unitN + (xid-1);
					cell[2] = yid*unitN + (xid-1);
					break;
				case 3:
					cell[0] = yid*unitN + (xid-1);
					cell[1] = (yid+1)*unitN + (xid-1);
					cell[2] = (yid+1)*unitN + xid;
					break;
			}

			for (int i=0; i<3; i++)
			{
				uint32_t _xid = cell[i] % unitN;
				uint32_t _yid = cell[i] / unitN;
				double _x = cellUnit*_xid + cellUnit/2;
				double _y = cellUnit*_yid + cellUnit/2;
				uint32_t _d = (rand() % (d-2)) + 1;
				
				if (objectN >= objectMax)
					break;
				if (objectN+_d >= objectMax)
					break;
				objectM += _d;
				for (uint32_t j=0; j<_d; j++)
				{
					for (uint32_t k=0; k<objectMax; k++)
					{
						OBJECT *obj = &object[k];
						if (!obj->occupy)
						{
							obj->x = _x;
							obj->y = _y;
							obj->vel = vel;
							obj->avgAngle = a; 
							obj->angle = a;
		
							obj->occupy = true;
							objectN += 1;
							break;
						}
					}
				}
			}
		}
	}

	MapUpdate ();
	if (reGen)
		Simulator::Schedule (Seconds(1/30.0), &ObjectContain::NewMulti, this, reGen);
}

void 
ObjectContain::MapUpdate (void)
{	
	memcpy (trackMap, zero, sizeof(double)*senN);
	memcpy (tempMap, zero, sizeof(double)*senN); // (0729)

	for (uint32_t i=0; i<objectMax; i++)
	{
		OBJECT *obj = &object[i];
		if (obj->occupy)
		{
			double objX = obj->x;
			double objY = obj->y;
			//double objTemp = obj->temp;
			/************
			* Tracking Part
			************/
			uint32_t xId = (uint32_t)(objX)/cellUnit;
			uint32_t yId = (uint32_t)(objY)/cellUnit;
			if (xId == unitN)
				xId -= 1;
			if (yId == unitN)
				yId -= 1;
			uint32_t cellId = yId * unitN + xId;
			trackMap[cellId] += 1; 	

			if (obsMod == "car")
			{
				tempMap[cellId] = 100;
			}

			/************
			* Temperature Part
			************/
			/*
			double objE = cFE * weight * (objTemp - room[cellId]);
			for (uint32_t xid=0; xid<unitN; xid++)
			{
				for (uint32_t yid=0; yid<unitN; yid++)
				{
					uint32_t cell = yid * unitN + xid;
					if (cell == cellId)
						tempMap[cell] += (objTemp - room[cell]);
					else
					{
						double x = cellUnit*xid + cellUnit/2;
						double y = cellUnit*yid + cellUnit/2;
						double distance = std::sqrt(std::pow(objX-x,2) + std::pow(objY-y,2));
						double cellE = objE / std::pow(distance, 2);  // cell energy
						double cellT = cellE / (c02 * 1200); // 1m^3 = 1200g
	
						if (cellT > (objTemp - room[cell]))
							cellT = objTemp - room[cell];
						tempMap[cell] += cellT;
					}
				}
			}
			*/

			// (0729) Temporarily Use Temp Mode for Car...
			if (obsMod == "temp")
			{
				for (uint32_t xid=0; xid<unitN; xid++)
				{
					for (uint32_t yid=0; yid<unitN; yid++)
					{
						uint32_t cell = yid*unitN + xid;
						if (cell == cellId) 
							tempMap[cell] += 10;
						else
						{
							double x = cellUnit*xid + cellUnit/2;
							double y = cellUnit*yid + cellUnit/2;
							double distance = std::sqrt(std::pow(objX-x,2) + std::pow(objY-y,2));
						
							uint32_t cellDist = (xid-xId)*(xid-xId) + (yid-yId)*(yid-yId);
							if (cellDist == 1 || cellDist == 2)
								tempMap[cell] += std::floor(10.0 / distance);
							else
								tempMap[cell] += 0;
						}
					}
				}
			}
		}
	}
	
	if (obsMod == "car") //(0727) For Multi Tracking
	{
		for (uint32_t xid=0; xid<unitN; xid++)
		{
			for (uint32_t yid=0; yid<unitN; yid++)
			{
				uint32_t cell = yid*unitN + xid;
				uint32_t adder = trackMap[cell] * 2;
				uint32_t counter = 1;

				if (!(xid==0 || yid==0 || xid==(unitN-1) || yid==(unitN-1)))
				{
					if (tempMap[cell] == 100)
					{
												
						for (uint32_t x=0; x<unitN; x++)
							for (uint32_t y=0; y<unitN; y++)
							{
								uint32_t neighbor = y*unitN + x;
								uint32_t cellDist = (xid-x)*(xid-x) + (yid-y)*(yid-y);
								uint32_t rander = 0;
								if (adder > 1)
								{
									while (true)
									{
										rander = (rand() % adder) + 1;
										if (rander <= trackMap[cell])
											break;
										//std::cout << rander << trackMap[cell] << std::endl;
									}
								}
								if (cellDist == 1 || cellDist == 2)
								{
									if (counter == 8)
									{
										trackMap[neighbor] += adder;
									}
									else
									{
										trackMap[neighbor] = trackMap[neighbor] + rander; //std::floor(trackMap[cell]/(cellDist*2))
									}

									adder -= rander;
									counter += 1;
								}
							}
					}
				}
				else
				{
					trackMap[cell] += adder;
				}
			}
		}
	}


	// LOG Checking
	//std::cout << (object[0].x) << " " << (object[0].y) << std::endl;
	//PrintState<double> (serId, "Tracking", trackMap, senN);
}

void ObjectContain::MapUpdateSumo(double term)
{
	int* vx ;
  int* vy ;
  int frame = 0;
  int interval = sumo_interval;

  if(term  ==-100)
  {

    //startTime = 150000;
    startTime = rand()%(300000-interval*3000);
    //startTime = rand()%(50000-interval*1500);
    vx = memoryX[startTime];
    vy = memoryY[startTime];
    MapCreateSumo(vx, vy);
    //initialize
  }	
  else
  {
    term += stackedT;
    frame = (int)(term/(0.033/interval));
    stackedT = term -frame*(0.033/interval);
    startTime += frame;

    vx = memoryX[startTime];
    vy = memoryY[startTime];
    MapCreateSumo(vx, vy);
  }
}

void ObjectContain::MapCreateSumo(int* x, int* y)
{

  //std::cout<< x->at((x->size())-2) <<std::endl;
  //std::cout<< y->size() <<std::endl;
  int x_t = 0;
  double y_t = 0;
  int target = 0;
  double node = sqrt(senN);

  memcpy (trackMap, zero, sizeof(double)*senN);
  int ii = 0;

  while (x[ii]!=0)
	{
    x_t  = (double)x[ii];	
    y_t  = (double)y[ii];	

    x_t /= 1000/node;
    y_t /= 1000/node;
    if (x_t>(node-1))
      x_t = (node-1);
    if (y_t>(node-1))
      y_t = (node-1);
    x_t = (int)x_t;
    y_t = (int)y_t;
    y_t = (node-1)-y_t; 
    target = (int)node*y_t + x_t;

    trackMap[target] +=1;
    ii++;
  }


  /*
     std::cout<<"\n##############\n";
     for(int i =0 ;i<144; i++)
     {
     std::cout<<trackMap[i]<<" " ;
     if(i%12 ==11)
     std::cout<<std::endl;
     }
     std::cout<<"\n\n";
     */	 
}

void ObjectContain::Moving (void)
{
	double now = Simulator::Now().GetSeconds ();
	double timeD = now - lastTime.GetSeconds ();

	if (timeD > 0)
	{
		if (envMod == "sumo")
			MapUpdateSumo (timeD);
		else
		{
			for (uint32_t i=0; i<objectMax; i++)
				if (object[i].occupy)
					object[i].timeD = timeD;
			if (obsMod == "car" || obsMod == "multi")
				MovFuncCar ();
			else
				MovFunc ();
			MapUpdate ();
		}

		// Time Update
		lastTime = Simulator::Now();

		// Observation
		if (trace)
		{
			printf("[Moving Trace]::serId:%d\n", serId);
			std::cout << "Time: " << Simulator::Now ().GetMilliSeconds() << std::endl;
			PrintState<double> (trackMap, senN);
		}
	}
}

void ObjectContain::MovFunc (void)
{
	for (uint32_t i=0; i<objectN; i++)
	{
		OBJECT *obj = &object[i];
		double angle = obj->angle;
		double vel = obj->vel;
		double timeD = obj->timeD;

		// Previous (x,y)
		double prev_x = obj->x;
		double prev_y = obj->y;
		// New (x,y)
		double x = prev_x + cos(angle)*vel*timeD;
		double y = prev_y + sin(angle)*vel*timeD;
		
		// Check out of boundary
		if (x < 0 || x > bound || y < 0 || y > bound)
		{
			uint32_t loopCnt = 0;
			do
			{
				//std::cout << "Cur: " << prev_x << "," << prev_y << std::endl; // mkris_log
				//std::cout << "Expect: " << x << "," << y << "\n" <<  std::endl; // mkris_log

				double boundX = -1;
				double boundY = -1;
				// Find bound cross-coordination (x,y)
				double xD = x - prev_x; // x diff
				double yD = y - prev_y; // y diff
				if (xD == 0)
				{
					boundX = x;
					if ( y <= 0 )
						boundY = 0;
					else if ( y >= bound )
						boundY = bound;
					else
					{
						std::cout << "tobject.cc::Bound Error(1)!!" << std::endl;
						exit(1);
					}
				}
				else if (yD == 0)
				{
					boundY = y;
					if ( x <=0 )
						boundX = 0;
					else if ( x >= bound )
						boundX = bound;
					else
					{
						std::cout << "tboject.cc::Bound Error(2)!!" << std::endl;
						exit(1);
					}
				}
				else
				{
					double r_x1;
					double r_x2;
					if (x > prev_x)
					{
						r_x1 = prev_x;
						r_x2 = x;
					}
					else
					{
						r_x1 = x;
						r_x2 = prev_x;
					}

					double tan = yD / xD;
					std::pair<double, double> cross[4];
					cross[0] = std::make_pair(0, prev_y - tan*prev_x);
					cross[1] = std::make_pair( (bound-prev_y) / tan + prev_x, bound);
					cross[2] = std::make_pair( (-1 * prev_y)/tan + prev_x, 0);
					cross[3] = std::make_pair(bound, tan*(bound-prev_x)+prev_y);

					//std::cout << "range x: " << r_x1 << ", " << r_x2 << std::endl; // mkris_log
					for (uint32_t j=0; j<4; j++)
					{
						double t_x = cross[j].first;
						double t_y = cross[j].second;

						//std::cout <<"Expect: " << t_x << ", " << t_y << std::endl; // mkris_log

						if ( (t_x >= r_x1 && t_x <= r_x2) && (t_x >=0 && t_x <= bound && t_y >= 0 && t_y <= bound) )
						{
							boundX = t_x;
							boundY = t_y;
						}
					}
				}
				if ( (boundX == -1) && (boundY == -1) )
				{
					std::cout << "tobject.cc::Bound Error(3)!!" << std::endl;
					exit (1);
				}

				timeD = timeD - std::sqrt(std::pow(prev_x-boundX,2)+std::pow(prev_y-boundY,2)) / vel;
				obj->x = boundX;
				obj->y = boundY;
				angle = Angle (obj); // Get New Angle
				x = boundX + cos(angle)*vel*timeD;
				y = boundY + sin(angle)*vel*timeD;
				// Prepare, new (x,y) out of bound
				prev_x = boundX;
				prev_y = boundY;

				// Check Loop Cnt
				if (loopCnt++ > 20)
				{
					std::cout << "tobject.cc::Bound Error (4)!!: Many Loop Cnt !" << std::endl;
					exit (1);
				}

				//std::cout << "Move: " << x << "," << y << "\n" << std::endl; // mkris_log
			// Check, Otherwise goto loop
			} while ( x < 0 || x > bound || y < 0 || y > bound);
		}
		obj->x = x;
		obj->y = y;
	}
}

void ObjectContain::MovFuncCar (void)
{
	for (uint32_t i=0; i<objectMax; i++)
	{
		OBJECT *obj = &object[i];
		if (obj->occupy)
		{
			double angle = obj->angle;
			double vel = obj->vel;
			double timeD = obj->timeD;

			// Previous (x,y)
			double prev_x = obj->x;
			double prev_y = obj->y;
			// New (x,y)
			double x = prev_x + cos(angle)*vel*timeD;
			double y = prev_y + sin(angle)*vel*timeD;
		
			// Check out of boundary
			if (x < 0 || x > bound || y < 0 || y > bound)
			{
				// Object out of boundary
				obj->occupy = false;
				objectN --;
				//std::cout << "Object Out of Boundary !!" << std::endl;
			}
			obj->x = x;
			obj->y = y;
		}
	}
}
double ObjectContain::Angle (OBJECT *obj)
{
	//double alpha = 0.5;

	double newAngle = 0;
	double x = obj->x;
	double y = obj->y;
	double *avgAngle = &(obj->avgAngle);

	double min = 0;
	double max = 2.0*PI;
	double min2 = 0;
	double max2 = 2.0*PI;

	uint32_t count = 0;

	int sign = rand()%2;
	
	if (sign == 0)
		sign = -1;

	while (1)
	{
		bool flag = false;
		// four point and four edge boundary
		if (x == 0  && y == 0)
		{
			*avgAngle = PI / 4.0;
			min = 0;
			max = PI/2.0;
			newAngle = *avgAngle + (sign*rand()%225/10.0)/360.0*2*PI;
		}
		else if (x == 0 && y == bound)
		{
			*avgAngle = 2.0* PI - PI/4.0;
			min = PI + PI/2.0;
			max = 2.0*PI;
			newAngle = *avgAngle + (sign*rand()%225/10.0)/360.0*2*PI;
		}
		else if (x == bound && y == bound)
		{
			*avgAngle = PI + PI/4.0;
			min = PI;
			max = PI + PI/2.0;
			newAngle = *avgAngle + (sign*rand()%225/10.0)/360.0*2*PI;
		}
		else if (x == bound && y == 0)
		{
			*avgAngle = PI - PI/4.0;
			min = PI/2.0;
			max = PI;
			newAngle = *avgAngle + (sign*rand()%225/10.0)/360.0*2*PI;
		}
		else if (x == 0)
		{
			flag = true;					//CreateObject (obj);
			*avgAngle = 2.0*PI;
			min = 0;
			max = PI/2.0;
			min2 = PI + PI/2.0;
			max2 = 2.0*PI;
			newAngle = *avgAngle + (sign*rand()%450/10.0)/360.0*2*PI;
		}
		else if (x == bound)
		{
			*avgAngle = PI;
			min = PI/2.0;
			max = PI + PI/2.0;
			newAngle = *avgAngle + (sign*rand()%450/10.0)/360.0*2*PI;
		}
		else if (y == 0)
		{
			*avgAngle = PI/2.0;
			min = 0;
			max = PI;
			newAngle = *avgAngle + (sign*rand()%450/10.0)/360.0*2*PI;
		}
		else if (y == bound)
		{
			*avgAngle = PI + PI/2.0;
			min = PI;
			max = 2.0*PI;
			newAngle = *avgAngle + (sign*rand()%450/10.0)/360.0*2*PI;
		}
		else
		{
			std::cout << "tobject.cc::Angel Error !" << std::endl;
			exit (1);
		}

		//double r = GetRandGauss ();
		//newAngle = alpha*(obj->angle) + (1-alpha)*(obj->avgAngle) + sqrt(1-alpha*alpha)*r;

		if (newAngle > 2.0*PI)
			newAngle -= 2.0*PI;

		
		if (!flag && (min<newAngle && newAngle<max))
			break;
		else if (flag && ((min<=newAngle && newAngle<max) || (min2<newAngle && newAngle<=max2)))
			break;

		// Error 
		if (count++ > 10)
		{
			printf("Object MovFunc Loop Error !\n");
			exit (1);
		}
	}
	// Save new angle at the object
	obj->angle = newAngle;
	return newAngle;
}

double ObjectContain::GetRandGauss (void)
{
	double u = ((double)(rand() / (double)RAND_MAX)) * 1.0;
	double v = ((double)(rand() / (double)RAND_MAX)) * 1.0;
	double r = std::sqrt (-2.0*std::log(u)) * std::cos(2.0*PI*v);

	return r;
}


}
