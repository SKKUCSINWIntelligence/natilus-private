#include "sensor.h"

namespace ns3{

Sensor::Sensor ()
{
}

Sensor::~Sensor ()
{
	delete[] ser;
}

void Start (void)
{
}

void Sensor::ServiceListGen ()
{
	ser = new Service[serviceN];
}

bool Sensor::InsertTxQ (DATA *m_data)
{
	bool insert;
	// zeroQ Modi 2020.07.21
	uint32_t qSize = txQ.size ();
	// Data insert into TxQ, when TxQ size free.
	if (qSize == 0)
	{
		txQ.push (m_data);
		insert = true;
	}
	else
	{
		while (!txQ.empty ())
		{
			DATA *old_data = txQ.front ();
			txQ.pop ();
			delete old_data;
		}
		txQ.push (m_data);
		insert = true;
	}

	/* No zeroQ Mode
	if (txQ.size () > qMaxSize)
	{
		// LOG::Queue
		//std::cout << "Sensor: " << sensorId << "::TxQueueFull" << std::endl;
		insert = false;
	}
	else
	{
		txQ.push (m_data);
		insert = true;
	}
	*/

	if (!(*isLinkScheWork))
	{
		*isLinkScheWork = true;
		LinkCheck ();
	}
	
	return insert;
}

// One data of a service insert into txQ.
// Round-robin access
bool Sensor::FillTxQ ()
{
	for (uint32_t i=0; i<serviceN; i++)
	{
		Service *t_ser = &ser[rr_idx];
		rr_idx = (rr_idx+1) % serviceN;
		if ( (t_ser->enable ) && (t_ser->saveSample) )
		{
			InsertTxQ (t_ser->saveData);
			t_ser->saveSample = false;
			t_ser->saveData = NULL;
			return true;
		}
	}
	return false;
}

// Sensor Recv Imple. See link.cc

// Callback
Callback<bool, DATA*> Sensor::CallbackInsertTxQ (void)
{
	return MakeCallback (&Sensor::InsertTxQ, this);
}


}
