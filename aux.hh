#ifndef _AUX_
#define _AUX_


#include "ns3/address.h"
#include "ns3/traced-value.h"
#include "ns3/trace-source-accessor.h"

using namespace ns3;
  
 int DC_nsta = 64; //number of delay-critical stations
 int DT_nsta = 64; //number of delay-tolerant stations
 uint32_t ap_one;
 uint32_t DC_down;
 uint32_t DC_up;
 uint32_t DT_down;
 uint32_t DT_up;
 uint64_t DC_DelayTime = 0;
 uint64_t DT_DelayTime = 0;
#endif
