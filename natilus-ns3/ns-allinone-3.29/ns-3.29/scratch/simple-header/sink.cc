#include "sink.h"
#include <math.h>

namespace ns3{

Sink::Sink ()
{
}

Sink::~Sink ()
{
  delete[] carReward;

  delete[] state;
  delete[] history;

  delete[] trackAcc;
  delete[] tempAcc;
  delete[] tempDiff;
  delete[] reward;
	delete[] cnt;
  delete[] tempAcc_avg;
  delete[] tempDiff_avg;
  delete[] reward_avg;
	delete[] reward_cnt;
	delete[] singleAcc_avg;
  delete[] threshold;

  for(uint32_t i = 0; i<serviceN ; i++)
  {
    delete[] trackMap[i];
    delete[] p_tempMap[i];
  }
  delete[] p_tempMap[serviceN];
  delete[] trackMap;
  delete[] p_tempMap;
}

void Sink::Start (void)
{
	episodeStart = true;
	episodeEnd = false;
	carReward = new double[serviceN];

	state = new STATE[serviceN];
	history = new std::queue<STATE>[serviceN];

	trackAcc = new double[serviceN];
	tempAcc = new double[serviceN];
	tempDiff = new double[serviceN];
	reward = new double[serviceN];
	cnt = new double[serviceN];
	tempAcc_avg = new double[serviceN];
	tempDiff_avg = new double[serviceN];
	reward_avg = new double[serviceN];
	reward_cnt = new double[serviceN];
	singleAcc_avg = new double[serviceN];
  trackMap = new double*[serviceN];
  p_tempMap = new double*[serviceN+1];
	threshold = new double[serviceN];

  for(uint32_t i = 0; i<serviceN ; i++)
  {
    trackMap[i] = new double[service_ssN[i]];
    p_tempMap[i] = new double[service_ssN[i]];
  }
	p_tempMap[serviceN] = new double[1000];

	// Initailize
	for (uint32_t i=0; i<serviceN; i++)
	{
		carReward[i] = 0;
		trackAcc[i] = 0;
		tempAcc[i] = 0;
		tempDiff[i] = 0;
		reward[i] = 0;
		cnt[i] = 0;
		tempAcc_avg[i] = 0;
		tempDiff_avg[i] = 0;
		reward_avg[i] = 0;
		reward_cnt[i] = 0;
		singleAcc_avg[i] = 0;
		threshold[i] = oc[i].object[0].temp;

		for (uint32_t j=0; j<1000; j++)
			state->sampleCar[j] = -1;
	}
}


void Sink::Send (void)
{
	haveSendAction = true;

	while (!txQ.empty())
	{ 
		DATA *removeAct = txQ.front();
		txQ.pop ();
		delete removeAct;
	}

	for (uint32_t i=0; i<serviceN; i++)
	{
		uint32_t ssN = service_ssN[i];
		for (uint32_t j=0; j<ssN; j++)
		{
			DATA *data = new DATA;
			data->dataSize = actionPacketSize;
			data->genTime = Simulator::Now();
			data->serId = i;
			data->cellId = j;
			data->action = state[i].action[j];

			txQ.push (data);
		}
	}

	if (!(*isLinkScheWork))
	{
		*isLinkScheWork = true;
		LinkCheck ();
	}
}


void Sink::Recv (std::queue<DATA*> *dataContain)
{
	while ( !(dataContain->empty ()) )
	{
		uint64_t now = Simulator::Now().GetMilliSeconds ();
		DATA *data = dataContain->front ();
		dataContain->pop ();

		// Get Data Info
		uint32_t serId = data->serId;
		uint32_t cellId = data->cellId;

		// Measurement
		uint64_t delay = now - (data->genTime).GetMilliSeconds ();
		totRecvByte += data->dataSize;
		totRecvCnt += 1;
		totDelay += delay;

		// Insert Info
		state[serId].sampleRate[cellId] = data->sampleRate;
		// If is track, first clear the map
		if (obsMod == "track")
		{
			if (data->sampleValue == 1)
			{
				for (uint32_t i=0; i<service_ssN[0]; i++)
				{
					state[serId].sampleValue[i] = 0;
				}
			}
		}
	
		if (obsMod == "car")
		{
			for (uint32_t i=0; i<oc[serId].objectMax; i++)
				if (data->carCell[i] == true)
					state[serId].sampleCar[i] = (int)cellId;

			for (uint32_t i=0; i<service_ssN[0]; i++)
				state[serId].sampleValue[i] = 0;

			for (uint32_t i=0; i<oc[serId].objectMax; i++)
			{
				int cell = state[serId].sampleCar[i];
				if (cell >= 0)
					state[serId].sampleValue[cell] += 1;
			}
		}
		else
			state[serId].sampleValue[cellId] = data->sampleValue;
		state[serId].upInter[cellId] = delay;
		state[serId].lastUpdateTime[cellId] = Simulator::Now (); 

		delete data;
	}	
	delete dataContain;

	if (firstEval)
	{
		// When the first packet is recv, Start
		firstEval = false;

		double commTime = 1.0/30.0;
		Simulator::Schedule (Seconds(commTime), &Sink::Communication, this);
		double evalTime = 1.0/(double)(rand() % 30 + 1 + 120);
		Simulator::Schedule (Seconds (evalTime), &Sink::Eval, this);

		if (channelInfo)
			PrintChannel ();
	}
}

void Sink::RecvSche (std::queue<DATA*> *dataContain, uint64_t micro_delay)
{
	Simulator::Schedule (MicroSeconds (micro_delay), &Sink::Recv, this, dataContain);
}

void Sink::Eval ()
{
	/* Truth Update */
	for (uint32_t i=0; i<serviceN; i++)
	{
		oc[i].Moving();
	}

	//if (stateInfo)
	//	PrintInfo ();
	
	// Evaluation Method
	Evaluation ();

		
	double evalTime = 1.0/(double)(rand() % 30 + 1 + 120);
	Simulator::Schedule (Seconds (evalTime), &Sink::Eval, this);
}

void Sink::Communication ()
{

	if (stateInfo)
		PrintInfo ();

	for (uint32_t i=0; i<serviceN; i++)
	{
		reward[i] = reward[i] / cnt[i];
		if (cnt[i] == 0)
			reward[i] = 0;
	}

	if (upMod == "rlidagan")
	{
		ZMQCommunication ();
		Send();
	}
	else if (upMod == "DAFU")
	{
		DAFU();
		Send();
	}

	if (evalInfo)
		PrintEval();
	
	for (uint32_t i=0; i<serviceN; i++)
	{
		cnt[i] = 0;
		reward[i] = 0;
	}

	if (upMod != "rlidagan")
	{
		// Simulation Destroy
		if (evalCnt > *stop)
		{
			// Print Car Reward 
			std::cout << "####Car Reward####" << std::endl;
			for (uint32_t i=0; i<serviceN; i++)
			{
				std::cout << "Service " << i << ": " << carReward[i] << ", " << cc[i].GetAccuracy() << "%" <<  std::endl;
			}
			std::cout << "\n\n##################\n" << "Episod Stopn\n" << "##################" << std::endl;
			Simulator::Stop ();
		}
		evalCnt++;
	}
	
	double commTime = 1.0/30.0;
	Simulator::Schedule (Seconds (commTime), &Sink::Communication, this);
}
void Sink::PrintInfo ()
{
	for (uint32_t i=0; i<serviceN; i++)
	{
		printf("------- Serivec %d --------\n", i);
		printf("[sampleRate]::Observed\n");
		PrintState<uint32_t> (state[i].sampleRate, service_ssN[i]);
		printf("[sampleValue]::Observed\n");
		PrintState<double> (state[i].sampleValue, service_ssN[i]);
		printf("[sampleValue]::Truth\n");
		if (obsMod == "temp")
			PrintState<double> (oc[i].tempMap, service_ssN[i]);
		else if (obsMod == "track" || obsMod == "car")
			PrintState<double> (oc[i].trackMap, service_ssN[i]);
		printf("--------------------------\n");
	}
}

void Sink::PrintChannel ()
{
	printf("========== Channel Info (duration 1s) ==========\n");
	double now = Simulator::Now().GetSeconds ();
	printf("Simulator Time: %lf\n", now);

	if (totRecvCnt == 0)
	{
		std::cout << "totRecvCnt: 0" << std::endl;
		Simulator::Schedule (Seconds (1), &Sink::PrintChannel, this);
		return;
	}

	double thr = Bit2Mbps (Byte2Bit (totRecvByte));
	double avgDelay = (double) totDelay / totRecvCnt;
	
	std::cout << "totRecvByte: " << totRecvByte << std::endl;
	std::cout << "totRecvCnt: " << totRecvCnt << std::endl;
	std::cout << "AvgDelay (ms): " << avgDelay << std::endl;
	std::cout << "Throughput (Mbps): " << thr << std::endl;

	// Initialize
	totRecvByte = 0;
	totRecvCnt = 0;
	totDelay = 0;
	Simulator::Schedule (Seconds (1), &Sink::PrintChannel, this);
}


Callback<void, std::queue<DATA*>*, uint64_t> Sink::CallbackRecvSche (void)
{
	return MakeCallback (&Sink::RecvSche, this);
}

/****************
* Modification Below
****************/

void Sink::Evaluation (void)
{
	for(uint32_t i = 0; i<serviceN ; i++)
  {
		memcpy(trackMap[i], oc[i].trackMap, sizeof(double)*(oc[i].senN)); // Need Modi.. mkris 5/24
    //PrintState<double>(i, "Past", trackMap[i], oc[i].senN);
		cnt[i] += 1;
		reward_cnt[i] += 1;

		// Move Car	
		if (simMod == "car")
		{
			cc[i].Moving (&state[i]);
			Reward ();
		}
	}

  //move object as time flow
	if (obsMod == "temp")
	{
		TempAcc ();
		TempDiff ();
	}
	
	// Caculate Reward
	Reward ();
}


void Sink::TrackAcc (double** observed)
{
  int16_t ground = -1;
  int16_t observ = -1;
  for(uint32_t i = 0; i<serviceN ; i++)
  {
    for(uint32_t j = 0; j<service_ssN[i]; j++)
    {
      if(oc[i].trackMap[j]==1)
			{
        ground = j;
			}
			if(observed[i][j]==1)
      {
				observ = j;
			}
    }

    if(observ==ground)
    {
      trackAcc[i] = 1;
    }
    else
		{
      trackAcc[i] = 0;
    }
  }
}

void Sink::TempAcc (void)
{
  double correct = 0;
  double diff = 0;
  for(uint32_t i = 0; i<serviceN ; i++)
  {
    for(uint32_t j = 0; j<service_ssN[i]; j++)
    {
      diff = oc[i].tempMap[j] - state[i].sampleValue[j];
      if(diff<5 && diff>-5) correct +=1;
    }
                
    tempAcc[i] = correct/service_ssN[i];
    tempAcc_avg[i] += tempAcc[i];
    correct = 0;
  }
}

void Sink::TempDiff (void)
{
  double global = 0;
  double diff = 0;
  for(uint32_t i = 0; i<serviceN ; i++)
  {
    for(uint32_t j = 0; j<service_ssN[i]; j++)
    {
      diff = oc[i].tempMap[j] - state[i].sampleValue[j];
      if(diff<0) diff *= -1;
      global += diff;
    }
                
    tempDiff[i] = global/service_ssN[i];
    tempDiff_avg[i] +=tempDiff[i];
    global = 0;
  }
}

void Sink::Reward (void)
{
	// Reward Function for Temperature
	if (obsMod == "temp")
	{
		double tmpReward = 0;

		for(uint32_t i = 0; i<serviceN ; i++)
		{

			//PrintState<double>(i, "Temp Truth", oc[i].tempMap, service_ssN[i]);
			//PrintState<double>(i, "Temp Observed", state[i].sampleValue, service_ssN[i]);
		
			double global = 0;
			// Local Reward
			for(uint32_t j = 0; j<service_ssN[i]; j++)
			{
				double diff = oc[i].tempMap[j] - state[i].sampleValue[j];

				if(diff < 0) 
					diff *= -1;
				global += diff;
			
				if (diff <= 5)
					tmpReward += 1;
				else
					tmpReward -= 1;
			}
			//tmpReward = tmpReward - (service_ssN[i] - 25);
			tmpReward /= service_ssN[i];
		
			// Global Reward
			if (global >= threshold[i] * 0.1) // 10% -> 100  
				tmpReward -= 1;
			else if (global >= threshold[i] * 0.05) // 5% -> 50
				tmpReward -= 0.5;
			else if (global >= threshold[i] * 0.04) // 4% -> 40
				tmpReward += 0;
			else if (global >= threshold[i] * 0.03) // 3% -> 30
				tmpReward += 0.5;
			else																		// 2% -> 20 
				tmpReward += 1;

			reward[i] += tmpReward;
			reward_avg[i] += tmpReward;
			tmpReward = 0; 
		}
	}
	// Reward Function for Single Track
	else if (obsMod == "track")
	{
		for (uint32_t i=0; i<serviceN; i++)
		{
			double tmpReward = 1;
			for (uint32_t j=0; j<service_ssN[i]; j++)
			{
				if (oc[i].trackMap[j] != state[i].sampleValue[j])
				{
					tmpReward = 0;
					break;
				}
			}
			reward[i] += tmpReward;
			reward_avg[i] += tmpReward;

			if (tmpReward == 1)
			{
				singleAcc_avg[i] += 1;
			}
		}
	}
}

void Sink::PrintEval(void)
{
	for(uint32_t i = 0; i<serviceN ; i++)
	{
		std::cout<<"\nService["<<i<<"]"<<std::endl;
		std::cout<<"trackAcc : "<<trackAcc[i]<<std::endl;
		std::cout<<"tempAcc : "<<tempAcc[i]<<std::endl;
		std::cout<<"tempDiff : "<<tempDiff[i]<<std::endl;
		std::cout<<"reward : "<<reward[i]<<std::endl;
		printf("\n");
	}
}


void Sink::DAFU(void)
{
	double max = -1;
	double min = 100;
	int32_t max_loc = -1;
	int32_t min_loc = -1;
	double temp = 0;
	int32_t target_loc[2] = {-1,-1};
	uint32_t node_len = 0;
	bool flag = false;
	bool flag2 = false;

	for(uint32_t i = 0; i<serviceN ; i++)
	{
		node_len = sqrt(service_ssN[i]);

		for(uint32_t j = 0; j<service_ssN[i]; j++)
		{
			temp = state[i].sampleValue[j]-p_tempMap[i][j];
                        //std::cout<< state[i].sampleValue[j]<<" " <<p_tempMap[i][j]<<" "<<temp<<std::endl;

			if(temp>max || temp>3)
			{
				max = state[i].sampleValue[j];
				max_loc = j;
				flag = true;
			}
			else if(temp<min && temp<-3)
			{
				min = state[i].sampleValue[j];
				min_loc =j;
				flag2 = true;
			}



		}

		if(flag){
			if(flag2){
				target_loc[0] = 2*max_loc - min_loc;
				target_loc[1] = max_loc;
				if((max_loc%node_len==0 && min_loc%node_len>0) 
						|| (max_loc%node_len==(node_len-1) && min_loc%node_len<(node_len-1))
						|| (max_loc/node_len == 0 && min_loc/node_len>0)
						|| (max_loc/node_len == (node_len-1) && min_loc/node_len<(node_len-1))
						|| (target_loc[0]<0 || target_loc[0]>(int32_t)node_len))
					target_loc[0] = max_loc;
			}
			else{
				target_loc[0] = max_loc;

			}
		}
		DAFU_setAction(p_tempMap[serviceN], target_loc, i);
		for(uint32_t j = 0; j<service_ssN[i]; j++)
		{
			p_tempMap[i][j] =  state[i].sampleValue[j];
		}
	}
}

void Sink::DAFU_setAction(double* target, int32_t* location, uint32_t index)
{
	int32_t jump = 0;
	uint32_t nodeNum = service_ssN[index];
	uint32_t nodeLen = sqrt(nodeNum);

	for(uint32_t i = 0; i<nodeNum; i++)
	{
		target[i] = 0;

	}
	for(uint32_t i = 0; i<2; i++)
	{
		if(location[i]<0 || location[i]>=(int32_t)nodeNum)
			break;
		for(int32_t j = -2; j<3; j++)
		{
			for(int32_t k = -2; k<3; k++)
			{
				jump = (int32_t)nodeLen*k;
                                int16_t x = (int16_t)location[i]+j+jump;
				if(x>-1 && x<(int16_t)nodeNum)
				{
                                        int16_t y = ((int16_t)location[i]+jump)%nodeLen;
					if(y>-1 && y+j<(int16_t)nodeLen)
					{
						target[location[i]+j+jump] = 1;
					}
				}
			}       
		}
	}
        
	uint32_t targetNum = 0;
	double FPS_for_target = 0;
	double FPS_for_target_min = 0.003;
	for(uint32_t i = 0; i<nodeNum; i++)
	{
		if(target[i]==1)
			targetNum +=1;
	}
        //std::cout<<"\ntargetNum : "<<targetNum <<std::endl;
	if(targetNum >0)
		FPS_for_target = (1-((nodeNum-targetNum)*FPS_for_target_min))/targetNum;
	else
	{
		FPS_for_target = 1/(double)nodeNum;
		FPS_for_target_min = FPS_for_target;
	}
        
	for(uint32_t i = 0; i<nodeNum; i++)
	{
		if(target[i]==1)
			state[index].action[i] = (uint32_t)(avgRate*service_ssN[index]*FPS_for_target); //#### -> targetThr
		else
			state[index].action[i] = (uint32_t)(avgRate*service_ssN[index]*FPS_for_target_min);
	}
        /*
        printf("[action]::Observed\n");
        for(uint32_t i = 0; i<serviceN; i++)
            PrintState<uint32_t> (state[index].action,nodeNum);
        */
}


void Sink::ZMQCommunication ()
{
	evalCnt ++;
	
	if (evalCnt <= *stop &&  episodeEnd == false)
	{
		if (episodeStart)
			episodeStart = false;
                
		else 
		{
			// Send Reward
      double reward_sum = 0;
      for(uint32_t i = 0; i<serviceN; i++)
				reward_sum += reward[i]; 
      reward_sum = reward_sum/serviceN;
      std::string message = "{\"reward\":" + std::to_string (reward_sum) + "}";
			ZMQSendJson (zmqsocket, message);
		}

		// Send State 
    for(uint32_t i = 0; i<serviceN; i++)
    {
	    uint32_t ssN = sqrt(service_ssN[i]);
	    ZMQSendObs (zmqsocket, stateMod, &state[i], &oc[i], ssN);
    }
		// Get Action
		//double *actionTmp = new double[ssN * ssN];


    uint32_t ssN = sqrt(service_ssN[0]);
    double *actionTmp ;
    actionTmp = ZMQRecvAction (zmqsocket, serviceN*ssN * ssN);
    for(uint32_t j = 0; j<serviceN; j++)
    {
      for (uint32_t i=0; i<ssN * ssN; i++)
      { 
				state[j].action[i] = actionTmp[j*ssN*ssN+i] * (avgRate * ssN * ssN*serviceN-ssN*ssN*serviceN);
        if(state[j].action[i]==0)
            state[j].action[i] = 1;
      }

		}
		delete[] actionTmp;
    
		/*
    printf("[sampleAction]::Observed\n");
    for(uint32_t i = 0; i<serviceN; i++)
      PrintState<uint32_t> (state[i].action,ssN*ssN);
		*/

		// If Cart Mode, End is different
		if (simMod == "car") 
		{
			if (cc[0].carN == 0)
			{
				episodeEnd = true;
			}
		}
				
		// Send End
		if (evalCnt < *stop && episodeEnd == false)
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
    double reward_sum = 0;
    for(uint32_t i = 0; i<serviceN; i++)
			reward_sum += reward[i]; 
    std::string message = "{\"reward\":" + std::to_string (reward_sum) + "}";
    ZMQSendJson (zmqsocket, message);
    for(uint32_t i = 0; i<serviceN; i++)
    {
			// Send State
	    uint32_t ssN = sqrt(service_ssN[i]);
			ZMQSendObs (zmqsocket, stateMod, &state[i], &oc[i], ssN);
    }
	
		// Stop the Simlator
		std::cout << "##################" << std::endl;
		std::cout << "Episod Stop\n" << std::endl;
		std::cout << "##################" << std::endl;

		if (simMod == "car")
		{
			std::cout << "####Car Reward####" << std::endl;
			for (uint32_t i=0; i<serviceN; i++)
			{
				std::cout << "Service " << i << ": " << reward[i] << ", " << cc[i].GetAccuracy() << "%" <<  std::endl;
			}
		}
		Simulator::Stop ();
	}	
}

void ZMQSendJson (zmq::socket_t* zmqsocket, std::string message)
{
	zmq::message_t request (message.size());
	memcpy (request.data (), message.c_str (), message.size ());
	zmqsocket->send(request);

	zmq::message_t reply;
	zmqsocket->recv (&reply);
}

void ZMQSendObs (zmq::socket_t* zmqsocket, std::string stateMod, STATE* state, ObjectContain* oc, uint32_t ssN)
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
		last[ssN - y - 1][x] = state->lastUpdateTime[i].GetMilliSeconds ();
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
	if (stateMod == "last" || stateMod == "action")
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
	if (stateMod == "action")
	{
		message += "],[";
		for (uint32_t i=0; i<ssN; ++i)
		{
			message += "[";
			for (uint32_t j=0; j<ssN; ++j)
			{
				message += std::to_string(rate[i][j]);
				
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
		delete[] rate[i];
		delete[] ground[i];
	}
	delete[] obs;
	delete[] last;
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

}