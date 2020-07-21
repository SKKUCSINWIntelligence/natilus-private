#include "function.h"
#include "sink.h"
#include "sensor.h"

namespace ns3{

class Link
{
public:
	std::string log;

	// Mod
	bool netMod;

	// Connection
	Sensor *sen;
	Sink *sink;
	bool *sinkHaveSendAction;
	uint32_t **txTable;

	bool *isLinkScheWork;
	uint32_t ssN;
	uint64_t bw; // bps

	uint64_t lasttime = 0;

	void Check (void);
	void SensorRecv (std::queue<DATA*> *dataContain);

	// Callback
	Callback<void, std::queue<DATA*>*, uint64_t> SinkRecvSche;

	// Callback Function
	Callback<void> CallbackCheck (void);
};


}
