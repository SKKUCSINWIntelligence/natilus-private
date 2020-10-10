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
	delete[] scoreMap;
	delete[] topLoc;
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
	/* malloc for DAFU */
	scoreMap = new double[ssN];
	topLoc = new int32_t[ssN];

	for (uint32_t i=0; i<ssN; i++)
	{
		recvBytes[i] = 0;
		seqNum[i] = 0;
		pktNum[i] = 0;
		scoreMap[i] = 0;
		topLoc[i] = -1;
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
	else if (upMod == "DAFU")
	{
		DAFU ();
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
		start = std::chrono::system_clock::now ();
    actionTmp = ZMQRecvAction (zmqsocket, actMod, ssN);
		end = std::chrono::system_clock::now ();
		msZMQ = std::chrono::duration_cast<std::chrono::milliseconds> (end - start);
		std::cout << "ACT Time: " << msZMQ.count() << std::endl;
	

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

void SimpleSink::DAFU(void)
{
	DAFUSetScore(state->sampleValue);
	DAFUTopK(scoreMap, topK);
	DAFUSetAction(topLoc, topK);
}

void SimpleSink::DAFUSetScore(double *map)
{
	if (dafuFtn == "map")
	{
		for(uint32_t i=0; i<ssN; i++)
		{
			scoreMap[i] = map[i];
		}
	}
	else if (dafuFtn == "around")
	{
		topK = 0;
		for (uint32_t i=0; i<ssN; i++)
		{
			scoreMap[i] = 0;

			int xId = i % ssN;
			int yId = i / ssN;
				
			int xxId = xId-1; 
			int yyId = yId;

			int sN = (int)ssN;

			if (xxId >= 0)
			{
				if (map[i] > map[yyId*ssN+xxId])
					scoreMap[i] += 1;
			}
			xxId = xId-1;
			yyId = yId+1;
			if (xxId >= 0 && yyId < sN)
			{
				if (map[i] > map[yyId*ssN+xxId])
					scoreMap[i] += 1; 
			}
			xxId = xId;
			yyId = yId+1;
			if (yyId < sN)
			{
				if (map[i] > map[yyId*ssN+xxId])
					scoreMap[i] += 1; 
			}
			xxId = xId+1;
			yyId = yId+1;
			if (xxId < sN && yyId < sN)
			{
				if (map[i] > map[yyId*ssN+xxId])
					scoreMap[i] += 1; 
			}
			xxId = xId+1;
			yyId = yId;
			if (xxId <sN)
			{
				if (map[i] > map[yyId*ssN+xxId])
					scoreMap[i] += 1; 
			}
			xxId = xId+1;
			yyId = yId-1;
			if (xxId < sN && yyId >= 0)
			{
				if (map[i] > map[yyId*ssN+xxId])
					scoreMap[i] += 1; 
			}
			xxId = xId;
			yyId = yId-1;
			if (yyId >= 0)
			{
				if (map[i] > map[yyId*ssN+xxId])
					scoreMap[i] += 1; 
			}
			xxId = xId-1;
			yyId = yId-1;
			if (xxId >= 0 && yyId >= 0)
			{
				if (map[i] > map[yyId*ssN+xxId])
					scoreMap[i] += 1; 
			}
				
			if (map[i] > 0)
			{
				if (xId == 0 && yId == 0)
					scoreMap[i] += 5;
				else if (xId == 0 && yId == (sN-1))
					scoreMap[i] += 5;
				else if (xId == (sN-1) && yId == 0)
					scoreMap[i] += 5;
				else if (xId == (sN-1) && yId == (sN-1))
					scoreMap[i] += 5;
				else if (xId == 0 || yId == 0 || xId == (sN-1) || yId == (sN-1))
					scoreMap[i] += 3;
			}

			if (scoreMap[i] >= 8)
				topK += 1;
		}
	}

	if (dafuInfo)
	{
		printf("------- DAFU Info-------- ");
		std::cout << "at " << Simulator::Now().GetSeconds() << std::endl;	
		printf("[[Truth Value]]\n");
		PrintState<double> (oc[0].trackMap, ssN);
		printf("[[Sample Value]]\n");
		PrintState<double> (state[0].sampleValue, ssN);
		printf("[[Score Value]]\n");
		PrintState<double> (scoreMap, ssN);
	}
}

void 
SimpleSink::DAFUTopK(double * map, uint32_t len)
{
	double min = 0;
	int32_t min_loc = 0;
	double temp = 0;  
	double temp2 = 0;
	double score_min = 0.001;
		
	for(uint32_t j = 0; j<ssN; j++)
	{
		temp = map[j];
		if(temp>min && temp!=0)
		{
			topLoc[min_loc] = j;
			min = temp;
			for(uint32_t k = 0; k<len; k++)
			{
				temp2 = score_min;
				if(topLoc[k] != -1)
					temp2 = map[topLoc[k]];
				if(min>temp2)
				{
					min = temp2;
					min_loc = k;
				}
			}
		}
	}	
	
	if (dafuInfo)
	{
		for(int32_t k =0 ; k<topK; k++)
		{
			std::cout<<topLoc[k]<<" ";
		}
		std::cout<<std::endl; 
	} 
}

void 
SimpleSink::DAFUSetAction(int32_t* scoreMap,int32_t len)	
{
	uint32_t nodeNum = ssN;
	uint32_t nodeLen = sqrt(nodeNum);
	int32_t* target = new int32_t[ssN];
	int32_t window = winSize;
	int32_t jump = 0;
	for(uint32_t i =0; i<nodeNum; i++)
	{
		target[i] = 1;
	}

	for(int32_t i = 0; i<len; i++)
	{
		if(scoreMap[i]>=0 && scoreMap[i]<(int32_t)nodeNum)
		{
			for(int32_t j = -window+1; j<window; j++)
			{
				for(int32_t k = -window+1; k<window; k++)
				{
					jump = (int32_t)nodeLen*k;
					int16_t x = (int16_t)scoreMap[i]+j+jump;
					if(x>-1 && x<(int16_t)nodeNum)
					{
						int16_t y = ((int16_t)scoreMap[i]+jump)%nodeLen;
						if(y+j>-1 && y+j<(int16_t)nodeLen)
						{
							target[scoreMap[i]+j+jump] = 2;
						}
					}
				}       
			}
		}
	}

	uint32_t targetNum = 0;
	double FPS_for_target = 0;
	double FPS_for_target_min = 10;

	for(uint32_t i = 0; i<nodeNum; i++)
	{
		if(target[i]==2)
			targetNum +=1;
	}

	if(targetNum >0)
		FPS_for_target = (avgRate*ssN-(nodeNum-targetNum)*FPS_for_target_min)/targetNum;
	else
	{
		FPS_for_target = avgRate*ssN/(double)nodeNum+1;
		FPS_for_target_min = FPS_for_target-1;
	}

	for(uint32_t i = 0; i<nodeNum; i++)
	{
		if(target[i]==2)
			state->action[i] = (uint32_t)FPS_for_target; //#### -> targetThr
		else
			state->action[i] = (uint32_t)FPS_for_target_min;
	}

	if (dafuInfo)
	{
		printf("\n[[Action Set]]\n");
		PrintState<uint32_t> (state->action, ssN);
	}
	for(uint32_t i = 0; i<ssN; i++)
		topLoc[i] = -1;

	delete[] target;
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

double* ZMQRecvAction (zmq::socket_t* zmqsocket, std::string actMod, uint32_t ssN)
{
  double *actions = new double[ssN];
	for (uint32_t i=0; i<ssN; i++)
		actions[i] = 0;

	/* For RED */
	if (actMod == "LA3")
	{
		zmq::message_t request (7);
		memcpy(request.data (), "Action", 7);
		zmqsocket->send (request);

		zmq::message_t reply;
		zmqsocket->recv (&reply);

		std::string s = std::string(static_cast<char*> (reply.data ()), reply.size());
	
		/* Set Point Num */
		uint32_t actionPoint = 0;
		if (ssN == 25 || ssN == 36) // Size 5, 6
			actionPoint = 9;
		else if (ssN == 49 || ssN == 64 || ssN == 100) // Size 7, 8, 10
			actionPoint = 16;
		else if (ssN == 144 || ssN == 256) // Size 12, 16
			actionPoint = 25;
		else
		{
			std::cout << "ZMQ LA3 Action Size Error !!!" << std::endl;
			exit (1);
		}
		double* actionLA = new double[actionPoint];
		
		s = s.substr(1, s.size()-2);
		s.erase(std::remove(s.begin(), s.end(), '\n'), s.end());
		s.erase(std::remove(s.begin(), s.end(), ' '), s.end());
	
		/* LA3 */
		double length = 1.0;
		for (uint32_t i=0; i<actionPoint; i++)
		{
			int find = s.find(",");
			std::string slice = s.substr(0, find);
			s = s.substr (find+1, s.size());		
			double retval = atof (slice.c_str ());
			actionLA[i] = retval;
		}
		
		uint32_t i = 0;
		for (uint32_t x=0; x<sqrt(ssN); x++)
		{
			for (uint32_t y=0; y<sqrt(ssN); y++)
			{
				double _x = y*length + 1;
				double _y = x*length + 1;
				
				for (uint32_t k=0; k<actionPoint; k++)
				{
					double _xp = (double)(k % (uint32_t)sqrt(actionPoint)) * (double)(sqrt(ssN)/sqrt(actionPoint)) + (double)(sqrt(ssN)/sqrt(actionPoint)/2.0);
					double _yp = (double)(k / (uint32_t)sqrt(actionPoint)) * (double)(sqrt(ssN)/sqrt(actionPoint)) + (double)(sqrt(ssN)/sqrt(actionPoint)/2.0);
					double dist = sqrt(((_x-_xp)*(_x-_xp)+(_y-_yp)*(_y-_yp)));
					if (dist < 1.0)
						dist = 1.0;
					actions[i] = actions[i] + (actionLA[k] / dist);
				}
				i++;
			}
		}	

		/* Clip Function */
		double thresh = 0.5;
		double eSum = 0;
		for (uint32_t i=0; i<ssN; i++)
		{
			if (actions[i] <= thresh)
			{
				if (ssN == 100 || ssN == 256)
					actions[i] = 0;
				else
					actions[i] = -2;
			}
			else
			{
				actions[i] *= 2;
			}
			
			eSum = eSum + pow(M_E, actions[i]);
		}

		/* Softmax Function */	
		for (uint32_t i=0; i<ssN; i++)
		{
			actions[i] = pow(M_E, actions[i]) / eSum;
		}
		
		delete[] actionLA;
	}
	else 
	{
		/* For Laive & SOTA */
		zmq::message_t request (7);
		memcpy(request.data (), "Action", 7);
		zmqsocket->send (request);

		zmq::message_t reply;
		zmqsocket->recv (&reply);

		std::string s = std::string(static_cast<char*> (reply.data ()), reply.size());

		s = s.substr(1, s.size()-2);
		s.erase(std::remove(s.begin(), s.end(), '\n'), s.end());
		s.erase(std::remove(s.begin(), s.end(), ' '), s.end());

		for (uint32_t i=0; i<ssN; i++)
		{
			int find = s.find(",");
			std::string slice = s.substr(0, find);
			s = s.substr (find+1, s.size());		
			double retval = atof (slice.c_str ());
			actions[i] = retval;
		}
	}
	
	return actions;
}

} // Namespace ns3
