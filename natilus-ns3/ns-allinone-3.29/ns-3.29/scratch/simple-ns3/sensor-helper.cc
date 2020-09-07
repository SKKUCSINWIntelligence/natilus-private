#include "ns3/inet-socket-address.h"
#include "ns3/packet-socket-address.h"
#include "ns3/string.h"
#include "ns3/names.h"
#include "ns3/uinteger.h"
#include "sensor-helper.h"

namespace ns3 {

SensorHelper::SensorHelper ()
{
	m_factory.SetTypeId ("ns3::SimpleSensor");
}
SensorHelper::SensorHelper (Address address, uint16_t port)
{
  m_factory.SetTypeId ("ns3::SimpleSensor");
  m_factory.Set ("RemoteAddress", AddressValue(address));
  m_factory.Set ("RemotePort", UintegerValue(port));
}

void
SensorHelper::SetAttribute (std::string name, const AttributeValue &value)
{
  m_factory.Set (name, value);
}

ApplicationContainer
SensorHelper::Install (Ptr<Node> node) const
{
  return ApplicationContainer (InstallPriv (node));
}

ApplicationContainer
SensorHelper::Install (NodeContainer c) const
{
  ApplicationContainer apps;
  for (NodeContainer::Iterator i = c.Begin (); i != c.End (); ++i)
    {
      apps.Add (InstallPriv (*i));
    }

  return apps;
}

Ptr<Application>
SensorHelper::InstallPriv (Ptr<Node> node) const
{
  Ptr<Application> app = m_factory.Create<Application> ();
  node->AddApplication (app);

  return app;
}

} // namespace ns3
