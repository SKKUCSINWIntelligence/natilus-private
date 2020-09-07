#include "sink-header.h"

#define HEADER_SIZE 8

namespace ns3{

SinkHeader::SinkHeader ()
{
}

SinkHeader::~SinkHeader ()
{
}

TypeId
SinkHeader::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::SinkHeader")
    .SetParent<Header> ()
    .AddConstructor<SinkHeader> ()
  ;
  return tid;
}

TypeId
SinkHeader::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}

void
SinkHeader::Print (std::ostream &os) const
{
}

uint32_t
SinkHeader::GetSerializedSize (void) const
{
	// HEDAER SIZE = 8
  return HEADER_SIZE;
}

void
SinkHeader::Serialize (Buffer::Iterator start) const
{
	start.WriteU64 (m_fps);
}

uint32_t
SinkHeader::Deserialize (Buffer::Iterator start)
{
	m_fps = start.ReadU64 ();	
	return GetSerializedSize ();
}

void 
SinkHeader::Set (uint64_t fps)
{
	m_fps = fps;
}

uint64_t
SinkHeader::GetFps (void)
{
	return m_fps;
}
}
