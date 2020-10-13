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

int** ReadFile (std::string, int);

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
	std::string actMod = "LA3"; // 1. LA3, 2. else
	std::string simMod = "tempx"; // 1. Temperature 2. Car
	std::string stateMod = "change"; //	1. last 2. change
	std::string testMod = "xtest"; // 1. test
	std::string envMod = "xsumo";

	/** LOG Setting **/
	// ObjectContain
	bool trace = false;
	// CarContain
	bool carInfo = false;
	// Sink
	bool channelInfo = false;
	bool stateInfo = false;
	bool evalInfo = false;
	bool dafuInfo = false;
	uint32_t errorRate = 0; // unit: %
	// Sensor #
	uint32_t ssN = 8;
	uint32_t objectMax = 80; 
	uint32_t bwLimit = 75; // unit: %
	uint32_t objLimit = 20; // unit: %

	// DAFU
	std::string scoreFtn = "halftop"; // 1. optimal 2. halftop
	uint32_t topK = 0;
	uint32_t winSize = 2;

	// SUMO
	int** memoryX = NULL;
	int** memoryY = NULL;

	int history = 300000;
  int interval = 10; //city :  city_nospot :   country : 30  highway : 4 
	
	std::string xPath = "sumo_data/x_city_v2.txt";
	std::string yPath = "sumo_data/y_city_v2.txt";

	/********************
	* Command Setting
	*********************/
	CommandLine cmd;
	cmd.AddValue ("port", "RL Socket Port", port);
	cmd.AddValue ("obsMod", "Obs Mod: track/car/temp/multi", obsMod);
	cmd.AddValue ("actMod", "Act Mod: LA3/else", actMod);
	cmd.AddValue ("netMod", "Network Mode: True/False", netMod);
	cmd.AddValue ("upMod", "Algorithm: uniform/DAFU/rlidagan", upMod);
	cmd.AddValue ("simMod", "Simul Mode: temp/car", simMod);
	cmd.AddValue ("testMod", "Test Mode: test/else", testMod);
	cmd.AddValue ("envMod", "Env Mode: sumo/else", envMod);
	cmd.AddValue ("sInfo", "State Info: true/false", stateInfo);
	cmd.AddValue ("dInfo", "DAFU Info: true/false", dafuInfo);
	cmd.AddValue ("eInfo", "Reward Info: true/false", evalInfo);
	cmd.AddValue ("scoreFtn", "Score Func: optimal/halftop", scoreFtn);
	cmd.AddValue ("ssN", "Sensor #", ssN);
	cmd.AddValue ("objMax", "Object Max", objectMax);
	cmd.AddValue ("bw", "BW Limit: 0~100%", bwLimit);
	cmd.AddValue ("sp", "Object Limit", objLimit);
	cmd.AddValue ("topK", "DAFU Top K Value", topK);
	cmd.AddValue ("error", "Error Rate", errorRate);
	cmd.Parse (argc, argv);
	
	/* RL Setting */
	if (upMod =="rlidagan")
	{
		trace = false;
		carInfo = false;
		channelInfo = false;
		//stateInfo = true;
		//evalInfo = false;
		rlMod = true;
	}
	if (envMod == "sumo")
	{
		memoryX = ReadFile(xPath, history);
		memoryY = ReadFile(yPath, history);
		std::cout<<"File Read Complete!\n";	
	}

	/********************
	* Fixed
	*********************/
	if (ssN==4)
	{
		objectMax = 30;
	}
	else if (ssN==5)
	{
		if (objLimit == 20)
			objectMax = 31;
	}
	else if (ssN==6)
	{ // 28
		if (objLimit == 20)
			objectMax = 40;
	}
	else if (ssN==7)
	{
		if (objLimit == 20)
			objectMax = 48;
	}
	else if (ssN==8)
	{ // 32
		if (objLimit == 10)
			objectMax = 38;
		else if (objLimit == 20)
			objectMax = 54;
		else if (objLimit == 30)
			objectMax = 75;
		else if (objLimit == 40)
			objectMax = 100;
		else if (objLimit == 50)
			objectMax = 135;
	}
	else if (ssN==10)
	{ // 52
		if (objLimit == 20)
			objectMax = 80;
	}
	else if (ssN==12)
	{ // 80
		if (objLimit == 20)
			objectMax = 110;
	}
	else if (ssN==16)
	{ // 160
		if (objLimit == 20)
			objectMax = 200;
	}
	else if (ssN==20)
	{
		if (objLimit == 20)
			objectMax = 330;
	}

	std::cout << "Objet Max: " << objectMax << std::endl;	
			
	// Variable Setting
	uint32_t serviceN = 1; // Service #
	
	uint32_t sensorAvgRate = 60; // When fair share, uint: #/s
	uint32_t sampleSize = 30 * 1024; //1500; // Bytes per Sample
	uint32_t actionPacketSize = 100; // Bytes per Action
	uint32_t senQMaxSize = serviceN; // Sensor Max txQ Size;

	// Object & Map Setting
	uint32_t objectN = 1; // per Service
	if (obsMod=="car" || obsMod=="multi")
		objectN = 0;
	double cellUnit = 2; // unit: m
	double speedRate = 40; // unit: %

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
	double totalTest = 10;
	double cntTest = 0;
	double tempDiffTest = 0;
	double tempAccTest = 0;
	double carErrorTest = 0;
	double singleAccTest = 0;
	double rewardAvgTest = 0;
	double multiCntTest = 0;
	double multiMaxTest = 0;

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
	uint64_t bw =  Byte2Bit (sampleSize) * (uint64_t) ssN * (uint64_t)sensorAvgRate; // bps
	std::cout << "bw: " << bw << std::endl;
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

	// Initial Sample Rate per Service
	uint32_t ini_sampleRate = sensorAvgRate * (double)bwLimit / 100 * ssN / tot_service_ssN;
	do
	{
		// Link Info
		bool isLinkScheWork = false;

		// Create Object
		ObjectContain *oc = new ObjectContain[serviceN];
		oc->obsMod = obsMod;
		oc->envMod = envMod;
		oc->objectMax = objectMax;

		if (envMod == "sumo")
		{
			oc->memoryX = memoryX;
			oc->memoryY = memoryY;
			oc->sumo_interval = interval;
		}
		
		for (uint32_t i=0; i<serviceN; i++)
		{
			ObjectContain *t_oc = &oc[i];
			t_oc->log = log;
			t_oc->trace = trace;
			t_oc->serId = i;
			t_oc->senN = ssN;
			t_oc->objectN = objectN;
			t_oc->cellUnit = cellUnit;
			t_oc->unitN = std::sqrt(ssN);
			t_oc->bound = cellUnit * std::sqrt(ssN);
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
				Service *service = &(sen[j].ser[i]); // Sensor-Service Connection
				service->log = log;
				service->upMod = upMod;
				service->obsMod = obsMod;
				service->enable = true;
				service->oc = &oc[i];
				service->senId = idx;
				service->serId = i;
				service->cellId = j;
				service->sampleSize = sampleSize;
				service->sampleRate = ini_sampleRate * 15/14 - 1;
				service->sampleValue = -100;

				// SetCallback Sensor-Service
				service->InsertTxQ = sen[idx].CallbackInsertTxQ ();
				
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
		}

		// Create Sink Node
		Sink *sink = new Sink;
		sink->log = log;
		sink->upMod = upMod;
		sink->actMod = actMod;
		sink->obsMod = obsMod;
		sink->simMod = simMod;
		sink->stateMod = stateMod;
		sink->testMod = testMod;
		sink->envMod = envMod;

		sink->channelInfo = channelInfo;
		sink->stateInfo = stateInfo;
		sink->evalInfo = evalInfo;
		sink->dafuInfo = dafuInfo;
		sink->oc = oc;
		sink->cc = cc;

		sink->objectN = objectN;
		sink->serviceN = serviceN;
		sink->service_ssN = service_ssN;
		sink->actionPacketSize = actionPacketSize;
		sink->stop = &maxStep;
    sink->avgRate = sensorAvgRate * (double)bwLimit / 100 * 15/14;
		std::cout << "avgRate: " << sink->avgRate << std::endl;
		sink->isLinkScheWork = &isLinkScheWork;
		sink->LinkCheck = link->CallbackCheck ();	
		sink->errorRate = errorRate;
		
		// DAFU Settings
		sink->scoreFtn = scoreFtn;
		sink->topK = topK;
		sink->winSize = winSize;
		sink->ssN = std::sqrt(ssN);
		sink->sssN = ssN;

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

		/********************
		* Start Simulation
		*********************/

		std::cout << "###########################" << std::endl;
		if (envMod == "sumo")
		{
			printf("[[SUMO Setting]]\n");
			cout << xPath << endl;
			cout << yPath << endl;
		}
		printf("\n[[Mode Settings]]\n");
		cout << "Network Mode : " << netMod << endl;
		cout << "Obsev Mode   : " << obsMod << endl;
		cout << "Act   Mode   : " << actMod << endl;
		cout << "Algorithm    : " << upMod;
		
		if (upMod == "DAFU")
			cout << " (K " << topK << ")" << endl;
		else
			cout << endl;
		cout << "speedRate (%): " << speedRate << endl;
		cout << "objLimit  (%): " << objLimit << " (Max " << objectMax << ")" << endl;
	
		printf("\n[[Channel Info]]\n");
		cout << "Sensor Avg Rate: " << sensorAvgRate << std::endl;
		cout << "BW Limit       : " << bwLimit << "(%) / " << Bit2Mbps(bw) << "(Mbps)" << endl;
		if (netMod)
		{
			cout << "[Max] Snesor2Sink Network Latency (ms): " << (double)Byte2Bit(sampleSize) * ssN / bw * 1000 << endl;
			cout << "[Max] Sink2Sensor Network Latency (ms): " << (double)Byte2Bit(actionPacketSize) * ssN / bw * 1000 << endl;
		}
		else
		{
			cout << "No Network Latency !! " << endl;
		}

    printf("\n[[Sensor Info]]\n");
    cout << "Sensor #: " << tot_service_ssN << " / Service #: "  << serviceN << endl;
    cout << "Init Rate: " << ini_sampleRate << "(fps)" << endl; 
		
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
		std::cout << "\n###########################" << std::endl;
		std::chrono::seconds sec = std::chrono::duration_cast<std::chrono::seconds> (end - start);
		std::chrono::milliseconds min = std::chrono::duration_cast<std::chrono::milliseconds> (end - start);
		std::cout << "Simulation Time Duration (Sec): " << sec.count() << std::endl;
		std::cout << "Simulation Time Duration (Milli): " << min.count() << std::endl;
		std::cout << "###########################" << std::endl;
		
		/*
		 * std::cout << "Mx: " << sink->eMa / sink->reward_cnt[0] << std::endl;
		std::cout << "My: " << sink->eMb / sink->reward_cnt[0] << std::endl;
		std::cout << "Vx: " << sink->eVa/ sink->reward_cnt[0] << std::endl;
		std::cout << "Vy: " << sink->eVb / sink->reward_cnt[0] << std::endl;
		std::cout << "cM: " << sink->cMa/ sink->reward_cnt[0] << std::endl;
		std::cout << "cM: " << sink->cMb/ sink->reward_cnt[0] << std::endl;	
		std::cout << "cV: " << sink->cVa / sink->reward_cnt[0] << std::endl;
		std::cout << "cV: " << sink->cVb / sink->reward_cnt[0] << std::endl;

		std::cout << "L: " << sink->eL / sink->reward_cnt[0] << std::endl;
		std::cout << "C: " << sink->eC / sink->reward_cnt[0]<< std::endl;
		std::cout << "S: " << sink->eS / sink->reward_cnt[0] << std::endl;
		std::cout << "Object: " << (double) oc[0].objectM / oc[0].objectG << std::endl;	
		*/

		printf("\n[[Drop Info]]\n");
		std::cout << "Set Drop : " << errorRate << "(%)" << std::endl;
		std::cout << "Drop Rate: " << (double)sink->dropCnt / (double)sink->recvCnt * 100 << "(%)" << std::endl;
		printf("\n[[Reward Info]]\n");
		//std::cout << "ACT Time: " << sink->totalZMQ << std::endl;
		std::cout << "TotalTimestep: " << sink->evalCnt << std::endl;
		std::cout << "Reward Cnt   : " << sink->reward_cnt[0] << std::endl;	
		if (obsMod == "temp")
		{
			std::cout << "Simulation Avg TempAcc: " << sink->tempAcc_avg[0]/sink->reward_cnt[0] << std::endl;
			std::cout << "Simulation Avg TempDiff: " << sink->tempDiff_avg[0]/sink->reward_cnt[0] << std::endl;
		}
		else if (obsMod == "track")
		{
			std::cout << "Simulation Avg SingleAcc: " << (sink->singleAcc_avg[0]/sink->reward_cnt[0])*100 << "%"  << std::endl;
		}
		else if (obsMod == "car")
		{
			std::cout << "Simulation Multi ObjectCnt: " << (sink->multi_cnt/1000.0) << std::endl;
		}
		else if (obsMod == "multi")
		{
			std::cout << "Simulation Multi Cnt: " << (sink->multiCnt/(sink->reward_cnt[0])) << std::endl;
			std::cout << "Simulation Multi Max: " << (sink->multiMax) << std::endl;
		}
		std::cout << "Simulation Avg Reward : " << sink->reward_avg[0]/sink->reward_cnt[0]  << std::endl;


		printf("\n[[Test Info]]\n");
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
			else if (obsMod == "multi")
			{
				multiCntTest += (sink->multiCnt/(sink->reward_cnt[0]));
				multiMaxTest += (sink->multiMax);
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
				else if (obsMod == "multi")
				{
					std::cout << "Simulation Test MultiCnt: " << multiCntTest/totalTest * 100 << " %" << std::endl;	
					std::cout << "Simulation Test MultiMax: " << multiMaxTest/totalTest * 100 << " %" << std::endl;
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

	}while (rlMod || testMod=="test");
	
	delete[] service_ssN;
	delete[] service_ssN2;
	
	if (envMod == "sumo")
	{
		for (int i=0; i<history; i++)
		{
			delete[] memoryX[i];
			delete[] memoryY[i];
		}
		delete[] memoryX;
		delete[] memoryY;
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
	char * case_buffer = new char[1800];
	int i = 0;
	
	if(File.is_open())
	{
		std::string case_line; 
		int* v1;
		while(i<history)
		{
      getline(File,case_line);        
			case_num ++;
			v1 = new int[300];
      
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
		std::cout<< "MAX : "<< case_max <<std::endl;
	}
	
	else
		std::cout<<"[ERROR] :: There are no file exist!"<<std::endl;

	File.close();
	
	delete[] case_buffer;
	return memory;
}


