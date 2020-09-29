#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/wifi-module.h"
#include "ns3/point-to-point-module.h"
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

int** ReadFile (std::string path, int history);

int
main (int argc, char *argv[])
{
	std::string log = "woox";
	std::string port = "5050";
	
	srand (time(NULL));
	uint64_t maxStep = 100;

	/* Mode Setting */
	bool rlMod = false;
	std::string env = "wireless";
	std::string obsMod = "multi"; // multi/sumo
	std::string upMod = "uniform"; // uniform/DAFU/rlidagan
	std::string simMod = "x";
	std::string stateMod = "change";
	std::string testMod = "false";

	/* Log Setting */
	bool trace = false;
	bool stateInfo = false;
	bool dafuInfo = false;

	/* DAFU Setting */
	std::string dafuFtn = "around";
	
	/* Sample Setting */
	uint32_t sensorAvgRate = 60;	// unit: #/s
	uint32_t bwLimit = 75;				// unit: %
	uint32_t frameSize = 30;			// unit: KB (IP Camera)
	uint32_t sampleSize = 1472;		// unit: Bytes per one packet
	uint32_t sampleNum = (uint32_t) std::ceil ((double) frameSize * 1024 / sampleSize); 
																// Number of Packets to send
	/* Object Setting */
	uint32_t ssN = 8;
	uint32_t objectN = 0;
	uint32_t objectMax = 80;
	uint32_t objectLimit = 20;
	double cellUnit = 2; // unit: m
	
	double maxSpeed = cellUnit*sensorAvgRate; // m/s
	double speedRate = 40; //unit: %
	double objectSpeed = maxSpeed*speedRate / 100;
		
	/* Wifi Setting */
	uint32_t dataSpeed = 8; // Size 4: 3, Size 6: 2, Size 8: 5,  
	std::string dataMode = "VhtMcs"+std::to_string(dataSpeed);
	
	/* Application Setting */
	double appStart = 1.0;
	double appEnd = 100.0;

	/* Command Setting */
	CommandLine cmd;
	cmd.AddValue ("port", "Socket Port", port);
	cmd.AddValue ("obsMod", "Obs Mode", obsMod);
	cmd.AddValue ("upMod", "Algorithm: uniform/DAFU/rlidagan", upMod);
	cmd.AddValue ("ssN", "Sesnsor #", ssN);
	cmd.AddValue ("frame", "Frame Size", frameSize);
	cmd.AddValue ("bwLimit", "BW Limit %", bwLimit);
	cmd.AddValue ("objLimit", "Object Max", objectMax);
	cmd.AddValue ("sInfo", "State Info", stateInfo);
	cmd.AddValue ("dInfo", "DAFU Info", dafuInfo);
	cmd.Parse (argc, argv);
	
	/* Read Sumo File */
	int** memory_X = NULL;
	int** memory_Y = NULL;

	int history = 300000;
	if(obsMod =="sumo")
	{	
		std::string xPath = "data/x_v6.txt";
		std::string yPath = "data/y_v6.txt";

		memory_X = ReadFile(xPath, history);
		memory_Y = ReadFile(yPath, history);
		std::cout<<"File Read Complete!\n";
	}

	if (ssN==4)
	{
		objectMax = 30;
	}
	else if (ssN==5)
	{
		if (objectLimit == 20)
			objectMax = 31;
	}
	else if (ssN==6)
	{ // 28
		if (objectLimit == 20)
			objectMax = 40;
	}
	else if (ssN==7)
	{
		if (objectLimit == 20)
			objectMax = 48;
	}
	else if (ssN==8)
	{ // 32
		if (objectLimit == 10)
			objectMax = 38;
		else if (objectLimit == 20)
			objectMax = 54;
		else if (objectLimit == 30)
			objectMax = 75;
		else if (objectLimit == 40)
			objectMax = 100;
		else if (objectLimit == 50)
			objectMax = 135;
	}
	else if (ssN==10)
	{ // 52
		if (objectLimit == 20)
			objectMax = 80;
	}
	else if (ssN==12)
	{ // 80
		if (objectLimit == 20)
			objectMax = 100;
	}
	else if (ssN==16)
	{ // 160
		if (objectLimit == 20)
			objectMax = 200;
	}

	ssN = ssN*ssN;
	sensorAvgRate = sensorAvgRate * bwLimit / 100;
	
	uint64_t bw = Byte2Bit(sampleSize) * ssN *sensorAvgRate;
	uint32_t totalSize = sampleNum * sampleSize;
	double needsThr = (double) totalSize * 8 * sensorAvgRate * ssN / 1000000;
	
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
		cout << "\n==================================" << endl;
		cout << "[[Sensor Setting]]" << endl;
		cout << "Sensor #: " << ssN << endl << endl;;
		cout << "[[Object Setting]]" << endl;
		cout << "Object Speed: " << objectSpeed << endl << endl;
		cout << "[[Sample Setting]]" << endl;
		cout << "Frame Size: " << frameSize << " KB (" << frameSize*1024 << " bytes)" <<  endl;
		cout << "Sample Num: " << sampleNum << endl;
		cout << "Needs Thr : " << needsThr << " Mbps" << endl << endl;
		cout << "[[Action Setting]]" << endl;
		cout << "Needs Thr : " << (double) 10 * 8 * 30 * ssN / 1000000 << " Mbps" << endl <<  endl;  
		cout << "[[Wifi Setting]]" << endl;
		cout << "Wifi DataMode: " << dataMode << endl << endl;

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
		oc->firstTime = appStart;
		if (obsMod == "sumo")
		{
			oc->memory_X = memory_X;
			oc->memory_Y = memory_Y;
		}

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
		//phy.Set ("ChannelWidth", UintegerValue (160));
		phy.Set ("Antennas", UintegerValue (4));
		//phy.Set ("ShortGuardEnabled", BooleanValue (true));
		phy.Set ("MaxSupportedTxSpatialStreams", UintegerValue (4));
		phy.Set ("MaxSupportedRxSpatialStreams", UintegerValue (4));
		//phy.Set ("ChannelNumber", UintegerValue (3));

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
																	 "DeltaX", DoubleValue (1.0),
																	 "DeltaY", DoubleValue (1.0),
																	 "GridWidth", UintegerValue (std::sqrt(ssN)),
																	 "LayoutType", StringValue ("RowFirst"));

		mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
		mobility.Install (sensorNode);
		mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel"); 
		mobility.Install (serverNode);
		
		// Config
		//Config::Set("/NodeList/*/DeviceList/*/$ns3::WifiNetDevice/Mac/$ns3::RegularWifiMac/Txop/MinCw", UintegerValue (64));
	
		// P2P Control
		NS_LOG_INFO ("Create P2P");
		PointToPointHelper p2p;
		p2p.SetDeviceAttribute ("DataRate", StringValue ("100Mbps"));
		p2p.SetChannelAttribute ("Delay", StringValue ("0ms"));
		
		std::vector<NetDeviceContainer> p2pDevice (ssN);
		for (uint32_t i=0; i<ssN; i++)
		{
			NodeContainer ss;
			ss.Add (sensorNode.Get(i));
			ss.Add (serverNode.Get(0));
			NetDeviceContainer device = p2p.Install(ss);
			p2pDevice[i] = device;
		}
		
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
		
		std::vector<Ipv4InterfaceContainer> p2pIp (ssN);
		for (uint32_t i=0; i<ssN; i++)
		{
			std::string addr = "10." + std::to_string(2+i) + ".1.0";	
			Ipv4Address ipv4 (addr.c_str());
			address.SetBase (ipv4, "255.255.255.0");
			Ipv4InterfaceContainer ip = address.Assign (p2pDevice[i]);
			p2pIp[i] = ip;
		}

		// Routing
		NS_LOG_INFO ("Create Routes");
		Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

		// App Server
		NS_LOG_INFO ("Create Server");
		uint16_t port = 4000;
		//UdpEchoServerHelper sinkHelper (port);
		SimpleSinkHelper sinkHelper (port);
		ApplicationContainer sinkApp = sinkHelper.Install (serverNode.Get (0));
		Ptr<Application> app = sinkApp.Get(0);
		Ptr<SimpleSink> simpleSink = DynamicCast<SimpleSink> (app);
		
		simpleSink->log = log;
		simpleSink->upMod = upMod;
		simpleSink->obsMod = obsMod;
		simpleSink->stateMod = stateMod;
		simpleSink->stateInfo = stateInfo;
		simpleSink->dafuInfo = dafuInfo;

		simpleSink->oc = oc;
		simpleSink->ssN = ssN;
		simpleSink->maxStep = maxStep;
		simpleSink->avgRate = sensorAvgRate;
		simpleSink->sampleNum = sampleNum;
		
		simpleSink->dafuFtn = dafuFtn;

		if (upMod == "rlidagan")
		{
			simpleSink->zmqsocket = &zmqsocket;
		}

		simpleSink->Set ();
		sinkApp.Start (Seconds (appStart - 1.0)); // 0.0 sec
		sinkApp.Stop (Seconds (appEnd));

		// App Sensor
		NS_LOG_INFO ("Create Sensor");
		//UdpEchoClientHelper sensorHelper (ip_rToS.GetAddress(0), port);
		SensorHelper sensorHelper (ip_rToS.GetAddress (0), port);
		std::vector<ApplicationContainer> sensorApp (ssN);
		Address *sensorAddressList = new Address[ssN];
		Address *sensorP2PList = new Address[ssN];
		//sensorHelper.SetAttribute ("MaxPackets", UintegerValue (10));
		//sensorHelper.SetAttribute ("Interval", TimeValue(Seconds(0.05)));
		//sensorHelper.SetAttribute ("PacketSize", UintegerValue (500));
		
		for (uint32_t i=0; i<ssN; i++)
		{
			sensorAddressList[i] = InetSocketAddress (ip_nToR.GetAddress (i,0), port);	
			sensorP2PList[i] = InetSocketAddress (p2pIp[i].GetAddress(0, 0), i+1);
			sensorApp[i] = sensorHelper.Install (sensorNode.Get(i));
			Ptr<Application> app = sensorApp[i].Get(0);
			Ptr<SimpleSensor> simpleSensor = DynamicCast<SimpleSensor> (app);
			
			simpleSensor->log = log;
			simpleSensor->upMod = upMod;
			simpleSensor->obsMod = obsMod;
		
			simpleSensor->senId = i;
			simpleSensor->sampleSize = sampleSize;
			simpleSensor->sampleNum = sampleNum;
			simpleSensor->sampleRate = sensorAvgRate * 15/14;
			simpleSensor->oc = oc;
			
			simpleSensor->Set ();
			sensorApp[i].Start (Seconds(appStart - 0.5)); // 0.5 sec
			sensorApp[i].Stop (Seconds(appEnd));
		}
	
		simpleSink->addressList = sensorAddressList;
		simpleSink->addressP2P = sensorP2PList;
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
		cout << "Total Thr: " << (double) simpleSink->GetTotalRx () * 8 / 1000000 / (simpleSink->endTime - 1.0) << "Mbps" << endl;
		for (uint32_t i=0; i<ssN; i++)
		{
			cout << i << " Sensor Thr: " << (double) simpleSink->recvBytes[i] * 8 / 1000000 / (simpleSink->endTime - 1.0) << "Mbps" << endl;
		}
		
		printf("\n[Loss Info]\n");
		std::cout << "Loss Pakcet: " << simpleSink->lossPkt << std::endl;
		std::cout << "Loss Frame: " << simpleSink->lossFrm << std::endl;

		printf("\n[Simulation Info]\n");
		std::cout << "Simulation Multi Cnt: " << simpleSink->multiCnt/simpleSink->cntAvg << std::endl;
		std::cout << "Simulation Multi Max: " << simpleSink->multiMax << std::endl;
		std::cout << "Simulation Avg Reward: " << simpleSink->rewardAvg/simpleSink->cntAvg  << std::endl;
		
		delete oc;	
		delete[] sensorAddressList;
		delete[] sensorP2PList;
	}while(rlMod);
	
	if (obsMod == "sumo")
	{
		for(int i =0; i<history; i++)
		{
			delete[] memory_X[i];
			delete[] memory_Y[i];
		}
		delete[] memory_X;
		delete[] memory_Y;
	}
	return 0;
}

int** ReadFile(std::string path, int history)
{
	int** memory = new int*[sizeof(int*)*history];

	std::ifstream File(path.data());
	int case_num = 0;
	int case_max = 0;
	int case_temp = 0;
	char * tok ;
	char * case_buffer = new char[600];
	int i = 0;
	
	if(File.is_open())
	{
		std::string case_line; 
		int* v1;
	
		while(i<history)
		{
      getline(File,case_line);        
			case_num ++;
			v1 = new int[100];
      
			int j = 0;
			strcpy(case_buffer, case_line.c_str());
			tok = strtok(case_buffer," ");
			while(tok!=NULL)
			{
				v1[j] =atoi(tok);
				case_temp ++;
				tok = strtok(NULL, " ");
				j++;
			}
			memory[i] = v1;
			if(case_temp>case_max)
				case_max = case_temp;
			case_temp = 0;
			i++;	
		}
		std::cout<<"MAX : "<<case_max<<std::endl;
	}	
	else
		std::cout<<"[ERROR] :: There are no file exist!"<<std::endl;

	File.close();
	
	delete[] case_buffer;
	return memory;
}
