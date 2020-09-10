#ifndef SENSOR_H
#define SENSOR_H

#include "ns3/address.h"
#include "ns3/applications-module.h"
#include "ns3/event-id.h"
#include "ns3/ptr.h"
#include "ns3/traced-callback.h"
#include "t-object.h"

namespace ns3 {

class Address;
class Socket;

class SimpleSensor : public Application
{
public:
  static TypeId GetTypeId (void);

  SimpleSensor ();
  virtual ~SimpleSensor ();
	
	void Set (void);
	void SetRemote (Address ip, uint16_t port);
	void SetRemote (Address addr);

	void SetSocketAddress (Address);	
  Ptr<Socket> GetSocket (void) const;
	
	/* Log Setting */
	std::string log;
	std::string upMod;
	std::string obsMod;

	/* Object Setting */
	ObjectContain *oc;

	/* Sensor Setting */
	uint32_t senId;
	uint32_t sampleSize;
	uint32_t sampleValue;
	uint32_t sampleNum;
	uint32_t sampleRate;
	uint16_t *carInfo;
	Time		 eventTime;

protected:
  virtual void DoDispose (void);

private:
  virtual void StartApplication (void);    // Called at time specified by Start
  virtual void StopApplication (void);     // Called at time specified by Stop
	
	void SendFirst (void);
	void SendData (void);
	void SendN (Ptr<Packet>, uint32_t);
	void HandleRead (Ptr<Socket> socket);
	void ScheduleTransmit (double fps);

	void GetSample (void);
	void PrintInfo (void);

	Ptr<Socket>     m_socket;     //!< Associated socket
	Address m_peerAddress;
	uint16_t m_peerPort;
	EventId m_sendEvent;
	uint64_t m_seqNum;

	TracedCallback<Ptr<const Packet> > m_txTrace;

};

} // namespace ns3

#endif
