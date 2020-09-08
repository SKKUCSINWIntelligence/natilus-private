#include "ns3/log.h"
#include "ns3/address.h"
#include "ns3/node.h"
#include "ns3/nstime.h"
#include "ns3/socket.h"
#include "ns3/tcp-socket.h"
#include "ns3/simulator.h"
#include "ns3/socket-factory.h"
#include "ns3/packet.h"
#include "ns3/uinteger.h"
#include "ns3/trace-source-accessor.h"
#include "ns3/internet-module.h"
#include "sensor.h"
#include "sensor-header.h"
#include "sink-header.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("SimpleSensor");

NS_OBJECT_ENSURE_REGISTERED (SimpleSensor);

TypeId
SimpleSensor::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::SimpleSensor")
    .SetParent<Application> ()
    .SetGroupName("Applications") 
    .AddConstructor<SimpleSensor> ()
    .AddAttribute ("RemoteAddress", "The address of the destination",
                   AddressValue (),
                   MakeAddressAccessor (&SimpleSensor::m_peerAddress),
                   MakeAddressChecker ())
    .AddAttribute ("RemotePort", "The port of the destination",
                   UintegerValue (0),
									 MakeUintegerAccessor (&SimpleSensor::m_peerPort),
                   MakeUintegerChecker<uint16_t> ())
  ;
  return tid;
}

SimpleSensor::SimpleSensor ()
{
	NS_LOG_FUNCTION (this);
	m_socket = 0;
	m_sendEvent = EventId ();
}

SimpleSensor::~SimpleSensor()
{
  NS_LOG_FUNCTION (this);
	m_socket = 0;
	delete[] carInfo;
}

void
SimpleSensor::Set ()
{
	carInfo = new uint16_t[oc->objectMax];
	for (uint32_t i=0; i<oc->objectMax; i++)
		carInfo[i] = (uint16_t) oc->senN + 1;	
}
void 
SimpleSensor::SetRemote (Address ip, uint16_t port)
{
	NS_LOG_FUNCTION (this << ip << port);
	m_peerAddress = ip;
	m_peerPort = port;
}

void
SimpleSensor::SetRemote (Address ip)
{
	NS_LOG_FUNCTION (this << ip);
	m_peerAddress = ip;
}

void
SimpleSensor::DoDispose (void)
{
  NS_LOG_FUNCTION (this);

  m_socket = 0;
  // chain up
  Application::DoDispose ();
}

void 
SimpleSensor::StartApplication (void)
{
	NS_LOG_FUNCTION (this);
	
	if (m_socket == 0)
	{
		TypeId tid = TypeId::LookupByName ("ns3::UdpSocketFactory");
		m_socket = Socket::CreateSocket (GetNode (), tid);
		
		if (Ipv4Address::IsMatchingType (m_peerAddress) == true)
		{
			if (m_socket->Bind () == -1)
			{
				NS_FATAL_ERROR ("Failed to bind socket");
			}
			m_socket->Connect (InetSocketAddress (Ipv4Address::ConvertFrom(m_peerAddress), m_peerPort));
		}
		else if (InetSocketAddress::IsMatchingType (m_peerAddress) == true)
		{
			if (m_socket->Bind () == -1)
			{
				NS_FATAL_ERROR ("Failed to bind socket");
			}
			m_socket->Connect (m_peerAddress);
		}
		else
		{
			NS_ASSERT_MSG (false, "Incompatible address type: " << m_peerAddress);
		}
	}
	
	m_socket->SetRecvCallback (MakeCallback (&SimpleSensor::HandleRead, this));
	m_socket->SetAllowBroadcast (true);
	ScheduleTransmit (0.);
}

void 
SimpleSensor::StopApplication ()
{
	NS_LOG_FUNCTION (this);

	if (m_socket != 0)
	{
		m_socket->Close ();
		m_socket->SetRecvCallback (MakeNullCallback<void, Ptr<Socket>> ());
		m_socket = 0;
	}

	Simulator::Cancel (m_sendEvent);
}

void
SimpleSensor::ScheduleTransmit (double time)
{
	NS_LOG_FUNCTION (this << time);
	m_sendEvent = Simulator::Schedule (Seconds (time), &SimpleSensor::SendData, this);
}

void
SimpleSensor::SendData (void)
{
	GetSample ();
	NS_LOG_FUNCTION (this);

	NS_ASSERT (m_sendEvent.IsExpired ()); 
	
	// (0908) Send Multiple n Pacets
	for (uint32_t i=0; i<sampleNum; i++)
	{

		uint32_t sendSize = sampleSize - SH_SIZE; 
		Ptr<Packet> pkt = Create<Packet> (sendSize);
		uint8_t *carCell = new uint8_t[oc->objectMax];
		for (uint32_t i=0; i<oc->objectMax; i++)
		{
			if (carInfo[i] == (int)senId)
				carCell[i] = 1;
			else
				carCell[i] = 0;
			//if (carCell[i] == 0)
			//	std::cout << "0";
		}

		SensorHeader spHeader;
		spHeader.Set (0, senId, sampleValue, sampleRate, carCell, oc->objectMax);
		pkt->AddHeader (spHeader);
		
		m_socket->Send (pkt);
	}	
	eventTime = Simulator::Now ();

	if (log == "woo")
		//std::cout << "Sensor " << senId << " send data at " << Simulator::Now().GetSeconds()<< std::endl;	
		PrintInfo ();

	ScheduleTransmit ((double)(1.0/sampleRate));
}

void 
SimpleSensor::HandleRead (Ptr<Socket> socket)
{
	NS_LOG_FUNCTION (this << socket);

	Ptr<Packet> pkt;
	Address from; 

	while ((pkt = socket->RecvFrom (from)))
	{
		SinkHeader spHeader;
		pkt->RemoveHeader (spHeader);

		uint64_t fps = spHeader.GetFps ();
		sampleRate = fps;
		
		Simulator::Cancel (m_sendEvent);
		uint64_t elapseTime = Simulator::Now().GetMicroSeconds () - eventTime.GetMicroSeconds ();
		uint64_t period = (double)1/sampleRate * 1000000;
		
		if (elapseTime >= period)
		{
			SendData ();
		}
		else
		{
			period = period - elapseTime;
			m_sendEvent = Simulator::Schedule (MicroSeconds (period), &SimpleSensor::SendData, this);	
		}
		//std::cout << "Sensor " << senId << " recv action " << fps  << std::endl;	
	}

}

void
SimpleSensor::GetSample (void)
{
	oc->Moving ();
	
	sampleValue = oc->trackMap[senId];

	for (uint32_t i=0; i<oc->objectMax; i++)
	{
		if ((oc->object[i]).occupy)
		{
			uint32_t xId = (uint32_t)(oc->object[i].x)/(oc->cellUnit);
			uint32_t yId = (uint32_t)(oc->object[i].y)/(oc->cellUnit);
			if (xId == (oc->unitN))
				xId -= 1;
			if (yId == (oc->unitN))
				yId -= 1;
			int cellid = (int)(yId * oc->unitN + xId);
			carInfo[i] = cellid;
		}
		else
		{
			carInfo[i] = (uint16_t) oc->senN + 1;
		}
	}
}

void
SimpleSensor::PrintInfo (void)
{
	std::cout << "At " << Simulator::Now().GetSeconds() << std::endl;
	std::cout << "Sensor " << senId << " sends " << sampleValue << std::endl;
	printf("[sampleValue]::Truth\n");
  PrintState<double> (oc->trackMap, oc->senN);
  printf("--------------------------\n");
}

} // Namespace ns3
