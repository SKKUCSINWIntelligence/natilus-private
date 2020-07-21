#include "ns3/log.h"
#include "opengym-interface.h"

#include <zmq.hpp> 

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("OpenGymInterface");

NS_OBJECT_ENSURE_REGISTERED (OpenGymInterface);

TypeId
OpenGymInterface::GetTypeId (void) {
	static TypeId tid = TypeId ("ns3::OpenGymInterface")
		.SetParent<Object> ()
		.SetGroupName ("OpenGym")
		.AddConstructor<OpenGymInterface> ()
	;
	
	return tid;
}

OpenGymInterface::OpenGymInterface () 
{	
}

OpenGymInterface::~OpenGymInterface () 
{
}

void 
OpenGymInterface::SetUp (uint32_t port) {
	std::cout << m_port << std::endl;
	m_port = port;
}

void
OpenGymInterface::SendJson (std::string message, std::string port) {
	zmq::context_t _context(1);
	zmq::socket_t _socket (_context, ZMQ_REQ);
	std::string addr = "tcp://localhost:" + port;
	_socket.connect (addr);

	// Send JSON to Python
	zmq::message_t request (message.size ());
	memcpy (request.data (), message.c_str (), message.size ());
	_socket.send (request);

	zmq::message_t reply;
	_socket.recv (&reply);

	_socket.close();
}

void
OpenGymInterface::SendObs(double** info, uint32_t sizei, uint32_t sizej) {
	zmq::context_t _context(1);
	zmq::socket_t _socket(_context,ZMQ_REQ);

	_socket.connect ("tcp://localhost:5050");
		
	std::string message = "{\"obs\":\"[";
	for (uint32_t i = 0; i < sizei ; i++) {
		message += "[";
		for (uint32_t j = 0; j < sizej; j++) {
			message += std::to_string(info[i][j]);
			if(j != sizej - 1) message += ",";
		}
		message += "]";
		if(i != sizei - 1) message += ",";
	}
	message += "]\"}";
	//std::cout << message << std::endl;
	zmq::message_t request (message.size());
	memcpy(request.data(), message.c_str(), message.size());
	_socket.send (request);
			
	zmq::message_t reply;
	_socket.recv (&reply);

	_socket.close();
}

double*
OpenGymInterface::RecvAction(uint32_t size) {
	zmq::context_t _context(1);
	zmq::socket_t _socket (_context, ZMQ_REQ);

	_socket.connect ("tcp://localhost:5050");
	
	double *actions = new double[size];
	
	for(uint32_t i = 0; i < size; i++) {
		zmq::message_t request(7);
		memcpy(request.data(), "Action", 7);
		_socket.send (request);

		zmq::message_t reply;
		_socket.recv (&reply);

		std::string action = std::string(static_cast<char*> (reply.data()), reply.size());
		double retval = atof (action.c_str());
		actions[i] = retval;
	}

	_socket.close();

	return actions;
}

void
OpenGymInterface::SendReward (double reward) {
	zmq::context_t _context(1);
	zmq::socket_t _socket (_context, ZMQ_REQ);

	_socket.connect ("tcp://localhost:5050");

	std::string message = "{\"reward\":" + std::to_string(reward) + "}";
	zmq::message_t request(message.size());
	memcpy (request.data (), message.c_str(), message.size());
	_socket.send (request);

	zmq::message_t reply;
	_socket.recv (&reply);

	_socket.close();
}

void
OpenGymInterface::SendEnd (uint8_t end) {
	zmq::context_t _context(1);
	zmq::socket_t _socket (_context, ZMQ_REQ);

	_socket.connect ("tcp://localhost:5050");

	zmq::message_t request(sizeof (end));
	memcpy (request.data (), &end, sizeof (end));
	_socket.send (request);

	zmq::message_t reply;
	_socket.recv (&reply);

	_socket.close();
}
}
