//
// Copyright (C) 2006-2012 Christoph Sommer <christoph.sommer@uibk.ac.at>
//
// Documentation for these modules is at http://veins.car2x.org/
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//

#ifndef InatisTestApp_H
#define InatisTestApp_H

#include <set>
#include <list>

#include <omnetpp.h>
#include <boost/unordered_map.hpp>

#include "veins/modules/application/ieee80211p/BaseWaveApplLayer.h"
#include "veins/base/modules/BaseApplLayer.h"
#include "veins/modules/mobility/traci/TraCIMobility.h"
#include "veins/modules/mobility/traci/TraCICommandInterface.h"
#include "veins/modules/world/annotations/AnnotationManager.h"
#include "InatisEntry.h"
#include "InatisBuffer.h"

using Veins::AnnotationManager;
using Veins::TraCIMobility;
using Veins::TraCICommandInterface;
using Veins::InatisEntry;
using Veins::InatisBuffer;

typedef std::set<double> map_type;
typedef boost::unordered_map<double,std::set<double>>::value_type map_value_type;

/**
 * FIXME
 */
namespace Veins {
class InatisTestApp : public BaseWaveApplLayer {
private:
        double startTime;

	public:
		int numInitStages() const { return std::max(BaseWaveApplLayer::numInitStages(), 1); }
		void initialize(int stage);
		void finish();
		double currentLaneLength=0.0f;
		simtime_t lastLaneChangeTime=0.0f;
		double lastLaneSpeed=0.0f;
		std::string lastJunction;
		InatisEntry currentEntry;
		InatisBuffer inatisBuffer;
		std::string lastPos="noPos";
		int lastSentTimedMessage = 0;


	protected:
		static const simsignalwrap_t mobilityStateChangedSignal;
		AnnotationManager* annotations;
		BaseMobility* mobi;
		bool sentMessage;

	protected:
		// module parameters
		bool debug;
		double maxOffset;
		int executionDuration;
		int inatisMsgInterval;

		TraCIMobility* mobility;
		TraCICommandInterface* traci;
		TraCICommandInterface::Vehicle* traciVehicle;
		bool hasStopped; /**< true if at some point in time this vehicle travelled at negligible speed */
		std::list<std::string> junctionIds; /**< set of junctions of the map */
		boost::unordered_map<double,std::set<double>> junctionsMap;

	protected:
		void handleSelfMsg(cMessage*);
		void handleLowerMsg(cMessage*);

		void receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj, cObject* details);

		void handlePositionUpdate();
		virtual void onData(WaveShortMessage* wsm);
		void sendInatisMessage(std::string inatisMessage);
		virtual void sendWSM(WaveShortMessage* wsm);
		virtual void onBeacon(WaveShortMessage* wsm);

private:
    void populateJunctionMap();
    bool checkIsInJunction(double vehicleXposition, double vehicleYposition);
    void printJunctionsMap();
};
}

#endif
