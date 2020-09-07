/* Copy Right by mkris*/
#ifndef	SINK_H
#define SINK_H

#include "ns3/applications-module.h"
#include "ns3/event-id.h"
#include "ns3/ptr.h"
#include "ns3/traced-callback.h"
#include "ns3/address.h"

#include "t-object.h"
#include "struct.h" 

#include <zmq.hpp>

namespace ns3 {

class Address;
class Socket;
class Packet;

class SimpleSink : public Application 
{
public:
  static TypeId GetTypeId (void);
  SimpleSink ();

  virtual ~SimpleSink ();
  uint64_t GetTotalRx () const;
	void Set (void);

	/* Mode Setting */
	std::string		log; 
	std::string		obsMod;
	std::string		upMod;
	std::string		simMod;
	std::string		stateMod;

	/* Log Setting */
	bool					channelInfo;
	bool					stateInfo;
	bool					evalInfo;
	
	/* State */ 
	STATE *state;

	/* Object Setting */
	ObjectContain* oc;
	uint32_t			objectN;
	uint32_t			serviceN;
	
	/* Sensor Setting */
	uint32_t			ssN;
	Address*			addressList;
	uint32_t			avgRate;

	/* Sink Setting */
	uint64_t*			recvBytes;
	double				endTime;

	/* RL Setting */
	uint32_t			maxStep;
	bool					firstEval = true;
	uint32_t			evalCnt = 0;
	zmq::socket_t		*zmqsocket;
	bool					episodeStart = true;
	bool					episodeEnd = false;

	/* Eval Setting */
	double				reward = 0;
	double				rewardAvg = 0;
	double				cnt = 0; 
	double				cntAvg = 0;
	double				multiCnt = 0;
	double				multiMax = 0;
	
protected:
  virtual void DoDispose (void);

private:
  // inherited from Application base class.
  virtual void StartApplication (void);    // Called at time specified by Start
  virtual void StopApplication (void);     // Called at time specified by Stop
	
	void HandleRead (Ptr<Socket> socket);
	void Eval (void); 
	void Comm (void);
	void ZMQComm (void);
	void SendData (void);

	void PrintInfo (void);

	// Variables
	Ptr<Socket>     m_socket;       //!< Listening socket
	uint16_t				m_port;

	Address         m_local;        //!< Local address to bind to
	uint64_t        m_totalRx;      //!< Total bytes received

	/* Sensor Socket Setting */
	Address				*m_sensorAddrList;
};

void ZMQSendJson	(zmq::socket_t*, std::string);
void ZMQSendObs		(zmq::socket_t*, std::string, STATE*, ObjectContain*, uint32_t, std::string); 
void ZMQSendEnd		(zmq::socket_t*, uint8_t);
double* ZMQRecvAction (zmq::socket_t*, uint32_t);
} // namespace ns3

#endif

