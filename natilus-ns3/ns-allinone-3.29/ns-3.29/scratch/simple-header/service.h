#ifndef SERVICE_H
#define SERVICE_H

#include "function.h"
#include "t-object.h"

namespace ns3{

class Service
{
public:
	std::string log;
	std::string upMod;
	std::string obsMod;

	bool enable = false;

	// Object Link
	ObjectContain *oc;

	// Data 
	uint32_t senId;
	uint32_t serId;
	uint32_t cellId;
	uint32_t sampleSize; // Unit: Byte
	uint32_t sampleRate; // Update Period, Unit: #/s
	double sampleValue;
	int *carInfo;

	// Save Data
	bool saveSample = false;
	DATA *saveData;

	// Measurement
	uint64_t totActRecvN = 0;
	uint64_t totActDelay = 0;

	// Event
	Time eventTime;
	EventId sendEvent;

	// Callback
	Callback<bool, DATA*> InsertTxQ;

	// Functions
	Service();
	~Service();

	void Start (void);
	void SendData (void);
	void ReSchedule (void);
	double GetActDelay (void);
	void GetSample (void);
};

}

#endif
