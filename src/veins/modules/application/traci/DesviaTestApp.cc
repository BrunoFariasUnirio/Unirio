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

#include <algorithm>
#include <string>
#include <boost/unordered_map.hpp>

#include "veins/base/utils/asserts.h"

#include "veins/modules/application/traci/DesviaTestApp.h"
#include "veins/modules/application/traci/InatisEntry.h"
#include "veins/modules/application/traci/InatisBuffer.h"
#include "veins/modules/mobility/traci/TraCIColor.h"

using Veins::TraCIMobility;
using Veins::TraCIMobilityAccess;

using Veins::DesviaTestApp;
using Veins::InatisEntry;
using Veins::InatisBuffer;

Define_Module(Veins::DesviaTestApp);

const simsignalwrap_t DesviaTestApp::mobilityStateChangedSignal = simsignalwrap_t(MIXIM_SIGNAL_MOBILITY_CHANGE_NAME);

static std::map<std::string, double> edges_lenghts;


void DesviaTestApp::initialize(int stage) {
	BaseWaveApplLayer::initialize(stage);
	startTime=simTime().dbl();
	findHost()->getDisplayString().updateWith("r=16,blue");
	if (stage == 0) {
		debug = par("debug");
		executionDuration = par("totalSimTime");
		inatisMsgInterval = par("inatisMsgInterval");

		// Initalize veins interface objects
		mobility = TraCIMobilityAccess().get(getParentModule());
		traci = mobility->getCommandInterface();
		traciVehicle = mobility->getVehicleCommandInterface();
/*
		std::list<std::string> l = traciVehicle->getPlannedRoadIds();

		std::cout << "Rota antes";
        for(auto p: l)
            std::cout << p << " ";
        std::cout <<std::endl;


        l = {"34","37","40","43","46","49"};
		for(auto p: l){
		    auto travelTime = traci->road(p).getCurrentTravelTime();
		    std::cout << "Segment "<< p << " travel time " << travelTime << std::endl;
		    travelTime+=200;
		    traciVehicle->changeRoute(p, travelTime);
		    auto k = traciVehicle->getPlannedRoadIds();
		    std::cout << "Nova rota Parcial";
		    for(auto x: k){
		        std::cout << x << " ";
		    }
		    std::cout <<std::endl;


		}


        l = {"34","37","40","43","46","49"};
        for(auto p: l){
            auto travelTime = traci->road(p).getCurrentTravelTime();
            std::cout << "Segment "<< p << " travel time " << travelTime << std::endl;
            travelTime-=202;
            traciVehicle->changeRoute(p, travelTime);
            auto k = traciVehicle->getPlannedRoadIds();
            std::cout << "Nova rota Parcial diminuida";
            for(auto x: k){
                std::cout << x << " ";
            }
            std::cout <<std::endl;


        }



		l = traciVehicle->getPlannedRoadIds();
        std::cout << "Nova rota final";
		for(auto p: l){
            std::cout << p << " ";
		}

        std::cout <<std::endl;

*/
/*
		for(auto x : traci->getLaneIds()){
		    auto roadId = traci->lane(x).getRoadId();
		    if(roadId.find('_') != std::string::npos){
		        continue;
		    }
		    auto travelTime = traci->road(roadId).getCurrentTravelTime();
		    std::cout << "Road Id = " << roadId << "\ttravel Time" << travelTime << std::endl;
		}

*
 *
 * 		std::list<std::string> l = {"25","28","31","34","37","40","43","46","49","52","55"};
		for(auto p: l){
            std::cout << "Segment "<< p << " travel time " << traci->road(p).getCurrentTravelTime() << std::endl;
            std::cout << "Segment "<< p << " mean speed " << traci->road(p).getMeanSpeed() << std::endl;
		}
		std::cout << " Planned road ids are: ";
		            for(auto p: traciVehicle->getPlannedRoadIds())
		                    std::cout << p << " ";

		            std::cout <<std::endl;


		// Catalog all Junctions
		junctionIds = traci->getJunctionIds();
		populateJunctionMap();
		lastJunction = "noJunctionYet";
        //printJunctionsMap();

*/
		// Initialize current segment info
		currentEntry.setDirection(0);
		currentEntry.setSegmentId(0L);
		currentEntry.setSpeed(0);
		currentEntry.setTimestamp(0);
	}
}

void DesviaTestApp::finish() {
}

void DesviaTestApp::handleSelfMsg(cMessage *msg) {
    std::cout << "At handleSelfMsg <" << findHost()->getId() << ">" << std::endl;
    findHost()->getDisplayString().updateWith("r=6,yellow");
}

void DesviaTestApp::handleLowerMsg(cMessage* msg) {
    findHost()->getDisplayString().updateWith("r=6,blue");
    WaveShortMessage *wsm = (WaveShortMessage *)msg;
    //std::cout << "Vehicle <" << findHost()->getId()<< "> received message"  << std::endl;

    InatisBuffer recvBuffer;
    recvBuffer.deserialize(wsm->getWsmData());

    //std::cout << "Vehicle <" << findHost()->getId()<< "> Deserialized " << std::endl;
    //recvBuffer.printMe();

    if (inatisBuffer.Merge(recvBuffer)){
        for(auto segmentId : inatisBuffer.getImpactedSegmentIds()){


            // Check if segment was already driven through...
            bool alreadyDriven = false;
            std::string currentRoadId = traciVehicle->getRoadId();
            for(auto p: traciVehicle->getPlannedRoadIds()){
                if (p.compare(segmentId)==0){
                    alreadyDriven=true;
                }
                if (p.compare(currentRoadId)==0){
                    if (!alreadyDriven){
                        InatisEntry segInfo = inatisBuffer.retrieveSegmentInfo(stol(segmentId));
                        // Oracle from sumo: auto travelTime = traci->road(segmentId).getCurrentTravelTime();


                        double travelTime =  GetRoadLength(segmentId) / segInfo.getSpeed();

                        std::cout << "Vehicle <" << findHost()->getId() << "> will reroute for segment " << segmentId << " speed "
                                << segInfo.getSpeed() << " time " << travelTime << std::endl;
                        std::cout << "Vehicle <" << findHost()->getId() << "> Planned road ids were:  \t";
                                    for(auto p: traciVehicle->getPlannedRoadIds())
                                            std::cout << p << " ";
                        traciVehicle->changeRoute(segmentId, travelTime);
                        std::cout << std::endl << "Vehicle <" << findHost()->getId() << "> Now Planned road ids are: \t";
                        for(auto p: traciVehicle->getPlannedRoadIds())
                                std::cout << p << " ";

                        std::cout <<std::endl;
                    }
                }else{
                    // not rerouting as the impacted segment has already been driven through
                    break;
                }
            }
        }

        // retransmit if new data
      //  std::cout << "Vehicle <" << findHost()->getId()<< "> Will retransmit message "<<std::endl;
        sendInatisMessage(inatisBuffer.serialize());
    }

	delete msg;
}

void DesviaTestApp::receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj, cObject* details) {
    Enter_Method_Silent();
	if (signalID == mobilityStateChangedSignal) {
		handlePositionUpdate();
	}
}

void DesviaTestApp::handlePositionUpdate() {

    int executionTime =(int)(simTime().dbl() - startTime);
    if (((executionTime % inatisMsgInterval) == 0) && (executionTime > lastSentTimedMessage)){
        std::cout << "Vehicle " << findHost()->getId() << " will send timed message at time " <<  simTime().dbl() << std::endl;
        sendInatisMessage(inatisBuffer.serialize());
        lastSentTimedMessage=executionTime;
    }

    // Update current segment
    std::string currentRoadId =  mobility->getRoadId();

/*
    if (lastPos.compare(r)!=0){
        lastPos = mobility->getRoadId();
        std::cout << "Vehicle " << findHost()->getId() << " position " <<  lastPos << std::endl;
    }

    double vehicleXposition = mobility->getCurrentPosition().x;
	double vehicleYposition = mobility->getCurrentPosition().y;

    if (checkIsInJunction(floor(vehicleXposition), floor(vehicleYposition))){*/
	if(currentRoadId.find('_') != std::string::npos){

	    findHost()->getDisplayString().updateWith("r=6,red");

	    std::string currentJunction =  mobility->getRoadId();
	    if (currentJunction.compare(lastJunction)!= 0){
	        lastJunction = currentJunction;

	        // calculate data to disseminate - vehicle in junction
            lastLaneSpeed = ( currentLaneLength / (simTime() - lastLaneChangeTime));
            std::cout << "Vehicle " << findHost()->getId() <<  " was at road id " << currentEntry.getSegmentId() << " from time<"
                      << lastLaneChangeTime << "> to <" << simTime() <<
                      "> Lane speed " << lastLaneSpeed << "m/s \n";

            currentEntry.setSpeed((int)floor(lastLaneSpeed));
            currentEntry.setTimestamp(floor(simTime().dbl()));

            inatisBuffer.forceEntry(currentEntry);
            sendInatisMessage(inatisBuffer.serialize());
            reroutingByProbabilityEvaluated = false;

	    }else {
	        //std::cout << "Vehicle " << findHost()->getId() << " already disseminated message at junction " << currentJunction << std::endl;
	    }
    }else{
        findHost()->getDisplayString().updateWith("r=6,green");

        if (currentEntry.getSegmentId()!=stol(currentRoadId)){
            currentEntry.setSegmentId(stol(currentRoadId));
            std::string currentLaneId = traciVehicle->getLaneId();
            currentLaneLength = traci->lane(currentLaneId).getLength();

            lastLaneChangeTime = simTime();
                        std::cout << "Vehicle " << findHost()->getId() <<  " at road id " << currentRoadId << ". "
                                "Lane length " << currentLaneLength << ". SimTime " << lastLaneChangeTime << "\n";

            delayFactorThreshold=2;

            InatisEntry entry = inatisBuffer.retrieveSegmentInfo(stol(currentRoadId));

            if (entry.getSegmentId() != 0L){
                currentLaneExpectedTravelTime = GetRoadLength(currentRoadId) / entry.getSpeed();
            }else{
                currentLaneExpectedTravelTime = GetRoadLength(currentRoadId) / traci->lane(currentLaneId).getMaxSpeed();
            }
        }else{
            // Check if it's taking too much time at this same segment...
             simtime_t timeAtSegment = simTime().dbl() - lastLaneChangeTime;
             double tatseg = timeAtSegment.dbl() ;
             double delayFactor = tatseg / currentLaneExpectedTravelTime;

             if (delayFactor > delayFactorThreshold){
                 delayFactorThreshold+=2;
                 std::cout << "Vehicle " << findHost()->getId() <<  " staying too long at road id " << currentRoadId << std::endl;
                 double newSpeed = (currentEntry.getSpeed() / delayFactor);



                 InatisEntry e;
                 e.setSpeed((int)floor(newSpeed));
                 e.setSegmentId(stol(currentRoadId));
                 e.setTimestamp(floor(simTime().dbl()));

                 inatisBuffer.forceEntry(currentEntry);
                 sendInatisMessage(inatisBuffer.serialize());

                 // Reroute the vehicle assuming the next segment might be congested.
                 bool foundSegmentOnRoute = false;
                 if (!reroutingByProbabilityEvaluated){
                     reroutingByProbabilityEvaluated=true;
                     //Vehicle will probably assume next segment might also be congested
                     for (auto s : traciVehicle->getPlannedRoadIds()){
                         if (foundSegmentOnRoute){
                             if (uniform(0,1) > 0.5){
                                 std::cout << "Rerouting vehicle "  << findHost()->getId() << " as road Id " << s << " might be congested" << std::endl;
                                 std::cout << "Vehicle <" << findHost()->getId() << "> Planned road ids were: \t";
                                                         for(auto p: traciVehicle->getPlannedRoadIds())
                                                                 std::cout << p << " ";
                                 traciVehicle->changeRoute(s, (GetRoadLength(s) / inatisBuffer.retrieveSegmentInfo(stol(s)).getSpeed()) + delayFactorThreshold + 1000);
                                 std::cout << std::endl << "Vehicle <" << findHost()->getId() << "> New planned road ids are: \t";
                                                         for(auto p: traciVehicle->getPlannedRoadIds())
                                                                 std::cout << p << " ";
                                 std::cout << std::endl;
                             }

                             break;
                         }
                         if (s.compare(currentRoadId) == 0){
                             foundSegmentOnRoute = true;
                         }
                     }
                 }
             }
        }
    }
}

void DesviaTestApp::onData(WaveShortMessage* wsm) {

    std::cout << "At  onData <" << findHost()->getId() << ">" << std::endl;
    findHost()->getDisplayString().updateWith("r=16,gray");

    annotations->scheduleErase(1, annotations->drawLine(wsm->getSenderPos(), mobi->getCurrentPosition(), "blue"));

}

void DesviaTestApp::onBeacon(WaveShortMessage* wsm) {

}


void DesviaTestApp::sendInatisMessage(std::string inatisMessage) {

    findHost()->getDisplayString().updateWith("r=16,gray");
    t_channel channel = dataOnSch ? type_SCH : type_CCH;
    WaveShortMessage* wsm = prepareWSM("data", dataLengthBits, channel, dataPriority, -1,2);

    wsm->setWsmData(inatisMessage.c_str());
    sendWSM(wsm);

}
void DesviaTestApp::sendWSM(WaveShortMessage* wsm) {

    sendDelayedDown(wsm,individualOffset);
}

bool DesviaTestApp::checkIsInJunction(double vehicleXposition, double vehicleYposition) {

    auto it = junctionsMap.find(vehicleXposition);

    if (it != junctionsMap.end()) {
       std::set<double> * x = &it->second;

       for(std::set<double>::iterator it=x->begin(); it != x->end(); ++it){
              if(vehicleYposition == *it){
                  std::cout << "Vehicle <" << findHost()->getId() << "> in junction x<" << vehicleXposition << "> y<"
                                      << vehicleYposition << ">" << std::endl;
              }
       }
       return true;
    }
    return false;
}

void DesviaTestApp::populateJunctionMap() {
    for (std::string s : junctionIds) {
        double junctionXCoord =
                floor(traci->junction(s).getPosition().x);
        double junctionYCoord =
                floor(traci->junction(s).getPosition().y);

        //std::cout << "Will insert junction name<" << s << "> x<"<< junctionXCoord << "> y<" << junctionYCoord << ">" << std::endl;

        auto it = junctionsMap.find(junctionXCoord);

        if (it == junctionsMap.end()) {
            std::set<double> * newSet;
            newSet = new std::set<double>();
            newSet->insert(junctionYCoord);
            junctionsMap.insert(std::make_pair(junctionXCoord, *newSet));
        } else {
            std::set<double> * x = &it->second;
            x->insert(junctionYCoord);
        }
    }
}

void DesviaTestApp::printJunctionsMap() {
    std::cout << "Dumping.. " << std::endl;
    for (auto itt = junctionsMap.begin(); itt != junctionsMap.end(); ++itt) {
        std::cout << itt->first << " , ";
        std::set<double> x = itt->second;
        std::cout << "Set contains:";
        for (std::set<double>::iterator it = x.begin(); it != x.end(); ++it) {
            std::cout << ' ' << *it;
        }
        std::cout << std::endl;
    }
}

std::map<std::string, double> DesviaTestApp::GetRoadsLengths() {
    if (edges_lenghts.size() == 0){
        for(auto laneId : traci->getLaneIds()){
            edges_lenghts.insert(std::make_pair(traci->lane(laneId).getRoadId(),traci->lane(laneId).getLength()));
        }
    }

    return edges_lenghts;
}

double DesviaTestApp::GetRoadLength(std::string roadId) {
    if (edges_lenghts.size() == 0){
                for(auto laneId : traci->getLaneIds()){
                    edges_lenghts.insert(std::make_pair(traci->lane(laneId).getRoadId(),traci->lane(laneId).getLength()));
                }
            }

        std::map<std::string,double>::iterator it;

        it = edges_lenghts.find(roadId);
        if (it != edges_lenghts.end())
            return (double)it->second;

        return (double)-1;
}

/*
std::map<std::string, double> DesviaTestApp::GetRoadsTravelTimes() {
    if (edges_travelTimes.size() == 0){
        for(auto laneId : traci->getLaneIds()){
            edges_lenghts.insert(std::make_pair(traci->lane(laneId).getRoadId(),traci->lane(laneId).getLength()));
        }
    }

    return edges_lenghts;
}

double DesviaTestApp::GetRoadTravelTime(std::string roadId) {
    if (edges_travelTimes.size() == 0){
        for(auto laneId : traci->getLaneIds()){
            edges_travelTimes.insert(std::make_pair(traci->lane(laneId).getRoadId(),traci->road(traci->lane(laneId).getRoadId()).getCurrentTravelTime()));
        }
    }

    std::map<std::string,double>::iterator it;

    it = edges_travelTimes.find(roadId);
    if (it != edges_travelTimes.end())
        return (double)it->second;

    return (double)-1;
}*/



