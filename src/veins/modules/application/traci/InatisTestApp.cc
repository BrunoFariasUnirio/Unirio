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

#include "veins/modules/application/traci/InatisTestApp.h"
#include "veins/modules/application/traci/InatisEntry.h"
#include "veins/modules/application/traci/InatisBuffer.h"
#include "veins/modules/mobility/traci/TraCIColor.h"

using Veins::TraCIMobility;
using Veins::TraCIMobilityAccess;

using Veins::InatisTestApp;
using Veins::InatisEntry;
using Veins::InatisBuffer;

Define_Module(Veins::InatisTestApp);

const simsignalwrap_t InatisTestApp::mobilityStateChangedSignal = simsignalwrap_t(MIXIM_SIGNAL_MOBILITY_CHANGE_NAME);

void InatisTestApp::initialize(int stage) {
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

void InatisTestApp::finish() {
}

void InatisTestApp::handleSelfMsg(cMessage *msg) {
    std::cout << "At handleSelfMsg <" << findHost()->getId() << ">" << std::endl;
    findHost()->getDisplayString().updateWith("r=6,yellow");
}

void InatisTestApp::handleLowerMsg(cMessage* msg) {
    findHost()->getDisplayString().updateWith("r=6,blue");
    WaveShortMessage *wsm = (WaveShortMessage *)msg;
    std::cout << "Vehicle <" << findHost()->getId()<< "> received message"  << std::endl;

    InatisBuffer recvBuffer;
    recvBuffer.deserialize(wsm->getWsmData());

    std::cout << "Vehicle <" << findHost()->getId()<< "> Deserialized " << std::endl;
    //recvBuffer.printMe();

    if (inatisBuffer.Merge(recvBuffer)){
        // retransmit if new data
      //  std::cout << "Vehicle <" << findHost()->getId()<< "> Will retransmit message "<<std::endl;
        sendInatisMessage(inatisBuffer.serialize());
    }

	delete msg;
}

void InatisTestApp::receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj, cObject* details) {
    Enter_Method_Silent();
	if (signalID == mobilityStateChangedSignal) {
		handlePositionUpdate();
	}
}

void InatisTestApp::handlePositionUpdate() {

    int executionTime =(int)(simTime().dbl() - startTime);
    if (((executionTime % inatisMsgInterval) == 0) && (executionTime > lastSentTimedMessage)){
        std::cout << "Vehicle " << findHost()->getId() << " will send timed message at time " <<  simTime().dbl() << std::endl;
        sendInatisMessage(inatisBuffer.serialize());
        lastSentTimedMessage=executionTime;
    }
    // Update current segment
    std::string r =  mobility->getRoadId();

/*
    if (lastPos.compare(r)!=0){
        lastPos = mobility->getRoadId();
        std::cout << "Vehicle " << findHost()->getId() << " position " <<  lastPos << std::endl;
    }

    double vehicleXposition = mobility->getCurrentPosition().x;
	double vehicleYposition = mobility->getCurrentPosition().y;

    if (checkIsInJunction(floor(vehicleXposition), floor(vehicleYposition))){*/
	if(r.find('_') != std::string::npos){

	    findHost()->getDisplayString().updateWith("r=6,red");

	    std::string currentJunction =  mobility->getRoadId();
	    if (currentJunction.compare(lastJunction)!= 0){
	        lastJunction = currentJunction;

	        // calculate data to disseminate
            lastLaneSpeed = ( currentLaneLength / (simTime() - lastLaneChangeTime));
            std::cout << "Vehicle " << findHost()->getId() <<  " was at road id " << currentEntry.getSegmentId() << " from time<"
                      << lastLaneChangeTime << "> to <" << simTime() <<
                      "> Lane speed " << lastLaneSpeed << "m/s \n";

            currentEntry.setSpeed((int)floor(lastLaneSpeed));
            currentEntry.setTimestamp(floor(simTime().dbl()));

            inatisBuffer.forceEntry(currentEntry);
            sendInatisMessage(inatisBuffer.serialize());
	    }else {
	        //std::cout << "Vehicle " << findHost()->getId() << " already disseminated message at junction " << currentJunction << std::endl;
	    }
    }else{
        findHost()->getDisplayString().updateWith("r=6,green");

        if (currentEntry.getSegmentId()!=stol(r)){
            currentEntry.setSegmentId(stol(r));
            currentLaneLength = mobility->getCommandInterface()->lane(
                                mobility->getVehicleCommandInterface()->getLaneId()
                                                                          ).getLength();
            lastLaneChangeTime = simTime();
                        std::cout << "Vehicle " << findHost()->getId() <<  " at road id " << r << ". "
                                "Lane length " << currentLaneLength << ". SimTime " << lastLaneChangeTime << "\n";

        }
    }
}

void InatisTestApp::onData(WaveShortMessage* wsm) {

    std::cout << "At  onData <" << findHost()->getId() << ">" << std::endl;
    findHost()->getDisplayString().updateWith("r=16,gray");

    annotations->scheduleErase(1, annotations->drawLine(wsm->getSenderPos(), mobi->getCurrentPosition(), "blue"));

}

void InatisTestApp::onBeacon(WaveShortMessage* wsm) {

}


void InatisTestApp::sendInatisMessage(std::string inatisMessage) {

    t_channel channel = dataOnSch ? type_SCH : type_CCH;
    WaveShortMessage* wsm = prepareWSM("data", dataLengthBits, channel, dataPriority, -1,2);

    wsm->setWsmData(inatisMessage.c_str());
    sendWSM(wsm);

}
void InatisTestApp::sendWSM(WaveShortMessage* wsm) {

    sendDelayedDown(wsm,individualOffset);
}

bool InatisTestApp::checkIsInJunction(double vehicleXposition, double vehicleYposition) {

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

void InatisTestApp::populateJunctionMap() {
    for (std::string s : junctionIds) {
        double junctionXCoord =
                floor(mobility->getCommandInterface()->junction(s).getPosition().x);
        double junctionYCoord =
                floor(mobility->getCommandInterface()->junction(s).getPosition().y);

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

void InatisTestApp::printJunctionsMap() {
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
