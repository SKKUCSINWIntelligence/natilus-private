#ifndef SINK_HEADER_H
#define SINK_HEADER_H

#include "ns3/ptr.h"
#include "ns3/packet.h"
#include "ns3/header.h"
#include <iostream>

#define SI_SIZE 8

namespace ns3{

class SinkHeader : public Header                                                                  
{
public:
	SinkHeader ();
	virtual ~SinkHeader();

	static TypeId GetTypeId (void);
	virtual TypeId GetInstanceTypeId (void) const;
	virtual void Print (std::ostream &os) const;
	virtual uint32_t GetSerializedSize (void) const; 
	virtual void Serialize (Buffer::Iterator start) const;
	virtual uint32_t Deserialize (Buffer::Iterator start);

	void Set (uint64_t);

	uint64_t GetFps (void);

private:
	uint64_t			m_fps;
};    
}
#endif
