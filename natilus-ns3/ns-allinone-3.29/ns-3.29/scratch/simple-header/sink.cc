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
		delete[] topLoc;
		delete[] scoreMap;;

		for(uint32_t i = 0; i<serviceN ; i++)
		{
			delete[] trackMap[i];
		}
		delete[] trackMap;
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
		threshold = new double[serviceN];

		for(uint32_t i = 0; i<serviceN ; i++)
		{
			topLoc = new int32_t[service_ssN[i]];
			scoreMap = new double[service_ssN[i]];
			trackMap[i] = new double[service_ssN[i]];
		}

		for(uint32_t i = 0; i<service_ssN[i]; i++)
		{
			topLoc[i] = -1;
			scoreMap[i] = 0;
		}

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

			uint32_t k = 0;
			for (uint32_t j=0; j<ssN; j++)
			{
				if (obsMod == "car")
				{
					uint32_t x = j % oc[0].unitN;
					uint32_t y = j / oc[0].unitN;

					if (!(x==0 || y==0 || x==(oc[0].unitN-1) || y==(oc[0].unitN-1)))
					{
						DATA *data = new DATA;
						data->dataSize = actionPacketSize;
						data->genTime = Simulator::Now();
						data->serId = i;
						data->cellId = j;
						data->action = state[i].action[k];

						txQ.push (data);

						k += 1;
					}
				}
				else
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
			recvCnt += 1;

			uint32_t ran = rand() % 100;
			if (firstEval)
				ran = 0;
			if(ran < (100-errorRate))
			{
				// Get Data Info
				uint32_t serId = data->serId;
				uint32_t cellId = data->cellId;

				// Measurement
				uint64_t delay = now - (data->genTime).GetMilliSeconds ();
				totRecvByte += data->dataSize;
				totRecvCnt += 1;
				totDelay += delay;
				double pastValue = state[serId].sampleValue[cellId];

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

				if (obsMod == "multi")
				{
					// Save car <-> cell
					for (uint32_t i=0; i<oc[serId].objectMax; i++)
					{	
						if (state[serId].sampleCar[i] == (int)cellId)
							state[serId].sampleCar[i] = -1;
						if (data->carCell[i] == true)
							state[serId].sampleCar[i] = (int)cellId;
					}				

					// Clear the map
					for (uint32_t i=0; i<service_ssN[0]; i++)
					{
						state[serId].sampleValue[i] = 0;
					}

					// Fill in the map
					for (uint32_t i=0; i<oc[serId].objectMax; i++)
					{
						int cell = state[serId].sampleCar[i];
						if (cell >= 0)
							state[serId].sampleValue[cell] += 1;
					}
				}
				else if (obsMod == "carx")
				{
					// Save car <-> cell
					for (uint32_t i=0; i<oc[serId].objectMax; i++)
						if (data->carCell[i] == true)
							state[serId].sampleCar[i] = (int)cellId;

					// Clear the map
					for (uint32_t i=0; i<service_ssN[0]; i++)
						state[serId].sampleValue[i] = 0;

					// Fill in the map
					for (uint32_t i=0; i<oc[serId].objectMax; i++)
					{
						int cell = state[serId].sampleCar[i];
						if (cell >= 0)
							state[serId].sampleValue[cell] += 1;
					}
				}
				else
					state[serId].sampleValue[cellId] = data->sampleValue;

				double curValue = state[serId].sampleValue[cellId];
				if (pastValue != curValue)
					state[serId].stateChangeTime[cellId] = Simulator::Now();
				state[serId].upInter[cellId] = delay;
				state[serId].lastUpdateTime[cellId] = Simulator::Now (); 
			}
			else
			{
				dropCnt += 1;
			}
			if (obsMod == "multi" || obsMod == "car")
				delete[] data->carCell;
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

		if (obsMod=="car")
		{ 
			// Count the Car Clusters
			uint32_t count = 0;
			uint32_t ssN = sqrt(service_ssN[0]); 
			for (uint32_t i=0; i<ssN; i++)
			{
				for (uint32_t j=0; j<ssN; j++)
				{
					uint32_t cell = j*ssN + i;
					if (oc[0].tempMap[cell] == 100)
					{
						count += 1;
						multi_cnt += 1;
					}
				}
			}
		}

		/* Check New Multi */
		/*if (obsMod == "multi")
			{
			for (uint32_t i=0; i<(oc->unitN/2)-1; i++)
			{
			uint32_t tag = oc->tag[i];
			if (tag != 0)
			{
			if (state[0].sampleValue[tag] == 0)
			state[0].sampleValue[tag] = -1;

			uint32_t xid = tag % (oc->unitN);
			uint32_t yid = tag / (oc->unitN);

			uint32_t cell[3] = {0};
			cell[0] = (yid+1)*(oc->unitN) + xid;
			cell[1] = (yid+1)*(oc->unitN) + (xid+1);
			cell[2] = yid*(oc->unitN) + (xid+1);

			for (uint32_t j=0; j<3; j++)
			if (state[0].sampleValue[cell[j]] == 0)
			state[0].sampleValue[cell[j]] = -1;
			}
			oc->tag[i] = 0;
			}
			}*/

		/* Print Before Comm */
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

		/* Clear New Multi: Does not affect DAFU */
		/*if (obsMod == "multi")
			{
			for (uint32_t i=0; i<service_ssN[0]; i++)
			{
			if (state[0].sampleValue[i] == -1)
			state[0].sampleValue[i] = 0;
			}
			}*/

		if (upMod == "DAFU")
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
				if (obsMod == "car")
				{
					std::cout << "####Car Reward####" << std::endl;
					for (uint32_t i=0; i<serviceN; i++)
					{
						std::cout << "Service " << i << ": " << carReward[i] << ", " << cc[i].GetAccuracy() << "%" <<  std::endl;
					}	
					std::cout << "\n\n##################\n" << "Episod Stopn\n" << "##################" << std::endl;
				}
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
			printf("------- Serivec %d -------- ", i);
			std::cout << "at " << Simulator::Now().GetSeconds() << std::endl;
			printf("[sampleRate]::Observed\n");
			PrintState<uint32_t> (state[i].sampleRate, service_ssN[i]);
			printf("[sampleValue]::Observed\n");
			PrintState<double> (state[i].sampleValue, service_ssN[i]);
			printf("[sampleValue]::Truth\n");
			if (obsMod == "temp")
				PrintState<double> (oc[i].tempMap, service_ssN[i]);
			else if (obsMod == "track" || obsMod == "car" || obsMod == "multi")
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
		// PrintInfo ();
		if (obsMod == "temp") //(0729)
		{
			/*
				 double tmpReward = 0;

				 for(uint32_t i = 0; i<serviceN ; i++)
				 {
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
			*/
			double Mx=0; //truth average
			double My=0; //observed average
			double Vx=0; //truth variance
			double Vy=0; //observed variance
			double Cxy=0; //correlation of truth, observed
			double C=1e-50;

			double l = 0;
			double c = 0;
			double s = 0;

			uint32_t ssN = sqrt(service_ssN[0]); 
			uint32_t sssN = ssN * ssN;

			//modification needed
			for(uint32_t j = 0; j<service_ssN[0]; j++)
			{
				Mx += oc[0].tempMap[j];
				My += state[0].sampleValue[j];
			}
			Mx /= sssN;
			My /= sssN;

			for(uint32_t j = 0; j<service_ssN[0]; j++)
			{			
				Vx += (oc[0].tempMap[j]-Mx)*(oc[0].tempMap[j]-Mx);
				Vy += (state[0].sampleValue[j]-My)*(state[0].sampleValue[j]-My);
				Cxy += (oc[0].tempMap[j]-Mx)*(state[0].sampleValue[j]-My);
			}

			Vx /= sssN;
			Vy /= sssN;
			Cxy /= (sssN-1);

			l = (2*Mx*My+C) / (Mx*Mx+My*My+C);
			c = (2*sqrt(Vx)*sqrt(Vy)+C) / (Vx+Vy+C);
			s = (Cxy+C) / (sqrt(Vx*Vy)+C);

			reward[0] += l*c*s;
			reward_avg[0] += l*c*s;

			//std::cout << "Reward: " << l*c*s << std::endl; 
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
		else if (obsMod == "car") 
		{

			// 1. MSE Distance Measure
			/*
				 uint32_t ssN = sqrt(service_ssN[0]); 
				 double dist = 0;

				 for (uint32_t i=0; i<service_ssN[0]; i++)
				 {
				 uint32_t x = i % ssN;
				 uint32_t y = i / ssN;	

				 if (!(x==0 || y==0 || x==(ssN-1) || y ==(ssN-1)))
				 {
				 double tmpDist = (oc[0].trackMap[i] - state[0].sampleValue[i]) * (oc[0].trackMap[i] - state[0].sampleValue[i]);
				 dist += tmpDist;
				 }
				 }


				 if (dist == 0)
				 {
				 reward[0] += 1;
				 reward_avg[0] += 1;
			//std::cout << "d " << dist << "  r " << 1 <<  std::endl;
			}
			else
			{
			reward[0] += 1/sqrt(dist);
			reward_avg[0] += 1/sqrt(dist);
			//std::cout << "d " << dist << "  r " << 1/ sqrt(dist)<<  std::endl;
			}
			*/

			// 2. SSIM Loss Measure
			double Mx=0; //truth average
			double My=0; //observed average
			double Vx=0; //truth variance
			double Vy=0; //observed variance
			double Cxy=0; //correlation of truth, observed
			double C1 = 0.01; //1e-50;
			double C2 = 0.03;
			double C3 = C2/2;

			double l = 0;
			double c = 0;
			double s = 0;

			uint32_t ssN = sqrt(service_ssN[0]); 
			uint32_t sssN = ssN * ssN;

			//modification needed
			for(uint32_t j = 0; j<service_ssN[0]; j++)
			{
				uint32_t x = j % ssN;
				uint32_t y = j / ssN;	

				if (!(x==0 || y==0 || x==(ssN-1) || y ==(ssN-1)))
				{
					Mx += oc[0].trackMap[j];
					My += state[0].sampleValue[j];
				}
			}
			Mx /= sssN;
			My /= sssN;

			for(uint32_t j = 0; j<service_ssN[0]; j++)
			{
				uint32_t x = j % ssN;
				uint32_t y = j / ssN;	

				if (!(x==0 || y==0 || x==(ssN-1) || y ==(ssN-1)))
				{
					Vx += (oc[0].trackMap[j]-Mx)*(oc[0].trackMap[j]-Mx);
					Vy += (state[0].sampleValue[j]-My)*(state[0].sampleValue[j]-My);
					Cxy += (oc[0].trackMap[j]-Mx)*(state[0].sampleValue[j]-My);
				}
			}

			Vx /= sssN;
			Vy /= sssN;
			Cxy /= (sssN-1);

			l = (2*Mx*My+C1) / (Mx*Mx+My*My+C1);
			c = (2*sqrt(Vx)*sqrt(Vy)+C2) / (Vx+Vy+C2);
			s = (Cxy+C3) / (sqrt(Vx*Vy)+C3);

			reward[0] += l*c*s;
			reward_avg[0] += l*c*s;
		}
		else if (obsMod == "multi")
		{
			double *truth = new double[ssN*ssN];
			double *obsrv = new double[ssN*ssN];
			for (uint32_t i=0; i<ssN*ssN; i++)
			{
				truth[i] = 0;
				obsrv[i] = 0;
			}

			//double sumOc = 0;
			//double sumTrack = 0;
			//double maxA = 0;
			//double maxB = 0;
			//for (uint32_t i=0; i<service_ssN[0]; i++)
			//{
				//if (oc[0].trackMap[i] > maxA)
				//	maxA = oc[0].trackMap[i];
				//if (state[0].sampleValue[i] > maxB)
				//		maxB = state[0].sampleValue[i];
				//sumOc += oc[0].trackMap[i];
				//sumTrack += state[0].sampleValue[i];
				//if (oc[0].trackMap[i] > 0)
				//	sumOc += 1;
				//if (state[0].sampleValue[i] > 0)
				//	sumTrack += 1;
			//}
			for (uint32_t i=0; i<service_ssN[0]; i++)
			{
				/*if (sumOc == 0)
					oc[0].trackMap[i] = 0;
				else
					oc[0].trackMap[i] /= sumOc;
				if (sumOc == 0)
					state[0].sampleValue[i] = 0;
				else
					state[0].sampleValue[i] /= sumOc;
				if (maxA > 0)
					oc[0].trackMap[i] /= maxA;
				if (maxB > 0)
					state[0].sampleValue[i] /= maxB;*/

				/*if (oc[0].trackMap[i] >= 15)
					truth[i] = oc[0].trackMap[i] / 3;
				else if (oc[0].trackMap[i] >= 10)
					truth[i] = oc[0].trackMap[i] / 2;
				else
					truth[i] = oc[0].trackMap[i];

				if (state[0].sampleValue[i] >= 15)
					obsrv[i] = state[0].sampleValue[i] / 3;
				else if (state[0].sampleValue[i] >= 10)
					obsrv[i] = state[0].sampleValue[i] / 2;
				else 
					obsrv[i] = state[0].sampleValue[i];*/
				truth[i] = oc[0].trackMap[i];
				obsrv[i] = state[0].sampleValue[i];
			}
			//PrintInfo ();
			double Mx=0; //truth average
			double My=0; //observed average
			double Vx=0; //truth variance
			double Vy=0; //observed variance
			double Cxy=0; //correlation of truth, observed
			double C1 = 0.001; //1e-50;
			double C2 = 0.003;
			double C3 = C2/2;

			double l = 0;
			double c = 0;
			double s = 0;

			uint32_t sssN = ssN * ssN;
			double tmpMultiCnt = 0;

			//modification needed
			for(uint32_t i = 0; i<service_ssN[0]; i++)
			{
				Mx += truth[i];
				My += obsrv[i];
				
				// Count Multi Objects
				if (truth[i] != 0)
				{
					tmpMultiCnt += 1;
				}
			}
			tmpMultiCnt /= service_ssN[0];
			multiCnt += tmpMultiCnt;
			if (tmpMultiCnt > multiMax)
			{
				multiMax = tmpMultiCnt;
			}
			Mx /= sssN;
			My /= sssN;

			for(uint32_t i = 0; i<service_ssN[0]; i++)
			{
				Vx += (truth[i]-Mx)*(truth[i]-Mx);
				Vy += (obsrv[i]-My)*(obsrv[i]-My);
				Cxy += (truth[i]-Mx)*(obsrv[i]-My);
			}

			Vx /= sssN;
			Vy /= sssN;
			Cxy /= (sssN-1);

			l = (2*Mx*My+C1) / (Mx*Mx+My*My+C1);				// Average
			c = (2*sqrt(Vx)*sqrt(Vy)+C2) / (Vx+Vy+C2);	// Deviation
			s = (Cxy+C3) / (sqrt(Vx)*sqrt(Vy)+C3);						// Corelation
	
			eMa += Mx;
			eMb += My;
			eVa += Vx;
			eVb += Vy;
			if (Mx-My >= 0)
				cMa = cMa + (Mx-My);
			else
				cMb = cMb + (My-Mx);

			if (Vx-Vy >= 0)
				cVa = cVa + (Vx-Vy);
			else
				cVb = cVb + (Vy-Vx);
			

			eL += l;
			eC += c;
			eS += s;
			
			reward[0] += l*c*s;
			reward_avg[0] += l*c*s;
			//std::cout << "Reward: " << l*c*s << std::endl;
			/*for (uint32_t i=0; i<service_ssN[0]; i++)
			{
				oc[0].trackMap[i] *= maxA;
				state[0].sampleValue[i] *= maxB;
			}*/

			delete[] truth;
			delete[] obsrv;
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
		//uint32_t len = topK;
		DAFUSetScore(state[0].sampleValue);
		DAFUTopK(scoreMap, topK);
		DAFUSetAction(topLoc, topK);

	}

	void Sink::DAFUSetScore(double *map)
	{
		if (scoreFtn == "map")
		{
			for(uint32_t i=0; i<service_ssN[0]; i++)
			{
				scoreMap[i] = map[i];
			}
		}
		else if (scoreFtn == "around")
		{
			topK = 0;
			for (uint32_t i=0; i<service_ssN[0]; i++)
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
			PrintState<double> (oc[0].trackMap, service_ssN[0]);
			printf("[[Sample Value]]\n");
			PrintState<double> (state[0].sampleValue, service_ssN[0]);
			printf("[[Score Value]]\n");
			PrintState<double> (scoreMap, service_ssN[0]);
		}
	}

	void Sink::DAFUTopK(double * map, uint32_t len)
	{
		double min = 0;
		int32_t min_loc = 0;
		double temp = 0;  
		double temp2 = 0;
		double score_min = 0.001;
		
		for(uint32_t i = 0; i<serviceN; i++)
		{
			for(uint32_t j = 0; j<service_ssN[i]; j++)
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

	void Sink::DAFUSetAction(int32_t* scoreMap,int32_t len)
	{
		uint32_t nodeNum = service_ssN[0];
		uint32_t nodeLen = sqrt(nodeNum);
		int32_t* target = new int32_t[service_ssN[0]];
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
			FPS_for_target = (avgRate*service_ssN[0]-(nodeNum-targetNum)*FPS_for_target_min)/targetNum;
		else
		{
			FPS_for_target = avgRate*service_ssN[0]/(double)nodeNum+1;
			FPS_for_target_min = FPS_for_target-1;
		}

		for(uint32_t i = 0; i<nodeNum; i++)
		{
			if(target[i]==2)
				state[0].action[i] = (uint32_t)FPS_for_target; //#### -> targetThr
			else
				state[0].action[i] = (uint32_t)FPS_for_target_min;
		}

		if (dafuInfo)
		{
			printf("\n[[Action Set]]\n");
			PrintState<uint32_t> (state[0].action, service_ssN[0]);
		}
		for(uint32_t i = 0; i<ssN; i++)
			topLoc[i] = -1;

		delete[] target;
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
				ZMQSendObs (zmqsocket, stateMod, &state[i], &oc[i], ssN, obsMod);
			}
			// Get Action
			//double *actionTmp = new double[ssN * ssN];


			uint32_t ssN = sqrt(service_ssN[0]);
			double *actionTmp ;
			if (obsMod == "car")
				actionTmp = ZMQRecvAction (zmqsocket, serviceN*(ssN-2)*(ssN-2));
			else
				actionTmp = ZMQRecvAction (zmqsocket, serviceN*ssN*ssN);

			for(uint32_t j = 0; j<serviceN; j++)
			{
				if (obsMod == "car")
				{
					for (uint32_t i=0; i<(ssN-2)*(ssN-2); i++) 
					{
						//uint32_t x = i % ssN;
						//uint32_t y = i / ssN;

						//if (x==0 || y==0 || x==ssN-1 || y==ssN-1)
						//	state[j].action[i] = 30;
						//else
						state[j].action[i] = actionTmp[i] * (avgRate * (ssN-2) * (ssN-2));

						if (state[j].action[i] == 0)
							state[j].action[i] = 1;

					}
				}
				else
				{
					for (uint32_t i=0; i<ssN * ssN; i++)
					{ 
						// 0819 set offset as 10FPS XXX
						state[j].action[i] = actionTmp[j*ssN*ssN+i] * (avgRate*ssN*ssN*serviceN-ssN*ssN*serviceN); 
						if(state[j].action[i]==0)
							state[j].action[i] = 1;
					}
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
				ZMQSendObs (zmqsocket, stateMod, &state[i], &oc[i], ssN, obsMod);
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

			if (obsMod == "car")
			{
				if(x==0 || y==0 || x==(oc->unitN-1) || y==(oc->unitN-1))
				{	
					obs[ssN-y-1][x] = oc->trackMap[i];
					last[ssN-y-1][x] = 0; //Simulator::Now().GetMilliSeconds();
				}
				else
				{
					obs[ssN-y-1][x] = state->sampleValue[i];
					last[ssN-y-1][x] = Simulator::Now().GetMilliSeconds() - state->lastUpdateTime[i].GetMilliSeconds();
					change[ssN-y-1][x] = Simulator::Now().GetMilliSeconds() - state->stateChangeTime[i].GetMilliSeconds();
				}
			}
			else
			{
				obs[ssN - y - 1][x] = state->sampleValue[i];
				last[ssN - y - 1][x] = Simulator::Now().GetMilliSeconds() - state->lastUpdateTime[i].GetMilliSeconds ();
				change[ssN-y-1][x] = Simulator::Now().GetMilliSeconds() - state->stateChangeTime[i].GetMilliSeconds();
			}
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

}
