#include "ns3/core-module.h"

#include "function.h"
#include "sensor.h"
#include "sink.h"
#include "link.h"
#include "t-object.h"
#include "car.h"

#include <cstdio>
#include <ctime>
#include <unistd.h> // sleep ();

/********************
*********************/

/********************
* n^2 Square Map

* Observation Mode
* 1. temperature
* 2. tracking

* DA Scheme
* 1. Uniform Update
* 2. RL-iDAGAN
*********************/

using namespace ns3;
using std::cout;
using std::cin;
using std::endl;

/*
template<typename T>
T add (T a, T b)
{
	return a+b;
}
*/

int
main (int argc, char *argv[])
{
	// Log Define(rand () % 8 + 1) * (PI/4.0);

	std::string log = "woo"; // Connected All Class.
	std::string port = "5050"; // ZMQ Default Port
		
	/********************
	* Variable Setting
	*********************/
	srand (time(NULL));
	uint64_t maxStep = 1000;

	// Mode Setting
	bool rlMod = false;
	bool netMod = true; // not impletation for false...
	std::string obsMod = "multi"; // 1. temp, 2. track 3. car
	std::string upMod = "rlidagan"; //1. uniform 2. DAFU  3. rlidagan
	std::string simMod = "tempx"; // 1. Temperature 2. Car
	std::string stateMod = "change"; //	1. last 2. change
	std::string testMod = "xtest"; // 1. test

	/** LOG Setting **/
	// ObjectContain
	bool trace = false;
	// CarContain
	bool carInfo = false;
	// Sink
	bool channelInfo = false;
	bool stateInfo = false;
	bool evalInfo = false;
	
	// Sensor #
	uint32_t ssN = 8;

	/********************
	* Command Setting
	*********************/
	CommandLine cmd;
	cmd.AddValue ("port", "RL Socket Port", port);
	cmd.AddValue ("obsMod", "Obs Mod: track/car/temp/multi", obsMod);
	cmd.AddValue ("netMod", "Network Mode: True/False", netMod);
	cmd.AddValue ("upMod", "Algorithm: uniform/DAFU/rlidagan", upMod);
	cmd.AddValue ("simMod", "Simul Mode: temp/car", simMod);
	cmd.AddValue ("testMod", "Test Mode: test/else", testMod);
	cmd.AddValue ("ssN", "Sensor #", ssN);
	cmd.AddValue ("sInfo", "State Info: true/false", stateInfo);
	cmd.Parse (argc, argv);

	if (upMod =="rlidagan")
	{
		trace = false;
		carInfo = false;
		channelInfo = false;
		//stateInfo = true;
		evalInfo = false;
		rlMod = true;
	}

	/********************
	* Fixed
	*********************/

	
	// Variable Setting
	uint32_t serviceN = 1; // Service #
	uint32_t bwLimit = 50; // unit: %

	uint32_t sensorAvgRate = 60; // When fair share, uint: #/s
	uint32_t sampleSize = 1500; // Bytes per Sample
	uint32_t actionPacketSize = 100; // Bytes per Action
	uint32_t senQMaxSize = serviceN; // Sensor Max txQ Size;

	// Object & Map Setting
	uint32_t objectN = 1; // per Service
	if (obsMod=="car" || obsMod=="multi")
		objectN = 0;
	double cellUnit = 2; // unit: m
	double speedRate = 40; // unit: %
	uint32_t objectMax = 80; // used in car obsMod

	// Car Setting 
	std::string navFunc = "Greedy"; // Random, Greedy	
	uint32_t ccN = 10;
	double threshTmp = 40.0; // Temperature Threshold
	
	// ZMQ Setting
	zmq::context_t zmqcontext {1}; // zmq context
	zmq::socket_t zmqsocket {zmqcontext, ZMQ_REQ}; // zmq socket
	if (upMod == "rlidagan")
	{
		std::string addr = "tcp://localhost:" + port;
		zmqsocket.connect (addr); // zmq connect
		std::string message = "{\"sensor_num\":" + std::to_string(ssN * ssN) + ", \"service_num\":"+std::to_string(serviceN)+"}";
		ZMQSendJson (&zmqsocket, message); // alert sensor num to RL
	}
	
	// Test Mode Setting
	double totalTest = 2000;
	double cntTest = 0;
	double tempDiffTest = 0;
	double tempAccTest = 0;
	double carErrorTest = 0;
	double singleAccTest = 0;
	double rewardAvgTest = 0;

	/********************
	* Automatic Calculation
	*********************/

	// Object & Map
	double maxSpeed = cellUnit * sensorAvgRate; // m/s
	double objectSpeed = maxSpeed * speedRate / 100;
	std::cout << "Object Speed: " << objectSpeed << std::endl;
	// Sensor
	ssN = ssN * ssN;

	// Bandwidth
  //uint64_t totRate = ssN * sensorAvgRate;
	uint64_t bw = Byte2Bit (sampleSize) * ssN * sensorAvgRate; // bps
	bw = (double) serviceN * bw * bwLimit / 100;

	/********************
	* Start Main
	*********************/

	// How many node need to create service#
	uint32_t *service_ssN = new uint32_t[serviceN];
	service_ssN[0] = ssN;
	uint32_t tot_service_ssN = ssN;

	uint32_t *service_ssN2 = new uint32_t[serviceN];
	service_ssN2[0] = std::sqrt(ssN);

	if (serviceN > 1)
	{
		printf("This is Multi-service Scenario\n");
		printf("Input Sensor # (#^2) per Service (Except Service #0)\n");
		for (uint32_t i = 1; i<serviceN; i++)
		{
			do{
				printf("Service #%d: ", i);
				uint32_t ssN2= std::sqrt(ssN) ;
                                //cin >> ssN2;
				service_ssN[i] = ssN2*ssN2;
				service_ssN2[i] = ssN2;
			}while (service_ssN[0] < service_ssN[i]);
			tot_service_ssN += service_ssN[i];
		}
	}

	// Initial Sample Rate per Service
	uint32_t ini_sampleRate = sensorAvgRate * (double)bwLimit / 100 * ssN / tot_service_ssN;
	do
	{
		// Link Info
		bool isLinkScheWork = false;

		// Create Object
		ObjectContain *oc = new ObjectContain[serviceN];
		oc->obsMod = obsMod;
		if (obsMod=="car" || obsMod=="multi")
			oc->objectMax = objectMax;
		else
			oc->objectMax = objectN;

		for (uint32_t i=0; i<serviceN; i++)
		{
			ObjectContain *t_oc = &oc[i];
			t_oc->log = log;
			t_oc->trace = trace;
			t_oc->serId = i;
			t_oc->senN = service_ssN[i];
			t_oc->objectN = objectN;
			t_oc->cellUnit = cellUnit;
			t_oc->unitN = service_ssN2[i];
			t_oc->bound = cellUnit * service_ssN2[i];
			t_oc->vel = objectSpeed;
		}
	
		// Create Link
		Link *link = new Link;
		link->netMod = netMod;
		link->isLinkScheWork = &isLinkScheWork;
		link->ssN = ssN;
		link->bw = bw;

		// Create Sesnor Node
		Sensor* sen = new Sensor[ssN];
		for (uint32_t i=0; i<ssN; i++)
		{
			sen[i].sensorId = i;
			sen[i].serviceN = serviceN;
			sen[i].qMaxSize = senQMaxSize;
			sen[i].ServiceListGen();
			sen[i].isLinkScheWork = &isLinkScheWork;
		}
		// Create Sensor-Service Table
		uint32_t **table = Arr2Create<uint32_t>(serviceN, ssN, ssN);

		// Create Service on Sensor
		for (uint32_t i=0; i<serviceN; i++)
		{
			uint32_t* check = Arr1Create<uint32_t> (ssN, 0);
			for (uint32_t j=0; j<service_ssN[i]; j++)
			{
				uint32_t idx;
				do
				{
					idx = rand() % ssN;
				}while (check[idx]);
				check[idx] = 1;
				
				// Insert Service Ini Info
				Service *service = &(sen[idx].ser[i]); // Sensor-Service Connection
				service->log = log;
				service->upMod = upMod;
				service->obsMod = obsMod;
				service->enable = true;
				service->oc = &oc[i];
				service->senId = idx;
				service->serId = i;
				service->cellId = j;
				service->sampleSize = sampleSize;
				service->sampleRate = ini_sampleRate;
				service->sampleValue = -100;

				// SetCallback Sensor-Service
				service->InsertTxQ = sen[idx].CallbackInsertTxQ ();
				
				// Insert Sensor-Service Table
				table[i][j] = idx;
			}
			delete[] check;
		}

		// Create Car Object
		CarContain *cc = new CarContain[serviceN];
		for (uint32_t i=0; i<serviceN; i++)
		{
			CarContain *t_cc = &cc[i];
			t_cc->oc = &oc[i];
			t_cc->carN = ccN;
			t_cc->navFunc = navFunc;
			t_cc->carInfo = carInfo;
			t_cc->threshTmp = threshTmp;
			//t_cc->state = &(sink->state[i]); // mkris ???
		}

		// Create Sink Node
		Sink *sink = new Sink;
		sink->log = log;
		sink->upMod = upMod;
		sink->obsMod = obsMod;
		sink->simMod = simMod;
		sink->stateMod = stateMod;
		sink->channelInfo = channelInfo;
		sink->stateInfo = stateInfo;
		sink->evalInfo = evalInfo;
		sink->oc = oc;
		sink->cc = cc;
		sink->objectN = objectN;
		sink->serviceN = serviceN;
		sink->service_ssN = service_ssN;
		sink->actionPacketSize = actionPacketSize;
		sink->stop = &maxStep;
    sink->avgRate = sensorAvgRate * (double)bwLimit / 100;
		sink->isLinkScheWork = &isLinkScheWork;
		sink->LinkCheck = link->CallbackCheck ();
		


		// ZMQ 
		if (upMod == "rlidagan")
		{
			sink->zmqsocket = &zmqsocket;
		}

		// Connection Sensor-Link-Sink
		for (uint32_t i=0; i<ssN; i++)
		{
			sen[i].log = log;
			sen[i].LinkCheck = link->CallbackCheck ();
		}
		link->log = log;
		link->sen = sen;
		link->sink = sink;
		link->SinkRecvSche = sink->CallbackRecvSche ();
		link->txTable = table;


		/********************
		* Start Simulation
		*********************/

		std::cout << "###########################" << std::endl;
		cout << "netMod: " << netMod << endl;
		cout << "obsMod: " << obsMod << endl;
		cout << "algorithm: " << upMod << endl;
		cout << "speedRate (%): " << speedRate << endl;
		printf("\n[Channel Info]\n");
		cout << "Sensor Avg Rate: " << sensorAvgRate << std::endl;
		cout << "BW Limit: " << bwLimit << "(%) / " << Bit2Mbps(bw) << "(Mbps)" << endl;
		if (netMod)
		{
			cout << "[Max] Snesor2Sink Network Latency (ms): " << (double)Byte2Bit(sampleSize) * ssN / bw * 1000 << endl;
			cout << "[Max] Sink2Sensor Network Latency (ms): " << (double)Byte2Bit(actionPacketSize) * ssN / bw * 1000 << endl;
		}
		else
		{
			cout << "No Network Latency !! " << endl;
		}

    printf("\n[Sensor Info]\n");
    cout << "totSensor #: " << tot_service_ssN << " / Service #: "  << serviceN << endl;
    cout << "ini SampleRate: " << ini_sampleRate << "(fps)" << endl;
    printf("\n[Sink Info]\n");
		
		if (!rlMod && !(testMod == "test"))
		{
			printf("\n No RL-Mode !! \n");
			printf("Run Wait 1s\n");
			sleep (1);
		}
		printf("\nStart !\n");

		// Object Start
		for (uint32_t i=0; i<serviceN; i++)
		{
			oc[i].Start ();
			cc[i].Start ();	
		}
		
		// Link, Sink, Service Start
		isLinkScheWork = true;
		link->Check ();
		sink->Start ();
		for (uint32_t i=0; i<ssN; i++)
		{
			for (uint32_t j=0; j<serviceN; j++)
			{
				sen[i].ser[j].Start ();
			}
		}

		
		/********************
		* Simulation Run
		*********************/
		std::chrono::system_clock::time_point start = std::chrono::system_clock::now();
		Simulator::Run ();
		Simulator::Destroy ();
		std::chrono::system_clock::time_point end = std::chrono::system_clock::now();

		/********************
		* End Info
		*********************/
		std::cout << "###########################" << std::endl;
		cout << "netMod: " << netMod << endl;
		cout << "obsMod: " << obsMod << endl;
		cout << "algorithm: " << upMod << endl;
		cout << "speedRate (%): " << speedRate << endl;
		printf("\n[Channel Info]\n");
		cout << "BW Limit: " << bwLimit << "(%) / " << Bit2Mbps(bw) << "(Mbps)" << endl;
		if (netMod)
		{
			cout << "[Max] Snesor2Sink Network Latency (ms): " << (double)Byte2Bit(sampleSize) * ssN / bw * 1000 << endl;
			cout << "[Max] Sink2Sensor Network Latency (ms): " << (double)Byte2Bit(actionPacketSize) * ssN / bw * 1000 << endl;
		}
		else
		{
			cout << "No Network Latency !! " << endl;
		}
		printf("\n[Sensor Info]\n");
		cout << "totSensor #: " << ssN << " / Service #: "  << serviceN << endl;
		cout << "ini SampleRate: " << ini_sampleRate << "(fps)" << endl;

		std::chrono::seconds sec = std::chrono::duration_cast<std::chrono::seconds> (end - start);
		std::chrono::milliseconds min = std::chrono::duration_cast<std::chrono::milliseconds> (end - start);
		std::cout << "Simulation Time Duration (Sec): " << sec.count() << std::endl;
		std::cout << "Simulation Time Duration (Milli): " << min.count() << std::endl;
		std::cout << "###########################" << std::endl;
		for (uint32_t i=0; i<serviceN; i++)
		{
			std::cout << "\nService num: " << i << std::endl;
			std::cout << "TotalTimestep: " << sink->evalCnt << std::endl;
			if (obsMod == "temp")
			{
				std::cout << "Simulation Avg TempAcc: " << sink->tempAcc_avg[i]/sink->reward_cnt[i] << std::endl;
				std::cout << "Simulation Avg TempDiff: " << sink->tempDiff_avg[i]/sink->reward_cnt[i] << std::endl;
			}
			else if (obsMod == "track")
			{
				std::cout << "Simulation Avg SingleAcc: " << (sink->singleAcc_avg[i]/sink->reward_cnt[i])*100 << "%"  << std::endl;
			}
			else if (obsMod == "car")
			{
				std::cout << "Simulation Multi ObjectCnt: " << (sink->multi_cnt/1000.0) << std::endl;
			}
			std::cout << "Simulation Avg Reward: " << sink->reward_avg[i]/sink->reward_cnt[i]  << std::endl;
		}

		if (testMod == "test")
		{
			cntTest += 1;
			
			if (obsMod == "temp")
			{
				tempAccTest += sink->tempAcc_avg[0]/sink->reward_cnt[0]; 
				tempDiffTest += sink->tempDiff_avg[0]/sink->reward_cnt[0];
			}
			else if (obsMod == "track")
			{
				singleAccTest += sink->singleAcc_avg[0]/sink->reward_cnt[0];
			}
			rewardAvgTest += sink->reward_avg[0]/sink->reward_cnt[0];

			std::cout << "Test Cnt: " << cntTest << std::endl;
			if (cntTest >= totalTest) 
			{
				std::cout << "Total Test: " << totalTest << std::endl;
				if (obsMod == "temp")
				{
					std::cout << "Simulation Test TempAcc: " << tempAccTest/totalTest << std::endl;
					std::cout << "Simulation Test TempDiff: " << tempDiffTest/totalTest << std::endl;
					std::cout << "Simulation Test CarError: " << carErrorTest/totalTest << std::endl;
				}	
				else if (obsMod == "track")
				{
					std::cout << "Simulation Test SingleAcc: " << singleAccTest/totalTest * 100 << " %" << std::endl;
				}
				std::cout << "Simulation Test RewardAvg: " << rewardAvgTest/totalTest << std::endl;
				break;
			}
		}
		

		/********************
		* Memory Free
		*********************/
		delete[] sen;
		delete sink;
		delete link;
		delete[] oc;
		delete[] cc;
		Arr2Delete<uint32_t> (table, serviceN);

	}while (rlMod || testMod=="test");
	
	delete[] service_ssN;
	delete[] service_ssN2;

	return 0; 
}

