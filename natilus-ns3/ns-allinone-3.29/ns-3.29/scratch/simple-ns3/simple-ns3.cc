#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/wifi-module.h"
#include "ns3/internet-module.h"
#include "ns3/mobility-module.h"

#include "t-object.h"
#include "sink-helper.h"
#include "sink.h"
#include "sensor-helper.h"
#include "sensor.h"

using namespace ns3;
NS_LOG_COMPONENT_DEFINE ("simple-ns3");
using std::cout;
using std::endl;

int
main (int argc, char *argv[])
{
	std::string log = "woo";
	std::string port = "5050";
	
	srand (time(NULL));
	uint64_t maxStep = 100;

	/* Mode Setting */
	bool rlMod = false;
	std::string env = "wireless";
	std::string obsMod = "multi";
	std::string upMod = "rlidagan"; // uniform/DAFU/rlidagan
	std::string simMod = "x";
	std::string stateMod = "change";
	std::string testMod = "false";

	/* Log Setting */
	bool trace = false;
	bool stateInfo = false;
	
	/* Variable Setting */
	uint32_t sensorAvgRate = 60;	// unit: #/s
	uint32_t sampleSize = 1500;		// Bytes per sample
	uint32_t bwLimit = 50;				// unit: %

	/* Object Setting */
	uint32_t ssN = 6;
	uint32_t objectN = 0;
	uint32_t objectMax = 80;
	double cellUnit = 2; // unit: m
	
	double maxSpeed = cellUnit*sensorAvgRate; // m/s
	double speedRate = 40; //unit: %
	double objectSpeed = maxSpeed*speedRate / 100;

	/* Wifi Setting */
	uint32_t dataSpeed = 9;
	std::string dataMode = "VhtMcs"+std::to_string(dataSpeed);
	
	/* Command Setting */
	CommandLine cmd;
	cmd.AddValue ("port", "Socket Port", port);
	cmd.AddValue ("obsMod", "Obs Mode", obsMod);
	cmd.AddValue ("upMod", "Algorithm: uniform/DAFU/rlidagan", upMod);
	cmd.AddValue ("ssN", "Sesnsor #", ssN);
	cmd.AddValue ("bwLimit", "BW Limit %", bwLimit);
	cmd.AddValue ("objMax", "Object Max", objectMax);
	cmd.AddValue ("sInfo", "State Info", stateInfo);
	cmd.Parse (argc, argv);
	
	if (ssN==6)
	{
		if(objLimit == 25)
			objectMax = 32;
		else if (objLimit == 40)
			objectMax = 32;
	}
	else if (ssN==8)
	{
		if (objLimit == 15)
			objectMax = 36;
		else if (objLimit == 20)
			objectMax = 48;
		else if (objLimit == 25)
			objectMax = 60;
		else if (objLimit == 30)
			objectMax = 72;
		else if (objLimit == 35)
			objectMax = 100;
		else if (objLimit == 40)
			objectMax = 48;
	}
	else if (ssN==10)
	{
		if (objLimit == 25)
			objectMax = 108;
		else if (objLimit == 40)
			objectMax = 80;
	}
	else if (ssN==12)
	{
		if (objLimit == 15)
			objectMax = 80;
		else if (objLimit == 20)
			objectMax = 120;
		else if (objLimit == 25)
			objectMax = 180;
		else if (objLimit == 30)
			objectMax = 280;
		else if (objLimit == 35)
			objectMax = 400;
		else if (objLimit == 40)
			objectMax = 128;
	}
	else if (ssN==16)
	{
		if (objLimit == 25)
			objectMax = 540;
		else if (objLimit == 40)
			objectMax = 260;
	}

	ssN = ssN*ssN;
	sensorAvgRate = sensorAvgRate * bwLimit / 100;
	uint64_t bw = Byte2Bit(sampleSize) * ssN *sensorAvgRate;
	
	/* ZMQ Setting */
	zmq::context_t zmqcontext {1};
	zmq::socket_t zmqsocket {zmqcontext, ZMQ_REQ};
	if (upMod == "rlidagan")
	{
		std::string addr = "tcp://localhost:" + port;
		zmqsocket.connect (addr); // zmq connect
		std::string message = "{\"sensor_num\":" + std::to_string(ssN) + ", \"service_num\":"+std::to_string(1)+"}";
		ZMQSendJson (&zmqsocket, message); // alert sensor num to RL	
	}

	do
	{
		/* Print Setting */
		cout << "Sensor #: " << ssN << endl;
		cout << "Object Speed: " << objectSpeed << endl;
		
		cout << "Wifi DataMode: " << dataMode << endl;
		// Create Object
		ObjectContain *oc = new ObjectContain;
		oc->obsMod = obsMod;
		oc->objectMax = objectMax;
		oc->objectN = objectN;

		oc->log = log;
		oc->trace = trace;
		oc->serId = 0;
		oc->senN = ssN; 
		oc->cellUnit = cellUnit;
		oc->unitN = std::sqrt(ssN); 
		oc->bound = cellUnit*std::sqrt(ssN);
		oc->vel = objectSpeed;
		
		// Node
		NS_LOG_INFO ("Create Node");
		NodeContainer sensorNode;
		NodeContainer serverNode;

		sensorNode.Create (ssN);
		serverNode.Create (1);
		
		// Wifi
		NS_LOG_INFO ("Create Wifi");
		YansWifiChannelHelper channel = YansWifiChannelHelper::Default ();
		YansWifiPhyHelper phy = YansWifiPhyHelper::Default ();
		phy.SetChannel (channel.Create ());
		phy.Set ("Antennas", UintegerValue (4));
		phy.Set ("ShortGuardEnabled", BooleanValue (true));
		phy.Set ("MaxSupportedTxSpatialStreams", UintegerValue (4));
		phy.Set ("MaxSupportedRxSpatialStreams", UintegerValue (4));

		WifiHelper wifi;
		wifi.SetStandard (WIFI_PHY_STANDARD_80211ac);
		wifi.SetRemoteStationManager ("ns3::ConstantRateWifiManager", 
																	"DataMode", StringValue (dataMode),
																	"ControlMode", StringValue ("VhtMcs0"));
		WifiMacHelper mac;
		Ssid ssid = Ssid ("ns-3-ssid");
		
		// NetDevice
		mac.SetType ("ns3::StaWifiMac",
								 //"ActiveProbing", BooleanValue (false), 
								 "Ssid", SsidValue (ssid));
		NetDeviceContainer staDevice;
		staDevice = wifi.Install (phy, mac, sensorNode);

		mac.SetType ("ns3::ApWifiMac",
								 "EnableBeaconJitter", BooleanValue (false),
								 "Ssid", SsidValue (ssid));
		NetDeviceContainer apDevice;
		apDevice = wifi.Install (phy, mac, serverNode);
		
		// Mobility
		NS_LOG_INFO ("Create Mobility");
		MobilityHelper mobility;
		mobility.SetPositionAllocator ("ns3::GridPositionAllocator",
																	 "MinX", DoubleValue (0.0),
																	 "MinY", DoubleValue (0.0),
																	 "DeltaX", DoubleValue (0.0),
																	 "DeltaY", DoubleValue (0.0),
																	 "GridWidth", UintegerValue (1),
																	 "LayoutType", StringValue ("RowFirst"));

		mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
		mobility.Install (sensorNode);
		mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel"); 
		mobility.Install (serverNode);

		// Internet
		NS_LOG_INFO ("Create Internet Stack");
		InternetStackHelper stack;
		stack.InstallAll ();

		// IpAddr
		NS_LOG_INFO ("Create Ipv4 Address");
		Ipv4AddressHelper address; 
		address.SetBase ("10.1.1.0", "255.255.255.0");
		Ipv4InterfaceContainer ip_nToR;
		ip_nToR = address.Assign (staDevice);
		Ipv4InterfaceContainer ip_rToS;
		ip_rToS = address.Assign (apDevice);

		//address.SetBase ("10.2.1.0", "255.255.255.0");
		//Ipv4InterfaceContainer ip_rToS;
		//ip_rToS = address.Assign (apDevice);

		// Routing
		NS_LOG_INFO ("Create Routes");
		Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

		// App Server
		NS_LOG_INFO ("Create Server");
		uint16_t port = 9;
		//UdpEchoServerHelper sinkHelper (port);
		SimpleSinkHelper sinkHelper (port);
		ApplicationContainer sinkApp = sinkHelper.Install (serverNode.Get (0));
		Ptr<Application> app = sinkApp.Get(0);
		Ptr<SimpleSink> simpleSink = DynamicCast<SimpleSink> (app);
		
		simpleSink->log = log;
		simpleSink->upMod = upMod;
		simpleSink->obsMod = obsMod;
		simpleSink->stateMod = stateMod;
		simpleSink->oc = oc;
		simpleSink->ssN = ssN;
		simpleSink->maxStep = maxStep;
		simpleSink->avgRate = sensorAvgRate;
		if (upMod == "rlidagan")
		{
			simpleSink->zmqsocket = &zmqsocket;
		}

		simpleSink->Set ();
		sinkApp.Start (Seconds (0.0));
		sinkApp.Stop (Seconds (40.0));

		// App Sensor
		NS_LOG_INFO ("Create Sensor");
		//UdpEchoClientHelper sensorHelper (ip_rToS.GetAddress(0), port);
		SensorHelper sensorHelper (ip_rToS.GetAddress (0), port);
		std::vector<ApplicationContainer> sensorApp (ssN);
		Address *sensorAddressList = new Address[ssN];
		//sensorHelper.SetAttribute ("MaxPackets", UintegerValue (10));
		//sensorHelper.SetAttribute ("Interval", TimeValue(Seconds(0.05)));
		//sensorHelper.SetAttribute ("PacketSize", UintegerValue (500));
		
		for (uint32_t i=0; i<ssN; i++)
		{
			sensorAddressList[i] = InetSocketAddress (ip_nToR.GetAddress (i,0), port);
			sensorApp[i] = sensorHelper.Install (sensorNode.Get(i));
			Ptr<Application> app = sensorApp[i].Get(0);
			Ptr<SimpleSensor> simpleSensor = DynamicCast<SimpleSensor> (app);
			
			simpleSensor->log = log;
			simpleSensor->upMod = upMod;
			simpleSensor->obsMod = obsMod;
			simpleSensor->senId = i;
			simpleSensor->sampleSize = sampleSize;
			simpleSensor->sampleRate = sensorAvgRate;
			simpleSensor->oc = oc;
			
			simpleSensor->Set ();
			sensorApp[i].Start (Seconds(1.0));
			sensorApp[i].Stop (Seconds(40.0));
		}
	
		simpleSink->addressList = sensorAddressList;
		//phy.EnablePcap ("simple-ns3", apDevice.Get(0));
		
		// Start Object 
		oc->Start ();

		//Simulator::Stop (Seconds(1000.0));
		Simulator::Run ();
		Simulator::Destroy ();
		
		/* End Info */
		std::cout << "###########################" << std::endl;
		cout << "obsMod: " << obsMod << endl;
		cout << "algorithm: " << upMod << endl;
		cout << "speedRate (%): " << speedRate << endl;

		printf("\n[Channel Info]\n");
		cout << "BW Limit: " << bwLimit << "(%) / " << Bit2Mbps(bw) << "(Mbps)" << endl;

		printf("\n[Sensor Info]\n");
		cout << "totSensor #: " << ssN << endl;
		cout << "ini SampleRate: " << sensorAvgRate << "(fps)" << endl;
		
		printf("\n[Sink Info]\n");
		cout << "Total Thr: " << (double) simpleSink->GetTotalRx () * 8 / 1000000 / (simpleSink->endTime - 0.0) << "Mbps" << endl;
		for (uint32_t i=0; i<ssN; i++)
		{
			cout << i << " Sensor Thr: " << (double) simpleSink->recvBytes[i] * 8 / 1000000 / (simpleSink->endTime - 0.0) << "Mbps" << endl;
		}
	
		printf("\n[Simulation Info]\n");
		std::cout << "Simulation Multi Cnt: " << simpleSink->multiCnt/simpleSink->cntAvg << std::endl;
		std::cout << "Simulation Multi Max: " << simpleSink->multiMax << std::endl;
		std::cout << "Simulation Avg Reward: " << simpleSink->rewardAvg/simpleSink->cntAvg  << std::endl;

		delete oc;
		delete[] sensorAddressList;
	}while(rlMod);

	return 0;
}
