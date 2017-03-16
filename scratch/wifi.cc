#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/applications-module.h"
#include "ns3/wifi-module.h"
#include "ns3/mobility-module.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/internet-module.h"
#include <assert.h> 
#include <set>
#include <fstream>
#include <iostream>
#include <string>

//my includes
#include "/home/youhui/Downloads/ns-allinone-3.26/ns-3.26/includes/aux.hh"


using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("before_change"); 

struct results
{
  double hits = 0;  
};
struct results result;

int v;
std::ofstream outData;
// define the trace function
void DC_Delay (struct results *result, Time oldValue, Time newValue)
{

  uint64_t temp_new = newValue.GetMilliSeconds ();
  //outData.open("/home/youhui/Downloads/ns-allinone-3.26/ns-3.26/scratch/delay_0.dat", std::ios::app);
  DC_DelayTime += temp_new;
  
}

void DT_Delay (struct results *result, Time oldValue, Time newValue)
{

  uint64_t temp_new = newValue.GetMilliSeconds ();
  DT_DelayTime += temp_new;
  
}
 
int main (int argc, char *argv[])
{
  double simulationTime = 60; //seconds
  int mcs = 9;
  int channelWidth = 160;
  int gi = 1; //Guard Interval

 //////////////////////////

  uint32_t payloadSize = 1472; 
  double distance = 1.0; //meters

  CommandLine cmd;

  cmd.AddValue ("simulationTime", "Simulation time in seconds", simulationTime);

  cmd.AddValue ("mcs", "Modulation and Coding scheme", mcs);
  cmd.AddValue ("channelWidth", "WiFi Channel width", channelWidth);
  cmd.AddValue ("gi", "Guard Interval", gi);

  cmd.AddValue ("DC_nsta", "Number of delay-critical stations per AP", DC_nsta);
  cmd.AddValue ("DT_nsta", "Number of delay-tolerant stations per AP", DT_nsta);

  cmd.AddValue ("payloadSize", "the size of payload", payloadSize);

  cmd.Parse (argc,argv);

  Time::SetResolution (Time::NS);
  LogComponentEnable ("before_change", LOG_LEVEL_INFO);

  if (simulationTime <= 0)
  {

    simulationTime = 10;
    std::cout<<" Invalid input, simulationTime set to 10 seconds"<<std::endl;
  }
  if (mcs > 9 || mcs < 0)
  {
    mcs = 5;
    std::cout<<" Invalid input, mcs set to 5"<<std::endl;
  }

  std::set<int> cw;
  std::set<int>::iterator it_cw;
  for (int i = 1; i <= 4; i++)
    cw.insert (pow(2,i) * 10);
  it_cw = cw.find (channelWidth);
  if (it_cw == cw.end())
  {
    channelWidth = 80;
    std::cout<<" Invalid input, channelWidth set to 80MHZ"<<std::endl;
  }

  if (gi != 0 && gi != 1)
  {
    gi = 1;
    std::cout<<" Invalid input, gi set to 1"<<std::endl;
  }
    
  if (DC_nsta <= 0)
  {
    DC_nsta = 64;
    std::cout<<" Invalid input, DC_nsta set to 10"<<std::endl;
  }
  if(DT_nsta <= 0)
  {
    DT_nsta = 64;
    std::cout<<" Invalid input, DT_nsta set to 100"<<std::endl;
  }

  if(payloadSize <= 0)
  {
    payloadSize = 1427;
    std::cout << " invalid input, payloadSize set to 1427" << std::endl;

  }

  for(v = 1; v <= DC_nsta; v+=2)
  {

    NodeContainer DCStaNodes;
    DCStaNodes.Create (v);
    NodeContainer DTStaNodes;
    DTStaNodes.Create (v);
    NodeContainer wifiApNode;
    wifiApNode.Create (1);
    std::cout << "MCS value: " << mcs << "Channel width: " << channelWidth << "short GI: " << gi << "number of delay-critical devices: " << v << "number of delay-tolerant devices: " << v << std::endl;


    YansWifiChannelHelper channel = YansWifiChannelHelper::Default ();
    YansWifiPhyHelper phy = YansWifiPhyHelper::Default ();
    phy.SetChannel (channel.Create ());
    
      // Set guard interval
    phy.Set ("ShortGuardEnabled", BooleanValue (gi));

    WifiHelper wifi;
    wifi.SetStandard (WIFI_PHY_STANDARD_80211ac);
    WifiMacHelper mac;
                  
    std::ostringstream oss;
    oss << "VhtMcs" << mcs;
    wifi.SetRemoteStationManager ("ns3::ConstantRateWifiManager","DataMode", StringValue (oss.str ()),
                                  "ControlMode", StringValue (oss.str ()));
    

    Ssid ssid = Ssid ("ns3-80211ac");

    mac.SetType ("ns3::StaWifiMac",
                "Ssid", SsidValue (ssid));

    NetDeviceContainer DC_staDevices;
    DC_staDevices = wifi.Install (phy, mac,DCStaNodes);
    NetDeviceContainer DT_staDevices;
    DT_staDevices = wifi.Install (phy, mac,DTStaNodes);

    mac.SetType ("ns3::ApWifiMac",
                "Ssid", SsidValue (ssid));

    NetDeviceContainer apDevice;
    apDevice = wifi.Install (phy, mac, wifiApNode);


    Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator> ();
      
    MobilityHelper mobility;
    positionAlloc->Add (Vector (0.0, 0.0, 0.0));
    positionAlloc->Add (Vector (distance, 0.0, 0.0));
    positionAlloc->Add (Vector (distance, 0.0, 0.0));
    

    mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");

    mobility.Install (wifiApNode);
    mobility.Install (DCStaNodes);
    mobility.Install (DTStaNodes);

    InternetStackHelper stack;
    stack.Install (wifiApNode);
    stack.Install (DCStaNodes);
    stack.Install (DTStaNodes);

    Ipv4AddressHelper address;
    address.SetBase ("192.168.0.0", "255.255.0.0");
    
    Ipv4InterfaceContainer apNodeInterface;
    Ipv4InterfaceContainer DCstaNodeInterface;
    Ipv4InterfaceContainer DTstaNodeInterface;

    apNodeInterface.Add( address.Assign (apDevice));
    //uint32_t  apint = apNodeInterface.GetAddress(0)//.ConvertTo().m_data();
   // std::string st = apNodeInterface.GetAddress(0).toString();
   // std::cout <<"the ap's address is: "<<st<< std::endl;
   
    int idx = 0;
    for (int s = 0; s < v; s++)
    { 
      DCstaNodeInterface.Add( address.Assign (DC_staDevices.Get (idx)));
      idx ++;
    }
      
    idx = 0;
    for (int s = 0; s < v; s++)
    {
      DTstaNodeInterface.Add( address.Assign (DT_staDevices.Get (idx)));
      idx ++;
    }
//uint32_t ni = apNodeInterface.GetAddress(0).ptr;
/*  ******************** test *********************

std::cout <<apNodeInterface.GetAddress(0).m_address<< std::endl;
std::cout <<apNodeInterface.GetAddress(0)<< std::endl;
std::cout <<DCstaNodeInterface.GetAddress(0).m_address<< std::endl;
std::cout <<DCstaNodeInterface.GetAddress(0)<< std::endl;

*/
   
   ap_one = apNodeInterface.GetAddress(0).m_address;
   DC_down = DCstaNodeInterface.GetAddress(0).m_address;
   DC_up = DCstaNodeInterface.GetAddress(v - 1).m_address;
   DT_down = DTstaNodeInterface.GetAddress(0).m_address;
   DT_up = DTstaNodeInterface.GetAddress(v - 1).m_address;
   std::cout <<DT_up<< std::endl;


     // ApplicationContainer serverApp, DC_clientApps, DT_clientApps;

    int port = 9000;
    idx = 0;

    ApplicationContainer serverApp;

    UdpServerHelper myServer (9);
    serverApp = myServer.Install (wifiApNode.Get (0));
    serverApp.Start (Seconds (0.0));
    serverApp.Stop (Seconds (simulationTime + 1));

    UdpClientHelper myClient (apNodeInterface.GetAddress (0), 9);
    myClient.SetAttribute ("MaxPackets", UintegerValue (4294967295u));
    myClient.SetAttribute ("Interval", TimeValue (Time ("0.00001"))); //packets/s
    myClient.SetAttribute ("PacketSize", UintegerValue (payloadSize));

    ApplicationContainer clientApps;

    clientApps.Add (myClient.Install (DCStaNodes));
    clientApps.Add (myClient.Install (DTStaNodes));
    clientApps.Start (Seconds (1.0));
    clientApps.Stop (Seconds (simulationTime + 1));
      

  
    Ipv4GlobalRoutingHelper::PopulateRoutingTables ();
    // tracer callback function
    Ptr<UdpServer> server = DynamicCast<UdpServer> (serverApp.Get (0));
    server->TraceConnectWithoutContext ("Delay_DC", MakeBoundCallback (&DC_Delay, &result));
    
    server->TraceConnectWithoutContext ("Delay_DT", MakeBoundCallback (&DT_Delay, &result));

    
    //Ptr<UdpServer> server = DynamicCast<UdpServer> (serverApp.Get(0));
    //server->TraceConnectWithoutContext ("Delay", MakeBoundCallback (&TraceArray));
    for( int u = 0; u < 1; u ++)
    {
      Simulator::Stop (Seconds (simulationTime + 1));
      Simulator::Run ();
      Simulator::Destroy ();
    }

    double T = 0.0;
    uint32_t totalPacketsThrough = DynamicCast<UdpServer> (serverApp.Get (0))->GetReceived ();
    std::cout<<totalPacketsThrough<<std::endl;
    uint64_t m = totalPacketsThrough*8*payloadSize;
    std::cout<<m<<std::endl;
    uint32_t n = simulationTime*1000000.0; 
    std::cout<<n<<std::endl;
    T = m / n / 20.0; //Mbit/s

    std::ofstream outData;
    outData.open("/home/youhui/Downloads/ns-allinone-3.26/ns-3.26/scratch/wifi.dat", std::ios::app);
    if (!outData)
    {
      std::cout <<"can not open the file"<< std::endl;
    }
    else
    {
      outData <<v<<"\t"<<v<<"\t" << T << std::endl;
      outData.close();
    }

    //add delay to files
    std::ofstream delayData;
    outData.open("/home/youhui/Downloads/ns-allinone-3.26/ns-3.26/scratch/delay_1.dat", std::ios::app);
    if (!delayData)
    {
      std::cout <<"can not open the file"<< std::endl;
    }
    else
    {
      outData <<v<<"\t"<<DC_DelayTime<<"\t" << DT_DelayTime << std::endl;
      outData.close();
    }
    DT_DelayTime = 0;
    DC_DelayTime = 0;

}
    
  return 0;
}
