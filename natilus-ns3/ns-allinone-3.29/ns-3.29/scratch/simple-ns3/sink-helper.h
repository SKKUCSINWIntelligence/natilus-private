#ifndef SINK_HELPER_H
#define SINK_HELPER_H

#include "ns3/object-factory.h"
#include "ns3/ipv4-address.h"
#include "ns3/node-container.h"
#include "ns3/application-container.h"

namespace ns3 {

class SimpleSinkHelper
{
public:
	SimpleSinkHelper ();
	
	SimpleSinkHelper (uint16_t port);
	
	void SetAttribute (std::string name, const AttributeValue &value);
	
	ApplicationContainer Install (Ptr<Node> node) const;

	ApplicationContainer Install (NodeContainer c) const;

private:
	ObjectFactory m_factory;
	Ptr<Application> InstallPriv (Ptr<Node> node) const;
};

} // namespace ns3

#endif
