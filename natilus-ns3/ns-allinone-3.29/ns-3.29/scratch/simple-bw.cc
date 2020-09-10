#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/wifi-module.h"
#include "ns3/internet-module.h"
#include "ns3/mobility-module.h"

#include "simple-ns3/sink-helper.h"
#include "simple-ns3/sink.h"
#include "simple-ns3/sensor-helper.h"
#include "simple-ns3/sensor.h"

using namespace ns3;
NS_LOG_COMPONENT_DEFINE ("simple-bw");
using std::cout;
using std::endl;

int
main (int argc, char *argv[])
{
	//LogComponentEnable ("simple-bw", LOG_LEVEL_INFO);
	//LogComponentEnable ("UdpClient", LOG_LEVEL_INFO);
	//LogComponentEnable ("UdpServer", LOG_LEVEL_INFO);

	/* Application Setting */
	double appStart = 0.0;
	double appEnd = 3.0;

	for (uint32_t i=0; i<9; i++)
	{
		/* Wifi Setting */
		uint32_t dataSpeed = i;
		std::string dataMode = "VhtMcs"+std::to_string(dataSpeed);
		std::cout << "[[DATA MODE: " << dataMode << "]]" << std::endl;

		// Node
		NS_LOG_INFO ("Create Node");
		NodeContainer sensorNode;
		NodeContainer serverNode;

		sensorNode.Create (1);
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

		// Routing
		NS_LOG_INFO ("Create Routes");
		Ipv4GlobalRoutingHelper::PopulateRoutingTables ();
		
		// Create Server
		NS_LOG_INFO ("Create Server");
		uint16_t port = 4000;
		UdpServerHelper server (port);
		ApplicationContainer apps = server.Install (serverNode.Get (0));
		apps.Start (Seconds (appStart));
		apps.Stop (Seconds (appEnd));
		Ptr<Application> app = apps.Get(0);
		Ptr<UdpServer> udps = DynamicCast<UdpServer> (app);

		// Create Client
		NS_LOG_INFO ("Create Client");	
		uint32_t MaxPacketSize = 1472;
		Time interPacketInterval = Seconds (0.000001);
		uint32_t maxPacketCount = 10000000;
		UdpClientHelper client (ip_rToS.GetAddress (0), port);
		client.SetAttribute ("MaxPackets", UintegerValue (maxPacketCount));
		client.SetAttribute ("Interval", TimeValue (interPacketInterval));
		client.SetAttribute ("PacketSize", UintegerValue (MaxPacketSize));
		apps = client.Install (sensorNode.Get (0));
		apps.Start (Seconds (appStart+1.0));
		apps.Stop (Seconds (appEnd));
	
		NS_LOG_INFO ("Run Simulation.");
		Simulator::Stop (Seconds (appEnd));
		Simulator::Run ();
		std::cout << "Num Packet Recv: " << udps->GetReceived () << std::endl;
		std::cout << "Num Packet Lost: " << udps->GetLost () << std::endl; 
		std::cout << "Throughput: " << (double) udps->GetReceived() * MaxPacketSize * 8 / 1000000 / (appEnd - appStart - 1.0) << " Mbps" << std::endl;
		Simulator::Destroy ();
		NS_LOG_INFO ("Done.");
		std::cout << std::endl;
	}
}
