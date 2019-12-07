//------------------------------------------------------------------------
// Name:    gps_ProtocolData.h
// Author:  ProtocolGenerator (by jjuiddong)
// Date:    
//------------------------------------------------------------------------
#pragma once

namespace gps {

using namespace network2;
using namespace marshalling;


	struct GPSInfo_Packet {
		cProtocolDispatcher *pdispatcher;
		netid senderId;
		string date;
		double lon;
		double lat;
	};



}
