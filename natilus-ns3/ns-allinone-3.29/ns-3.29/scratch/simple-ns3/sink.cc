#include "ns3/core-module.h"
#include "ns3/address.h"
#include "ns3/address-utils.h"
#include "ns3/log.h"
#include "ns3/inet-socket-address.h"
#include "ns3/inet6-socket-address.h"
#include "ns3/node.h"
#include "ns3/socket.h"
#include "ns3/udp-socket.h"
#include "ns3/simulator.h"
#include "ns3/socket-factory.h"
#include "ns3/packet.h"
#include "ns3/trace-source-accessor.h"
#include "ns3/tcp-socket-factory.h"
#include "ns3/tcp-socket.h"
#include "ns3/tcp-socket-base.h"
#include "ns3/tcp-tx-buffer.h"
#include "sink.h"
#include "sensor-header.h"
#include "sink-header.h"
#include "struct.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("SimpleSink");

NS_OBJECT_ENSURE_REGISTERED (SimpleSink);

TypeId 
SimpleSink::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::SimpleSink")
    .SetParent<Application> ()
    .SetGroupName("Applications")
    .AddConstructor<SimpleSink> ()
    .AddAttribute ("Port",
                   "Port on which we listen for incoming packets.",
                   UintegerValue (9),
									 MakeUintegerAccessor (&SimpleSink::m_port),
									 MakeUintegerChecker<uint16_t> ())
  ;
  return tid;
}

SimpleSink::SimpleSink ()
{
  NS_LOG_FUNCTION (this);
  m_socket = 0;
  m_totalRx = 0;

	state = new STATE;
}

SimpleSink::~SimpleSink()
{
  NS_LOG_FUNCTION (this);
	
	m_socket = 0;
	delete state;
	delete[] recvBytes;
	delete[] seqNum;
	delete[] pktNum;
}

void 
SimpleSink::Set (void)
{
	/* malloc addr list */
	addressList = new Address[ssN];
	addressP2P = new Address[ssN];
	for (uint32_t i=0; i<1000; i++)
		state->sampleCar[i] = -1;

	/* malloc recv list */
	recvBytes = new uint64_t[ssN];
	/* malloc sensor seq num */
	seqNum = new uint64_t[ssN];
	pktNum = new uint32_t[ssN];
	for (uint32_t i=0; i<ssN; i++)
	{
		recvBytes[i] = 0;
		seqNum[i] = 0;
		pktNum[i] = 0;
		/* set uniform action */
		state->action[i] = avgRate;
	}
}

uint64_t 
SimpleSink::GetTotalRx () const
{
  NS_LOG_FUNCTION (this);
  return m_totalRx;
}

void 
SimpleSink::DoDispose (void)
{
  NS_LOG_FUNCTION (this);  
	Application::DoDispose ();
}

// Application Methods
void 
SimpleSink::StartApplication ()    // Called at time specified by Start
{
  NS_LOG_FUNCTION (this);	

  if (m_socket == 0)
  {
		TypeId tid = TypeId::LookupByName ("ns3::UdpSocketFactory");
    m_socket = Socket::CreateSocket (GetNode (), tid);
		InetSocketAddress local = InetSocketAddress (Ipv4Address::GetAny (), m_port);

		if (m_socket->Bind (local) == -1)
		{
			NS_FATAL_ERROR ("Failed to bind socket");
    }
		if (addressUtils::IsMulticast (m_local))
		{
			Ptr<UdpSocket> udpSocket = DynamicCast<UdpSocket> (m_socket);
			if (udpSocket)
			{
				udpSocket->MulticastJoinGroup (0, m_local);
			}
			else
			{
				NS_FATAL_ERROR ("Error: Failed to join multicast group");
			}
		} 
  }
	
	for (uint32_t i=0; i<ssN; i++)
	{
		TypeId tid = TypeId::LookupByName ("ns3::UdpSocketFactory");
		c_socket.push_back(Socket::CreateSocket (GetNode (), tid));
		
		if (Ipv4Address::IsMatchingType (addressP2P[i]) == true)
		{
			if (c_socket[i]->Bind () == -1)
			{
				NS_FATAL_ERROR ("Failed to bind socket");
			}
			c_socket[i]->Connect (InetSocketAddress (Ipv4Address::ConvertFrom(addressP2P[i]), i+1));
		}
		else if (InetSocketAddress::IsMatchingType (addressP2P[i]) == true)
		{
			if (c_socket[i]->Bind () == -1)
			{
				NS_FATAL_ERROR ("Failed to bind socket");
			}
			c_socket[i]->Connect (addressP2P[i]);
		}
		else
		{
			NS_ASSERT_MSG (false, "Incompatible address type: " << addressP2P[i]);
		}
	}

	m_socket->SetRecvCallback (MakeCallback (&SimpleSink::HandleRead, this)); 
}

void 
SimpleSink::StopApplication ()     // Called at time specified by Stop
{
  NS_LOG_FUNCTION (this);

	if (m_socket != 0)
	{
		m_socket->Close ();
		m_socket->SetRecvCallback (MakeNullCallback<void, Ptr<Socket>> ());
	} 
}


void 
SimpleSink::HandleRead (Ptr<Socket> socket)
{
	NS_LOG_FUNCTION (this << socket);
	Ptr<Packet> pkt;
	Address from;

	while ((pkt = socket->RecvFrom (from)))
	{

		m_totalRx += pkt->GetSize ();
		
		/* Retrieve Sensor Header */
		SensorHeader spHeader; 
		pkt->RemoveHeader (spHeader);
		
		uint64_t seq = spHeader.GetSeqNum ();
		uint32_t sensorId = spHeader.GetSensorId ();
		uint32_t sensorVl = spHeader.GetSensorVl ();
		uint64_t fps = spHeader.GetFps ();
		uint8_t *carCell = spHeader.GetCarInfo ();
	
		//std::cout << from << " " << addressList[sensorId] <<  std::endl;
		//std::cout <<	InetSocketAddress::ConvertFrom (from).GetIpv4 () << " " << InetSocketAddress::ConvertFrom (addressList[sensorId]).GetIpv4 () << std::endl; 
		//std::cout << "[[Sink Info]] at " << Simulator::Now().GetSeconds() <<  std::endl;
		//std::cout << "Sequence Number: " << seq << std::endl;
		//std::cout << "SensorId: " << sensorId << std::endl;
		/*std::cout << "SensorVl: " << sensorVl << std::endl;
		std::cout << "Fps     : " << fps << std::endl;*/
		/*for (uint32_t i=0; i<oc->objectMax; i ++)
		{
			if (carCell[i]== 0)
				std::cout << "0";
			else
				std::cout << "1";
		}*/
		
		if (seq == 0)
		{		
			/* Save Address */
			addressList[sensorId] = from;
			//std::cout << from << std::endl;
		}
		else
		{
			// Save Recv Bytes
			recvBytes[sensorId] += pkt->GetSize ();
		
			// Check Sequence Number
			if (seq != seqNum[sensorId])
			{
				if (seq < seqNum[sensorId])
				{
					printf("Sequence Number Error!!!");
				}
				if (pktNum[sensorId] < sampleNum)
				{
					lossFrm += 1;
					lossPkt = lossPkt + (sampleNum - pktNum[sensorId]);
				}
				seqNum[sensorId] = seq;
				pktNum[sensorId] = 1;
			}
			else
			{
				pktNum[sensorId] += 1;
			}
		
			// If Enough Packet Recv than Update the map
			if (pktNum[sensorId] == sampleNum)
			{
				//std::cout << "Sensor " << sensorId << " Seq Num is " << seqNum[sensorId] << std::endl;
				// State Fps
				state->sampleRate[sensorId] = fps;
		
				// State Car Number
				double pastValue = state->sampleValue[sensorId];
				if (obsMod == "multi")
				{
					// Save Car's cell
					for (uint32_t i=0; i<oc->objectMax; i++)
					{
						if (state->sampleCar[i] == (int) sensorId)
							state->sampleCar[i] = -1;
						if (carCell[i] == 1)
							state->sampleCar[i] = (int)sensorId;
					}

					// Clear the map
					for (uint32_t i=0; i<ssN; i++)
					{
						state->sampleValue[i] = 0;
					}
			
					// Fill the map
					for (uint32_t i=0; i<oc->objectMax; i++)
					{
						int cell = state->sampleCar[i];
						if (cell >= 0)
							state->sampleValue[cell] += 1;
					}
				}
				else if (obsMod == "sumo")
				{
					state->sampleValue[sensorId] = (double) sensorVl;
				}
		
				// State Change Time
				double curValue = state->sampleValue[sensorId];
				if (pastValue != curValue)
					state->stateChangeTime[sensorId] = Simulator::Now ();
				// State Last Update Time
				state->lastUpdateTime[sensorId] = Simulator::Now ();
			}
		
			if (firstEval)
			{
				firstEval = false;
				double commTime = 1.0/30.0;
				Simulator::Schedule (Seconds (commTime), &SimpleSink::Comm, this);
				double evalTime = 1.0/(double)(rand()%30+1+120);
				Simulator::Schedule (Seconds (evalTime), &SimpleSink::Eval, this);
			}
		}	
	}
}

void 
SimpleSink::Eval (void)
{
	// Update Truth
	oc->Moving ();
	
	//PrintInfo ();
	
	// Reward Count
	cnt += 1;
	cntAvg += 1;

	// Calculate Reward //
	double Mx=0; //truth average
  double My=0; //observed average
	double Vx=0; //truth variance
  double Vy=0; //observed variance
  double Cxy=0; //correlation of truth, observed
  double C =  1e-50;

  double l = 0;
  double c = 0;
  double s = 0;

	double tmpMultiCnt = 0;

  for(uint32_t j = 0; j<ssN; j++)
  {
    Mx += oc->trackMap[j];
    My += state->sampleValue[j];

		// Count Multi Objects
		if (oc->trackMap[j] != 0)
		{
			tmpMultiCnt += 1;
		}
  }
	
	tmpMultiCnt /= ssN;
	multiCnt += tmpMultiCnt;
	if (tmpMultiCnt > multiMax)
	{
		multiMax = tmpMultiCnt;
	}
      
	Mx /= ssN;
  My /= ssN;

  for(uint32_t j = 0; j<ssN; j++)
  {
    Vx += (oc->trackMap[j]-Mx)*(oc->trackMap[j]-Mx);
    Vy += (state->sampleValue[j]-My)*(state->sampleValue[j]-My);
    Cxy += (oc->trackMap[j]-Mx)*(state->sampleValue[j]-My);
  }

  Vx /= ssN;
	Vy /= ssN;
	Cxy /= (ssN-1);

  l = (2*Mx*My+C) / (Mx*Mx+My*My+C);
  c = (2*sqrt(Vx)*sqrt(Vy)+C) / (Vx+Vy+C);
  s = (Cxy+C) / (sqrt(Vx*Vy)+C);

  reward += l*c*s;
  rewardAvg += l*c*s;	
	//std::cout << "Reward: " << l*c*s << std::endl;
	// Calculate Done //
	
	double evalTime = 1.0/(double)(rand()%30+1+120);
	Simulator::Schedule (Seconds (evalTime), &SimpleSink::Eval, this);	
}

void 
SimpleSink::Comm (void)
{
	if (stateInfo)
		PrintInfo ();
	//std::cout << "Reward: " << reward /cnt << std::endl << std::endl;
	evalCnt ++;

	if (cnt == 0)
		reward = 0;
	else
		reward = reward / cnt;
	
	if (upMod == "rlidagan")
	{
		ZMQComm ();
		SendData ();
	}
	else if (upMod == "xuniform")
	{
		SendData ();
	}
	else
	{
		SendData ();
	}
	
	reward = 0;
	cnt = 0; 
	
	if (upMod != "rlidagan")
	{
		if (evalCnt > maxStep)
		{
			// Stop the Simlator
			endTime = Simulator::Now ().GetSeconds ();
			std::cout << "##################" << std::endl;
			std::cout << "Episod Stop\n" << std::endl;
			std::cout << "##################" << std::endl;
			Simulator::Stop ();
		}
	}

	double commTime = 1.0/30.0;
  Simulator::Schedule (Seconds (commTime), &SimpleSink::Comm, this);
}

void
SimpleSink::SendData (void)
{
	uint32_t sendSize = 10 - SI_SIZE;
	
	for (uint32_t i=0; i<ssN; i++) 
	{
		Ptr<Packet> pkt = Create<Packet> (sendSize);

		SinkHeader spHeader;
		spHeader.Set ((uint64_t)state->action[i]);
		pkt->AddHeader (spHeader);
		
		c_socket[i]->Send (pkt);
		//m_socket->SendTo (pkt, 0, addressList[i]);
		//std::cout << addressP2P[i] << std::endl;	
		//std::cout << "At time " << Simulator::Now ().GetSeconds () << 
		//	"s server sent " << pkt->GetSize () << 
		//	" bytes to " << InetSocketAddress::ConvertFrom (addressP2P[i]).GetIpv4 () << 
		//	" port " <<  InetSocketAddress::ConvertFrom (addressP2P[i]).GetPort () << std::endl;
  }
}

void
SimpleSink::ZMQComm (void)
{
  if (evalCnt <= maxStep &&  episodeEnd == false)
  {
    if (episodeStart)
      episodeStart = false;

    else 
    {
      // Send Reward
			std::string message = "{\"reward\":" + std::to_string (reward) + "}";
      ZMQSendJson (zmqsocket, message);
    }

    uint32_t sN = sqrt(ssN);

    // Send State 
    ZMQSendObs (zmqsocket, stateMod, state, oc, sN, obsMod);

    // Get Action
    double *actionTmp;
    actionTmp = ZMQRecvAction (zmqsocket, ssN);
		
    for (uint32_t i=0; i<ssN; i++)
    { 
			state->action[i] = actionTmp[i] * (avgRate*ssN);	
			if(state->action[i]==0)
				state->action[i] = 1;        
    }
    delete[] actionTmp;
 
   // printf("[sampleAction]::Observed\n");
   // PrintState<uint32_t> (state->action,ssN);
    

    // Send End
    if (evalCnt < maxStep && episodeEnd == false)
    {
      uint8_t end = 0;
      ZMQSendEnd (zmqsocket, end);
    }
		else
    {
      uint8_t end = 1;
      ZMQSendEnd (zmqsocket, end);
    }
  }
  else
  {
    // Send Reward 
    std::string message = "{\"reward\":" + std::to_string (reward) + "}";
    ZMQSendJson (zmqsocket, message);
    
		uint32_t sN = sqrt(ssN);

		// Send State
    ZMQSendObs (zmqsocket, stateMod, state, oc, sN, obsMod);
		
		// Set End Time
		endTime = Simulator::Now().GetSeconds();

		// Stop the Simlator
		std::cout << "##################" << std::endl;
		std::cout << "Episod Stop\n" << std::endl;
		std::cout << "##################" << std::endl;
		Simulator::Stop ();
	}
}

void
SimpleSink::PrintInfo (void)
{
  std::cout << "At " << Simulator::Now().GetSeconds() << std::endl;
  printf("[sampleRate]::Observed\n");
  PrintState<uint32_t> (state->sampleRate, ssN);
  printf("[sampleValue]::Observed\n");
  PrintState<double> (state->sampleValue, ssN);
  printf("[sampleValue]::Truth\n");
  PrintState<double> (oc->trackMap, ssN);
  printf("--------------------------\n");
}

void ZMQSendJson (zmq::socket_t* zmqsocket, std::string message)
{
  zmq::message_t request (message.size());
  memcpy (request.data (), message.c_str (), message.size ());
  zmqsocket->send(request);

  zmq::message_t reply;
  zmqsocket->recv (&reply);
}

void ZMQSendObs (zmq::socket_t* zmqsocket, std::string stateMod, STATE* state, ObjectContain* oc, uint32_t ssN, std::string obsMod)
{ 
  double **obs = new double*[ssN];
  for (uint32_t i=0; i<ssN; i++)
  {
    obs[i] = new double[ssN];
    for (uint32_t j=0; j<ssN; j++)
      obs[i][j] = 0;
  }

  double **last = new double*[ssN];
  for (uint32_t i=0; i<ssN; i++)
	{
    last[i] = new double[ssN];
    for (uint32_t j=0; j<ssN; j++)
			last[i][j] = 0;
  }

  double **change = new double*[ssN];
  for (uint32_t i=0; i<ssN; i++)
  {
    change[i] = new double[ssN];
    for (uint32_t j=0; j<ssN; j++)
      change[i][j] = 0;
  }

  uint32_t **rate = new uint32_t*[ssN];
  for (uint32_t i=0; i<ssN; i++)
  {
    rate[i] = new uint32_t[ssN];
    for (uint32_t j=0; j<ssN; j++)
      rate[i][j] = 0;
  }

  double **ground = new double*[ssN];
  for (uint32_t i=0; i<ssN; i++)
  {
    ground[i] = new double[ssN];
    for (uint32_t j=0; j<ssN; j++)
      ground[i][j] = 0;
  }

  for (uint32_t i=0; i<ssN * ssN; ++i)
  {
    uint32_t x = i % ssN;
    uint32_t y = i / ssN;

    obs[ssN - y - 1][x] = state->sampleValue[i];
    last[ssN - y - 1][x] = Simulator::Now().GetMilliSeconds() - state->lastUpdateTime[i].GetMilliSeconds ();
    change[ssN-y-1][x] = Simulator::Now().GetMilliSeconds() - state->stateChangeTime[i].GetMilliSeconds();
    rate[ssN - y - 1][x] = state->sampleRate[i];
    ground[ssN - y - 1][x] = oc->trackMap[i];
  }

  std::string message = "";

  message = "{\"obs\":[[";
  for (uint32_t i=0; i<ssN; ++i)
  {
    message += "[";
    for (uint32_t j=0; j<ssN; ++j)
    {
      message += std::to_string(obs[i][j]);

      if(j != ssN - 1) 
        message += ",";
    }
    message += "]";

    if (i != ssN - 1) 
      message += ",";
  }
  if (stateMod == "last" || stateMod == "change")
  {
    message += "],[";
    for (uint32_t i=0; i<ssN; ++i)
    {
      message += "[";
      for (uint32_t j=0; j<ssN; ++j)
      {
        message += std::to_string(last[i][j]);

        if (j != ssN - 1)
          message += ",";
      }
      message += "]";

      if (i != ssN -1)
        message += ",";
    }
  }
  if (stateMod == "change")
  {
    message += "],[";
    for (uint32_t i=0; i<ssN; ++i)
    {
      message += "[";
      for (uint32_t j=0; j<ssN; ++j)
      {
        message += std::to_string(change[i][j]);

        if (j != ssN - 1)
          message += ",";
      }
      message += "]";

      if (i != ssN -1)
        message += ",";
    }
  }
    if (stateMod == "action")
    {
      message += "],[";
      for (uint32_t i=0; i<ssN; ++i)
      {
        message += "[";
        for (uint32_t j=0; j<ssN; ++j)
        {
          message += std::to_string(ground[i][j]);

          if (j != ssN - 1)
            message += ",";
        }
        message += "]";

        if (i != ssN -1)
          message += ",";
      }
    }

    message += "]]}";

    zmq::message_t request (message.size ());
    memcpy (request.data (), message.c_str (), message.size ());
    zmqsocket->send (request);

    zmq::message_t reply;
    zmqsocket->recv (&reply);

    for (uint32_t i=0; i<ssN; i++)
    {
      delete[] obs[i];
      delete[] last[i];
      delete[] change[i];
      delete[] rate[i];
      delete[] ground[i];
    }
    delete[] obs;
    delete[] last;
    delete[] change;
    delete[] rate;
    delete[] ground;
  }

void ZMQSendEnd (zmq::socket_t* zmqsocket, uint8_t end)
{
  zmq::message_t request(sizeof (end));
  memcpy (request.data (), &end, sizeof (end));
  zmqsocket->send (request);

  zmq::message_t reply;
  zmqsocket->recv (&reply);
}

double* ZMQRecvAction (zmq::socket_t* zmqsocket, uint32_t ssN)
{
  double *actions = new double[ssN];

  for (uint32_t i=0; i<ssN; i++)
  {
    zmq::message_t request (7);
    memcpy(request.data (), "Action", 7);
    zmqsocket->send (request);

    zmq::message_t reply;
    zmqsocket->recv (&reply);

    std::string action = std::string(static_cast<char*> (reply.data ()), reply.size());
    double retval = atof (action.c_str ());

    actions[i] = retval;
  }
  return actions;
}

} // Namespace ns3
