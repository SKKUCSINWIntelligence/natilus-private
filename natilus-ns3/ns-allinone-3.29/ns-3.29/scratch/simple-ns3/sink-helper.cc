#include "ns3/string.h"
#include "ns3/inet-socket-address.h"
#include "ns3/names.h"
#include "ns3/uinteger.h"
#include "sink-helper.h"

namespace ns3 {

SimpleSinkHelper::SimpleSinkHelper ()
{
  m_factory.SetTypeId ("ns3::SimpleSink");
}

SimpleSinkHelper::SimpleSinkHelper (uint16_t port)
{
  m_factory.SetTypeId ("ns3::SimpleSink");
	SetAttribute ("Port", UintegerValue (port));
}

void 
SimpleSinkHelper::SetAttribute (std::string name, const AttributeValue &value)
{
  m_factory.Set (name, value);
}

ApplicationContainer
SimpleSinkHelper::Install (Ptr<Node> node) const
{
  return ApplicationContainer (InstallPriv (node));
}

ApplicationContainer
SimpleSinkHelper::Install (NodeContainer c) const
{
  ApplicationContainer apps;
  for (NodeContainer::Iterator i = c.Begin (); i != c.End (); ++i)
  {
    apps.Add (InstallPriv (*i));
  }

  return apps;
}

Ptr<Application>
SimpleSinkHelper::InstallPriv (Ptr<Node> node) const
{
  Ptr<Application> app = m_factory.Create<Application> ();
  node->AddApplication (app);

  return app;
}

} // namespace ns3
