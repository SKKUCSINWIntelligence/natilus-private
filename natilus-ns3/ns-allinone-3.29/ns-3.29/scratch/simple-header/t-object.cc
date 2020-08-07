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
		
		if (unitN == 6)
		{
			loc[0] = 7;
			loc[1] = 16;
		}
		else if (unitN == 8)
		{
			loc[0] = 18;
			loc[1] = 29; 
			loc[2] = 43;
			
			ang[0] = 35.0/360.0*2*PI;
			ang[1] = 105.0/360.0*2*PI;
			ang[2] = 200.0/360.0*2*PI;
			ang[3] = 220.0/360.0*2*PI;
			ang[4] = 145.0/360.0*2*PI;
			ang[5] = 90.0/360.0*2*PI;
			ang[6] = 12.0/360.0*2*PI;
			ang[7] = 110.0/360.0*2*PI;
			ang[8] = 255.0/360.0*2*PI;
		}
		else if (unitN == 10)
		{
			loc[0] = 34;
			loc[1] = 57; 
			loc[2] = 62;
			loc[3] = 85;

			ang[0] = 45.0/360.0*2*PI;
			ang[1] = 135.0/360.0*2*PI;
			ang[2] = 225.0/360.0*2*PI;
			ang[3] = 100.0/360.0*2*PI;
			ang[4] = 190.0/360.0*2*PI;
			ang[5] = 270.0/360.0*2*PI;
			ang[6] = 30.0/360.0*2*PI;
			ang[7] = 90.0/360.0*2*PI;
			ang[8] = 345.0/360.0*2*PI;
			ang[9] = 0;
			ang[10] = 180.0/360.0*2*PI;
			ang[11] = 270.0/360.0*2*PI;
		}
		else if (unitN == 12)
		{
			loc[0] = 39;
			loc[1] = 57; 
			loc[2] = 78;
			loc[3] = 116;
			loc[4] = 124;
		
			ang[0] = 10.0/360.0*2*PI;
			ang[1] = 50.0/360.0*2*PI;
			ang[2] = 340.0/360.0*2*PI;
			ang[3] = 100.0/360.0*2*PI;
			ang[4] = 180.0/360.0*2*PI;
			ang[5] = 240.0/360.0*2*PI;
			ang[6] = 70.0/360.0*2*PI;
			ang[7] = 170.0/360.0*2*PI;
			ang[8] = 270.0/360.0*2*PI;
			ang[9] = 120.0/360.0*2*PI;;
			ang[10] = 200.0/360.0*2*PI;
			ang[11] = 280.0/360.0*2*PI;	
			ang[12] = 10.0/360.0*2*PI;;
			ang[13] = 190.0/360.0*2*PI;
			ang[14] = 340.0/360.0*2*PI;	
		}
		else if (unitN == 14)
		{
			loc[0] = 25;
			loc[1] = 31; 
			loc[2] = 65;
			loc[3] = 89;
			loc[4] = 119;
			loc[5] = 136;
		}
		else if (unitN == 16)
		{
			loc[0] = 26;
			loc[1] = 68; 
			loc[2] = 98;
			loc[3] = 107;
			loc[4] = 119;
			loc[5] = 163;
			loc[6] = 204;
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

	MapUpdate ();
	lastTime = Simulator::Now ();
	
	if (obsMod == "car")
	{
		Simulator::Schedule (Seconds(1/30.0), &ObjectContain::NewObject, this, false);
	}
	if (obsMod == "multi")
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
			case 0:
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
		Simulator::Schedule (Seconds(1/30.0), &ObjectContain::NewMulti, this, reGen);
}

void ObjectContain::MapUpdate (void)
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

void ObjectContain::Moving (void)
{
	double now = Simulator::Now().GetSeconds ();
	double timeD = now - lastTime.GetSeconds ();

	if (timeD > 0)
	{
		for (uint32_t i=0; i<objectMax; i++)
			if (object[i].occupy)
				object[i].timeD = timeD;
		if (obsMod == "car" || obsMod == "multi")
			MovFuncCar ();
		else
			MovFunc ();
		MapUpdate ();

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
