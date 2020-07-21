#include "link.h"

namespace ns3{

void Link::Check (void)
{
	if (*isLinkScheWork)
	{
		// Schdule Wrong ?
		uint64_t now = Simulator::Now().GetMicroSeconds();
		if ( lasttime > now )
		{
			std::cout << "Now: " << now << std::endl;
			std::cout << "Diff: " << lasttime - now << std::endl;
			std::cout << "Link Check Error" << std::endl;
			exit (1);
		}

		// Get DATA from sen queue. 
		uint64_t totDataSize = 0;
		std::queue<DATA*> *dataContain = new std::queue<DATA*>;

		bool sinkFirst = false;

		if (sink->haveSendAction)
		{
			sink->haveSendAction = false;
			sinkFirst = true;
		}
		
		if (sinkFirst) // Sink Case (Priority)
		{
			std::queue<DATA*> *queue = &(sink->txQ);
			while (!(queue->empty ()) )
			{
				DATA *data = queue->front ();
				queue->pop ();
				totDataSize += (uint64_t)(data->dataSize);
				dataContain->push (data);
			}
		}
		else // Sensor Case
		{
			for (uint32_t i=0; i<ssN;i++)
			{
				std::queue<DATA*> *queue = &(sen[i].txQ);
				if ( !(queue->empty ()) )
				{
					DATA* data = queue->front ();
					queue->pop ();
					sen[i].FillTxQ (); // Beacause size of TxQ decrease 1, add one saved data in TxQ. 
					totDataSize += (uint64_t)(data->dataSize);
					dataContain->push (data);
				}
			}
		}

		// Link Delay Calculation
		totDataSize = Byte2Bit (totDataSize); // Byte to Bit
		double delay = (double) totDataSize / bw;
		uint64_t micro_delay = (uint64_t)(delay * 1000000);

		if (!netMod)
			micro_delay = 0;
		
		// "Link Check()" Scedule based on the link delay
		if (totDataSize != 0)
		{
			if (sinkFirst)
				Simulator::Schedule (MicroSeconds (micro_delay), &Link::SensorRecv, this, dataContain);
			else
				SinkRecvSche (dataContain, micro_delay);

			// Link Check Schedule
			Simulator::Schedule (MicroSeconds (micro_delay), &Link::Check, this);
		}
		else if (totDataSize == 0)
		{
			*isLinkScheWork = false;
			delete dataContain;
		}
		else
		{
			std::cout << "Link Cehck()::Error:totDataSize" << std::endl;
		}

		// Last Link Check() time record 
		lasttime = now + micro_delay;
	}
}

void Link::SensorRecv (std::queue<DATA*> *dataContain)
{
	while (!(dataContain->empty()))
	{
		uint64_t now = Simulator::Now().GetMilliSeconds ();
		DATA *data = dataContain->front ();
		dataContain->pop ();
		
		uint32_t serId = data->serId;
		uint32_t cellId = data->cellId;
		uint32_t senId = txTable[serId][cellId];

		Service *ser = &(sen[senId].ser[serId]);
		(ser->totActRecvN)++;
		(ser->totActDelay) += ( now - (data->genTime).GetMilliSeconds() );
		ser->sampleRate = data->action;
		ser->ReSchedule ();

		delete data;
	}
	delete dataContain;
}


// Callback
Callback<void> Link::CallbackCheck (void)
{
	return MakeCallback (&Link::Check, this);
}


}
