#include "sensor-header.h"

#define HEADER_SIZE 516

namespace ns3{

SensorHeader::SensorHeader ()
{
}

SensorHeader::~SensorHeader ()
{
}

TypeId
SensorHeader::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::SensorHeader")
    .SetParent<Header> ()
    .AddConstructor<SensorHeader> ()
  ;
  return tid;
}

TypeId
SensorHeader::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}

void
SensorHeader::Print (std::ostream &os) const
{
}

uint32_t
SensorHeader::GetSerializedSize (void) const
{
	// HEDAER SIZE = 4 + 4 + 8 + 1000
  return HEADER_SIZE;
}

void
SensorHeader::Serialize (Buffer::Iterator start) const
{
	start.WriteU32 (m_sensorId);
	start.WriteU32 (m_sensorVl);
	start.WriteU64 (m_fps);
	for (uint32_t i=0; i<500; ++i)
		start.WriteU8 (m_carInfo[i]);
}

uint32_t
SensorHeader::Deserialize (Buffer::Iterator start)
{
	m_sensorId = start.ReadU32 ();
	m_sensorVl = start.ReadU32 ();
	m_fps = start.ReadU64 ();
	
	for (uint32_t i=0; i<500; ++i)
		m_carInfo[i] = start.ReadU8 ();

	return GetSerializedSize ();
}

void 
SensorHeader::Set (uint32_t sensorId, uint32_t sensorVl, uint64_t fps, uint8_t* carInfo, uint32_t size)
{
	m_sensorId = sensorId;
	m_sensorVl = sensorVl;
	m_fps = fps;
	for (uint32_t i=0; i<size; i++)
		m_carInfo[i] = carInfo[i];
}

uint32_t
SensorHeader::GetSensorId (void)
{
	return m_sensorId;
}

uint32_t
SensorHeader::GetSensorVl (void)
{
	return m_sensorVl;
}

uint64_t
SensorHeader::GetFps (void)
{
	return m_fps;
}

uint8_t*
SensorHeader::GetCarInfo (void)
{
	return m_carInfo;
}

}
