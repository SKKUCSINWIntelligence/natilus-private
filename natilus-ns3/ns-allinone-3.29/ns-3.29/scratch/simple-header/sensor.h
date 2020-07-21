#ifndef SENSOR_H
#define SENSOR_H

#include "service.h"
#include "function.h"

namespace ns3{

class Sensor
{
public:
	std::string log;

	uint32_t sensorId;
	uint32_t serviceN;
	Service* ser;	

	uint32_t qMaxSize;
	std::queue<DATA*> txQ;
	uint32_t rr_idx = 0; // Service Access Round Robin Index

	// Callback
	Callback<void> LinkCheck;

	// Link Schedule Info
	bool *isLinkScheWork;
	
	// Functions
	Sensor ();
	~Sensor ();

	void Start (void);
	void ServiceListGen (void);

	bool InsertTxQ (DATA *m_data);
	bool FillTxQ ();

	// Callback Function
	Callback<bool, DATA*> CallbackInsertTxQ (void);

};


}
#endif
