#ifndef SINK_H
#define SINK_H

#include "function.h"
#include "t-object.h"
#include "car.h"
#include "struct.h"

#include <zmq.hpp>

namespace ns3{

class Sink
{
public:
	std::string log;
	std::string upMod;
	std::string obsMod;
	std::string simMod;
	std::string stateMod;

	// LOG Setting
	bool channelInfo;
	bool stateInfo;
	bool evalInfo;

	// Object Linking
	ObjectContain *oc;
	
	// Car Linking
	CarContain *cc;
	
	// Car Evaluation
	double *carReward;

	// Setting
	uint32_t objectN;
	uint32_t serviceN;
	uint32_t *service_ssN;
	bool haveSendAction = false;
	uint32_t actionPacketSize;

	// Measurement per 1s 
	uint64_t totRecvByte = 0;
	uint64_t totRecvCnt = 0;
	uint64_t totDelay = 0; // MilliSecond

	// State
	STATE *state;
	std::queue<STATE> *history;
	uint32_t historyN = 4;
	double **truth;

	// Link Schedule Info
	bool *isLinkScheWork;
  uint64_t avgRate;

	// Eval
	bool firstEval = true;
	uint64_t evalCnt = 0;
	uint64_t *stop;
	uint32_t sP = 30; // 30 ~ 60 ms
	uint32_t eP = 60 - sP;

	// TxQ
	std::queue<DATA*> txQ;

	// Callback
	Callback<void> LinkCheck;

	// Function
	Sink ();
	~Sink ();
	void Start (void);
	void Send (void);
	void Recv (std::queue<DATA*> *dataContain);
	void RecvSche (std::queue<DATA*> *dataContain, uint64_t micro_delay);

	void Eval ();
	void Communication ();
	void Evaluation ();
	void PrintInfo ();
	void PrintChannel ();

	// Callback Function
	Callback<void, std::queue<DATA*>*, uint64_t> CallbackRecvSche (void);

	/****************
	* Modification Below
	****************/

	// Eval Parameter (Evaluation)
	double *trackAcc;
	double *tempAcc;
	double *tempDiff;
	double *reward;
	double *cnt;
	double *threshold;
  double** trackMap;
	double *tempAcc_avg;
	double *tempDiff_avg;
	double *reward_avg;
	double *reward_cnt;
	double *singleAcc_avg;
	

	// Function
	void TrackAcc (double**);
	void TempAcc (void);
	void TempDiff (void);
	void Reward (void);
  void PrintEval(void);

  //DAFU
  double** p_tempMap;
  void DAFU (void);
	void DAFU_setAction (double*, int32_t*, uint32_t);

	// OpenGym
	bool episodeStart;
	bool episodeEnd;
	zmq::socket_t *zmqsocket;
	void ZMQCommunication ();
};

void ZMQSendJson	(zmq::socket_t*, std::string);
void ZMQSendObs		(zmq::socket_t*, std::string, STATE*, ObjectContain*, uint32_t, std::string); 
void ZMQSendEnd		(zmq::socket_t*, uint8_t);
double* ZMQRecvAction (zmq::socket_t*, uint32_t);

}

#endif
