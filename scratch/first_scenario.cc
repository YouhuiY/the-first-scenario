/* this version-1 program is to builf a wifi network contain two type of devicecs: delay-critical and delay_tolerant ( need to be expand) and test the throughput with
different nodes numbers
*/

/*version2 add csma
*/



#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/applications-module.h"
#include "ns3/wifi-module.h"
#include "ns3/mobility-module.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/internet-module.h"
#include "ns3/csma-module.h"
#include "ns3/bridge-helper.h"
#include <assert.h> 
#include <set>
#include <fstream>
#include <iostream>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("first scenario");

int main (int argc, char *argv[])
{
	double simulationTime = 5; //seconds

	int mcs = 5;
    int channelWidth = 80;
    int gi = 1; //Guard Interval

    int DC_nsta = 10; //number of delay-critical stations
    int DT_nsta = 100; //number of delay-tolerant stations

    CommandLine cmd;

    cmd.AddValue ("simulationTime", "Simulation time in seconds", simulationTime);

    cmd.AddValue ("mcs", "Modulation and Coding scheme", mcs);
    cmd.AddValue ("channelWidth", "WiFi Channel width", channelWidth);
    cmd.AddValue ("gi", "Guard Interval", gi);

    cmd.AddValue ("DC_nsta", "Number of delay-critical stations per AP", DC_nsta);
    cmd.AddValue ("DT_nsta", "Number of delay-tolerant stations per AP", DT_nsta);

    cmd.Parse (argc,argv);

    Time::SetResolution (Time::NS);
    LogComponentEnable ("first scenario", LOG_LEVEL_INFO);

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
    	DC_nsta = 10;
    	std::cout<<" Invalid input, DC_nsta set to 10"<<std::endl;
    }
    if(DT_nsta <= 0)
    {
    	DT_nsta = 100;
    	std::cout<<" Invalid input, DT_nsta set to 100"<<std::endl;
    }

    uint32_t payloadSize = 1472;
    
    NodeContainer DCStaNodes;
    DCStaNodes.Create (DC_nsta);
    NodeContainer DTStaNodes;
    DTStaNodes.Create (DT_nsta);
    NodeContainer wifiApNode;
    wifiApNode.Create (1);
    NodeContainer csmaNodes;
    csmaNodes.Create(1);
    csmaNodes.Add(wifiApNode.Get(0));
    NetDeviceContainer csmaDevices;
    Ipv4InterfaceContainer csmaInterfaces;





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
    
     MobilityHelper mobility;
     mobility.SetPositionAllocator ("ns3::GridPositionAllocator",
                                 "MinX", DoubleValue (-100.0),
                                 "MinY", DoubleValue (-100.0),
                                 "DeltaX", DoubleValue (5.0),
                                 "DeltaY", DoubleValue (20.0),
                                 "GridWidth", UintegerValue (20),
                                 "LayoutType", StringValue ("RowFirst"));
     
    mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
    mobility.Install (wifiApNode);
    mobility.Install (DCStaNodes);
    mobility.Install (DTStaNodes);

    InternetStackHelper stack;
    stack.Install (wifiApNode);
    stack.Install (DCStaNodes);
    stack.Install (DTStaNodes);
    stack.Install (csmaNodes.Get(0));


    CsmaHelper csma;
    csmaDevices = csma.Install (csmaNodes);//




    Ipv4AddressHelper address;
    address.SetBase ("192.168.0.0", "255.255.0.0");
  
    Ipv4InterfaceContainer apNodeInterface;
    Ipv4InterfaceContainer DCstaNodeInterface;
    Ipv4InterfaceContainer DTstaNodeInterface;

    csmaInterfaces.Add(address.Assign(csmaDevices.Get(0)));

    BridgeHelper bridge;
    NetDeviceContainer bridgeDev;
    bridgeDev = bridge.Install (csmaNodes.Get (1), NetDeviceContainer (apDevice, csmaDevices.Get (1)));
    apNodeInterface = address.Assign (bridgeDev);

    //apNodeInterface.Add (address.Assign (apDevice.Get (0)));
    

    int idx = 0;
    for (int s = 0; s < DC_nsta; s++)
    { 
          DCstaNodeInterface.Add( address.Assign (DC_staDevices.Get (idx)));
          idx ++;
    }
    
    idx = 0;
    for (int s = 0; s < DT_nsta; s++)
    { 
          DTstaNodeInterface.Add( address.Assign (DT_staDevices.Get (idx)));
          idx ++;
    }
    ApplicationContainer serverApps, clientApps;

    int totalSta = DC_nsta + DT_nsta;
    int portBase = 9000;
    idx = 0;


    for (int s = 0; s < totalSta; s++)
    {
    	UdpServerHelper myServer (portBase + idx);
        ApplicationContainer server = myServer.Install (wifiApNode.Get (0));
        server.Start (Seconds (0.0));
        server.Stop (Seconds (simulationTime + 1));
        serverApps.Add (server);
        idx++;
    }
    idx = 0;
    for(int s = 0; s < DC_nsta; s++)
    {
        UdpClientHelper myClient (apNodeInterface.GetAddress (0), (portBase + idx));
        myClient.SetAttribute ("MaxPackets", UintegerValue (4294967295u));
        myClient.SetAttribute ("Interval", TimeValue (Time ("0.0000001"))); //packets/s
        myClient.SetAttribute ("PacketSize", UintegerValue (payloadSize));
        ApplicationContainer client = myClient.Install (DCStaNodes.Get (idx));
        //printf("%d\n", s);
        
        client.Start (Seconds (1.0));
        client.Stop (Seconds (simulationTime + 1));
        clientApps.Add (client);

        idx ++;
    }
    
    assert (idx == DCstaNodeInterface.GetN ());
    //printf("out\n");

    for(int s = 0; s < DT_nsta; s++)
    {
        UdpClientHelper myClient (apNodeInterface.GetAddress (0), (portBase + idx));
        myClient.SetAttribute ("MaxPackets", UintegerValue (4294967295u));
        myClient.SetAttribute ("Interval", TimeValue (Time ("0.0000001"))); //packets/s
        myClient.SetAttribute ("PacketSize", UintegerValue (payloadSize));
        ApplicationContainer client = myClient.Install (DTStaNodes.Get (idx - DC_nsta));
        client.Start (Seconds (1.0));
        client.Stop (Seconds (simulationTime + 1));
        clientApps.Add (client);
        //printf("%d\n", s);
        idx ++;
    }
    //printf("out\n");
    assert (idx == DTstaNodeInterface.GetN () +  DCstaNodeInterface.GetN ());
    //printf("out\n");
    Ipv4GlobalRoutingHelper::PopulateRoutingTables ();
    Simulator::Stop (Seconds (simulationTime + 1));
    //printf("out\n");

    Simulator::Run ();
   // printf("out\n");
    Simulator::Destroy ();

    int T = 0; 
    idx = 0;
    for (int s = 0; s < totalSta; s++)
    { 
        double throughput = 0;

        int totalPacketsThrough = DynamicCast<UdpServer> (serverApps.Get (idx))->GetReceived ();
        throughput = totalPacketsThrough * payloadSize * 8 / (simulationTime * 1000000.0); //Mbit/s
        std::cout << "\t--->STA: " << s << ", throughput: " << throughput << " Mbps" << std::endl;

        idx ++;
        T += throughput;
    }
    std::cout << "\t the total throughput: " << T << " Mbps" << std::endl;
    std::ofstream outData;
    outData.open("/home/youhui/Downloads/ns-allinone-3.26/ns-3.26/first_scenario.dat");
    if (!outData)
    {
    	std::cout <<"can not open the file"<< std::endl;
    }
    else
    {
    	outData <<DC_nsta<<"\t"<<DT_nsta<<"\t" << T << std::endl;
    	outData.close();
    }

    return 0;
}

