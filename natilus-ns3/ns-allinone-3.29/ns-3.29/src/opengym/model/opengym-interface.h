#ifndef OPENGYM_INTERFACE_H
#define OPENGYM_INTERFACE_H

#include "ns3/object.h"
#include <zmq.hpp>

namespace ns3 {

class OpenGymInterface : public Object {
public: 
	static TypeId GetTypeId (void);

	OpenGymInterface ();
	virtual ~OpenGymInterface ();
	
	void SetUp (uint32_t);
	void SendJson (std::string, std::string);
	void SendObs(double**, uint32_t, uint32_t);
	double* RecvAction (uint32_t);
	void SendReward (double);
	void SendEnd (uint8_t);

private:
	uint32_t m_port;
};

}
#endif
