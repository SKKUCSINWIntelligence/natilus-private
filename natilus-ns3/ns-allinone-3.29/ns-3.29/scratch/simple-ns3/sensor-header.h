#ifndef SENSOR_HEADER_H
#define SENSOR_HEADER_H

#include "ns3/ptr.h"
#include "ns3/packet.h"
#include "ns3/header.h"
#include <iostream>

#define SH_SIZE 516

namespace ns3{

class SensorHeader : public Header                                                                             
{
	public:
		SensorHeader ();
		virtual ~SensorHeader();

		static TypeId GetTypeId (void);
		virtual TypeId GetInstanceTypeId (void) const;
		virtual void Print (std::ostream &os) const;
		virtual uint32_t GetSerializedSize (void) const; 
		virtual void Serialize (Buffer::Iterator start) const;
		virtual uint32_t Deserialize (Buffer::Iterator start);

		void Set (uint32_t, uint32_t, uint64_t, uint8_t*, uint32_t);
	
		uint32_t GetSensorId (void);
		uint32_t GetSensorVl (void);
		uint64_t GetFps (void);
		uint8_t* GetCarInfo (void);

	private:
		uint32_t			m_sensorId;
		uint32_t			m_sensorVl;
		uint64_t			m_fps;
		uint8_t				m_carInfo[500] = {0};

};    


}

#endif
