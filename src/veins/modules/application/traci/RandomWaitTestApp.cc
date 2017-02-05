#include <algorithm>
#include <string>
#include <boost/unordered_map.hpp>
#include "veins/base/utils/asserts.h"

#include "veins/modules/application/traci/RandomWaitTestApp.h"
#include "veins/modules/application/traci/InatisEntry.h"
#include "veins/modules/application/traci/InatisBuffer.h"
#include "veins/modules/mobility/traci/TraCIColor.h"

using Veins::TraCIMobility;
using Veins::TraCIMobilityAccess;

using Veins::RandomWaitTestApp;
using Veins::InatisEntry;
using Veins::InatisBuffer;

Define_Module(Veins::RandomWaitTestApp);

const simsignalwrap_t RandomWaitTestApp::mobilityStateChangedSignal = simsignalwrap_t(MIXIM_SIGNAL_MOBILITY_CHANGE_NAME);

void RandomWaitTestApp::initialize(int stage) {
	BaseWaveApplLayer::initialize(stage);
	startTime=simTime().dbl();
	findHost()->getDisplayString().updateWith("r=16,blue");
	if (stage == 0) {
		debug = par("debug");
		executionDuration = par("totalSimTime");
		randomWaitMsgInterval = uniform(1,par("randomWaitMsgInterval"));

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

void RandomWaitTestApp::finish() {
    std::cout << "<"<<  (int)(simTime().dbl())<<"> Vehicle <" << findHost()->getId()<< "> sent <" << messagesSent <<"> messages. "  << std::endl;
    std::cout << "<"<<  (int)(simTime().dbl())<<"> Vehicle <" << findHost()->getId()<< "> received <" << messagesReceived <<"> messages. Finish Time <"<<  (int)(simTime().dbl())  <<">"<< std::endl;
    std::cout << "<"<<  (int)(simTime().dbl())<<"> Vehicle <" << findHost()->getId()<< "> has info about <" << inatisBuffer.getSize() <<"> segments. Finish Time <"<<  (int)(simTime().dbl() ) << ">"<<std::endl;

}

void RandomWaitTestApp::handleSelfMsg(cMessage *msg) {
    std::cout << "At handleSelfMsg <" << findHost()->getId() << ">" << std::endl;
    findHost()->getDisplayString().updateWith("r=6,yellow");
}

void RandomWaitTestApp::handleLowerMsg(cMessage* msg) {
    findHost()->getDisplayString().updateWith("r=6,blue");
    WaveShortMessage *wsm = (WaveShortMessage *)msg;
    std::cout << "<"<<  (int)(simTime().dbl())<<"> Vehicle <" << findHost()->getId()<< "> received message at time <" <<  simTime().dbl()  << ">"<<std::endl;

    messagesReceived++;
    InatisBuffer recvBuffer;
    recvBuffer.deserialize(wsm->getWsmData());

    inatisBuffer.Merge(recvBuffer);

	delete msg;
}

void RandomWaitTestApp::receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj, cObject* details) {
    Enter_Method_Silent();
	if (signalID == mobilityStateChangedSignal) {
		handlePositionUpdate();
	}
}

void RandomWaitTestApp::handlePositionUpdate() {

    int executionTime =(int)(simTime().dbl() - startTime);
    if (((executionTime - randomWaitMsgInterval - lastSentTimedMessage ) == 0) && (executionTime > lastSentTimedMessage)){
        randomWaitMsgInterval = uniform(1,par("randomWaitMsgInterval"));
        std::cout << "<"<<  (int)(simTime().dbl())<<"> Vehicle <" << findHost()->getId() << "> will send timed message at time <" << simTime().dbl() << ">"<< std::endl;
        messagesSent++;
        // Update current segment
        std::string r =  mobility->getRoadId();
        if(r.find('_') == std::string::npos){
            currentEntry.setSegmentId(stol(r));
            currentEntry.setSpeed((int)mobility->getSpeed());
            currentEntry.setTimestamp(floor(simTime().dbl()));
        }
        inatisBuffer.forceEntry(currentEntry);
        sendRandomWaitMessage(inatisBuffer.serialize());
        lastSentTimedMessage=executionTime;
    }

    // Update current segment
    std::string r =  mobility->getRoadId();

}

void RandomWaitTestApp::onData(WaveShortMessage* wsm) {

    std::cout << "At  onData <" << findHost()->getId() << ">" << std::endl;
    findHost()->getDisplayString().updateWith("r=16,gray");

    annotations->scheduleErase(1, annotations->drawLine(wsm->getSenderPos(), mobi->getCurrentPosition(), "blue"));

}

void RandomWaitTestApp::onBeacon(WaveShortMessage* wsm) {

}


void RandomWaitTestApp::sendRandomWaitMessage(std::string inatisMessage) {

    t_channel channel = dataOnSch ? type_SCH : type_CCH;
    WaveShortMessage* wsm = prepareWSM("data", dataLengthBits, channel, dataPriority, -1,2);

    wsm->setWsmData(inatisMessage.c_str());
    sendWSM(wsm);

}
void RandomWaitTestApp::sendWSM(WaveShortMessage* wsm) {

    sendDelayedDown(wsm,individualOffset);
}


