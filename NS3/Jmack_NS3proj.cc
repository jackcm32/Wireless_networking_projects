/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
// This code is a modified version of "third.cc" from the tutorials directory of 
// the NS3 libary. 
// Packet loss counter code modified version from examples/wireless/wifi-hidden-terminal.cc

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/applications-module.h"
#include "ns3/wifi-module.h"
#include "ns3/mobility-module.h"
#include "ns3/internet-module.h"
#include "ns3/flow-monitor-module.h"
#include "ns3/ipv4-global-routing-helper.h" 
#include "ns3/random-variable-stream.h"
#include "ns3/config-store.h"
#include <ns3/lte-module.h>
#include "ns3/buildings-helper.h"
#include <ns3/buildings-module.h>
#include <ns3/mobility-building-info.h>
#include <ns3/buildings-propagation-loss-model.h>
#include <ns3/building.h>


using namespace ns3;

static void
SetPosition (Ptr<Node> node, Vector position)
{
  Ptr<ConstantPositionMobilityModel> mobility = node->GetObject<ConstantPositionMobilityModel> ();
  mobility->SetPosition (position);
}

NS_LOG_COMPONENT_DEFINE ("JackMackWifi");

int 
main (int argc, char *argv[])
{
  uint32_t nWifi = 1;
  double StartTime = 0.0;
  double StopTime = 10.;

  // Default config settings
  uint32_t payloadSize = 1472; 
  uint32_t maxPacket = 10000; 
  StringValue DataRate;
  DataRate = StringValue("DsssRate11Mbps"); //Max rate for 802.11b

  // Default building topology
  double x_min = -10.0;
  double x_max = 10.0;
  double y_min = -20.0;
  double y_max = 20.0;
  double z_min = 0.0;
  double z_max = 10.0;
  double nfloors = 1.0;
  double x_rooms = 1.0;
  double y_rooms = 1.0;
  double x_ap = 0.0;
  double y_ap = 0.0;
  double z_ap = 0.0;


  // Create randomness based on time 
  time_t timex;
  time(&timex); 
  RngSeedManager::SetSeed(timex); 
  RngSeedManager::SetRun(1);

  CommandLine cmd;
  // building dimentions
  cmd.AddValue("x_min", "Building min x dimention", x_min);
  cmd.AddValue("x_max", "Building max x dimention", x_max);
  cmd.AddValue("y_min", "Building min y dimention", y_min);
  cmd.AddValue("y_max", "Building min y dimention", y_max);
  cmd.AddValue("z_min", "Building min z dimention", z_min);
  cmd.AddValue("z_max", "Building min z dimention", z_max);
  cmd.AddValue("nfloors", "Building number of floors", nfloors);
  cmd.AddValue("x_rooms", "Building number of x direction rooms", x_rooms);
  cmd.AddValue("y_rooms", "Building number of x direction rooms", y_rooms);
  cmd.AddValue("x_ap", "Access point x position", x_ap);
  cmd.AddValue("y_ap", "Access point y position", y_ap);
  cmd.AddValue("z_ap", "Access point z position", z_ap);
  cmd.Parse (argc, argv);

  // Create and access point
  NodeContainer wifiApNode;
  wifiApNode.Create (1);

  // Create some station wifi nodes
  NodeContainer wifiStaNodes;
  wifiStaNodes.Create (nWifi);

  // Create the interconnection channel between the wifi nodes with propigtion delay
  YansWifiPhyHelper phy = YansWifiPhyHelper::Default ();
 
  YansWifiChannelHelper channel;
  channel.SetPropagationDelay ("ns3::ConstantSpeedPropagationDelayModel");
  channel.AddPropagationLoss ("ns3::HybridBuildingsPropagationLossModel");
  phy.SetChannel (channel.Create ());

  // MAC layer setup
  WifiHelper wifi = WifiHelper::Default ();
  wifi.SetStandard (WIFI_PHY_STANDARD_80211b);
  wifi.SetRemoteStationManager ("ns3::AarfWifiManager");

  NqosWifiMacHelper mac = NqosWifiMacHelper::Default ();

  // SSID infastructure. No active probing
  Ssid ssid = Ssid ("ns-3-ssid");
  mac.SetType ("ns3::StaWifiMac",
               "Ssid", SsidValue (ssid),
               "ActiveProbing", BooleanValue (false));

  // Create wifi devices 
  NetDeviceContainer staDevices;
  staDevices = wifi.Install (phy, mac, wifiStaNodes);

  // Configure access point
  mac.SetType ("ns3::ApWifiMac",
               "Ssid", SsidValue (ssid));

  NetDeviceContainer apDevices;
  apDevices = wifi.Install (phy, mac, wifiApNode);

  // Create building 
  Ptr<Building> b = CreateObject <Building> ();
  b->SetBoundaries (Box (x_min, x_max, y_min, y_max, z_min, z_max));
  b->SetBuildingType (Building::Residential);
  b->SetExtWallsType (Building::ConcreteWithWindows);
  b->SetNFloors (nfloors);
  b->SetNRoomsX (x_rooms);
  b->SetNRoomsY (y_rooms);

  // Mobility *****change me later******
  MobilityHelper mobility;

  mobility.SetMobilityModel ("ns3::RandomDirection2dMobilityModel",
                               "Bounds", RectangleValue (Rectangle (-2, 2, -2, 2)),
                               "Speed", StringValue ("ns3::ConstantRandomVariable[Constant=2]"),
                               "Pause", StringValue ("ns3::ConstantRandomVariable[Constant=0.2]"));

  mobility.Install (wifiStaNodes);
  BuildingsHelper::Install (wifiStaNodes);

  // Access point to stay staionary at position (10,0)
  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobility.Install (wifiApNode);
  SetPosition(wifiApNode.Get(0), Vector (x_ap, y_ap, z_ap));
  BuildingsHelper::Install (wifiApNode);

  BuildingsHelper::MakeMobilityModelConsistent();

  // Install internet stacks
  InternetStackHelper stack;
  stack.Install (wifiApNode);
  stack.Install (wifiStaNodes);

  // Assign addresses to device interfaces
  Ipv4AddressHelper address;
  Ipv4Address addr;
  address.SetBase ("10.1.1.0", "255.255.255.0"); 
  Ipv4InterfaceContainer staNodesInterface; 
  Ipv4InterfaceContainer apNodeInterface; 
  staNodesInterface = address.Assign (staDevices); 
  apNodeInterface = address.Assign (apDevices);

  // Create traffic generator (UDP)
  ApplicationContainer serverApp;
  UdpServerHelper myServer (4001); //port 4001
  serverApp = myServer.Install (wifiStaNodes.Get (0));
  serverApp.Start (Seconds(StartTime));
  serverApp.Stop (Seconds(StopTime));
  UdpClientHelper myClient (apNodeInterface.GetAddress (0), 4001); //port 4001 
  myClient.SetAttribute ("MaxPackets", UintegerValue (maxPacket)); myClient.SetAttribute ("Interval", TimeValue (Time ("0.002"))); //packets/s 
  myClient.SetAttribute ("PacketSize", UintegerValue (payloadSize)); 
  ApplicationContainer clientApp = myClient.Install (wifiStaNodes.Get (0)); 
  clientApp.Start (Seconds(StartTime));


  // Enable internetwork routing 
  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

  // Calculate Throughput & Delay using Flowmonitor  
  FlowMonitorHelper flowmon;
  Ptr<FlowMonitor> monitor = flowmon.InstallAll();

  //Need to run the simulation for +20 seconds because packets are only considered missing after 10 seconds
  Simulator::Stop (Seconds (StopTime+20));


  Simulator::Run ();
  
  monitor->CheckForLostPackets ();
  Ptr<Ipv4FlowClassifier> classifier = DynamicCast<Ipv4FlowClassifier> (flowmon.GetClassifier ()); 
  std::map<FlowId, FlowMonitor::FlowStats> stats = monitor->GetFlowStats ();
  for (std::map<FlowId, FlowMonitor::FlowStats>::const_iterator i = stats.begin (); i != stats.end (); ++i) {
    // std::cout << " Tx Packets: " << i->second.txPackets << "\n";
    // std::cout << " Rx Packets:  "<< i->second.rxPackets << "\n";
    // std::cout << " Tx Bytes: " << i->second.txBytes << "\n";
    // std::cout << " Rx Bytes: " << i->second.rxBytes << "\n";
    std::cout << " Lost Packets: " << i->second.lostPackets << "\n";
  };

  Simulator::Destroy ();


  return 0;
}
