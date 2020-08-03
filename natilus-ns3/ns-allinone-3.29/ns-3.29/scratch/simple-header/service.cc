#include "service.h"

namespace ns3
{

Service::Service ()
{
}

Service::~Service ()
{
	delete[] carInfo;
}

void Service::Start (void)
{
	carInfo = new int[oc->objectMax];
	for (uint32_t i=0; i<oc->objectMax; i++)
		carInfo[i] = -1;

	if (enable)
	{
		/* Log */
		//std::cout << "Start: " << serId << " / " << cellId << std::endl;
		uint32_t x = cellId % (oc->unitN);
		uint32_t y = cellId / (oc->unitN);

		if (obsMod == "car")
		{	
			if (!(x==0 || y==0 || x==(oc->unitN-1) || y==(oc->unitN-1)))
			{
				SendData ();
			}
		}
		else
		{
			SendData ();
		}
	}
}

// Period Data Collection and Sending
void Service::SendData (void)
{
	if (sendEvent.IsRunning ())
	{
		std::cout << "Service SendData Schedule Error !" << std::endl;
		exit (1);
	}
	// Get Samples
	GetSample ();

	// Memory Management
	// Previous Data Free
	if (saveSample)
	{
		delete saveData;
		saveData = NULL;
	}

	// Data Generator
	DATA *data = new DATA;
	data->dataSize = sampleSize;
	data->genTime = Simulator::Now();
	data->serId = serId;
	data->cellId = cellId;
	data->sampleRate = sampleRate; 
	data->sampleValue = sampleValue;
	if (obsMod == "car" || obsMod == "multi")
	{
		data->carCell = new bool[oc->objectMax];
		
		for (uint32_t i=0; i<oc->objectMax; i++)
		{
			if (carInfo[i] == (int)cellId)
				data->carCell[i] = true;
			else
				data->carCell[i] = false;
		}
	}

	// Whether insert data in TxQ Success ?
	// If TxQ Full, Sample data save in memory (saveData)
	if (!InsertTxQ (data))
	{
		saveSample = true;
		saveData = data;
	}
	else
	{
		saveSample = false;
		saveData = NULL;
	}
	
	// Next Sampling
	eventTime = Simulator::Now ();
	if (firstEvent)
	{
		firstEvent = false;
		double offset = (double) (rand()%1000)/100000;
		sendEvent = Simulator::Schedule (Seconds (1.0/sampleRate+offset), &Service::SendData, this);
	}
	else
	{
		sendEvent = Simulator::Schedule (Seconds (1.0/sampleRate), &Service::SendData, this);	
	}
}

void Service::ReSchedule (void)
{
	Simulator::Cancel (sendEvent);
	uint64_t elapseTime = Simulator::Now().GetMicroSeconds () - eventTime.GetMicroSeconds ();
	uint64_t period = (double)1/sampleRate * 1000000;

	if (elapseTime >= period)
	{
		SendData ();
	}
	else
	{
		period = period - elapseTime;
		sendEvent = Simulator::Schedule (MicroSeconds (period), &Service::SendData, this);	
	}
}

double Service::GetActDelay (void)
{
	if (!totActRecvN)
		return (double) totActDelay / totActRecvN;
	else
		return -1;
}

void Service::GetSample (void)
{
	oc->Moving ();

	if (obsMod == "car" || obsMod == "track" || obsMod == "multi")
	{
		sampleValue = oc->trackMap[cellId];

		if (obsMod == "car" || obsMod == "multi")
		{
			for (uint32_t i=0; i<(oc->objectMax); i++)
			{
				if ((oc->object[i]).occupy)
				{
					uint32_t xId = (uint32_t)(oc->object[i].x)/(oc->cellUnit);
					uint32_t yId = (uint32_t)(oc->object[i].y)/(oc->cellUnit);
					if (xId == (oc->unitN))
						xId -= 1;
					if (yId == (oc->unitN))
						yId -= 1;
					int cellid = (int)(yId * oc->unitN + xId);
					
					carInfo[i] = cellid;
				}
				else
				{
					carInfo[i] = -1;
				}
			}
		}
	}
	else if (obsMod == "temp")
	{
		sampleValue = oc->tempMap[cellId];
	}
	else
	{
		std::cout << "Service::No obsMod !" << std::endl;
		exit (1);
	}
}


}
