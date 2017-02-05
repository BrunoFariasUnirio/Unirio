#include <algorithm>
#include <string>
#include <boost/unordered_map.hpp>
#include "veins/base/utils/asserts.h"

#include "veins/modules/application/traci/SOTISTestApp.h"
#include "veins/modules/application/traci/InatisEntry.h"
#include "veins/modules/application/traci/InatisBuffer.h"
#include "veins/modules/mobility/traci/TraCIColor.h"

using Veins::TraCIMobility;
using Veins::TraCIMobilityAccess;

using Veins::SOTISTestApp;
using Veins::InatisEntry;
using Veins::InatisBuffer;

Define_Module(Veins::SOTISTestApp);

const simsignalwrap_t SOTISTestApp::mobilityStateChangedSignal = simsignalwrap_t(MIXIM_SIGNAL_MOBILITY_CHANGE_NAME);

void SOTISTestApp::initialize(int stage) {
	BaseWaveApplLayer::initialize(stage);
	startTime=simTime().dbl();
	findHost()->getDisplayString().updateWith("r=16,blue");
	if (stage == 0) {
		debug = par("debug");
		executionDuration = par("totalSimTime");
		sotisMsgInterval = par("sotisMsgInterval");

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

void SOTISTestApp::finish() {
    std::cout << "<"<<  (int)(simTime().dbl())<<"> Vehicle <" << findHost()->getId()<< "> sent <" << messagesSent <<"> messages. "  << std::endl;
    std::cout << "<"<<  (int)(simTime().dbl())<<"> Vehicle <" << findHost()->getId()<< "> received <" << messagesReceived <<"> messages. Finish Time <"<<  (int)(simTime().dbl())  <<">"<< std::endl;
    std::cout << "<"<<  (int)(simTime().dbl())<<"> Vehicle <" << findHost()->getId()<< "> has info about <" << inatisBuffer.getSize() <<"> segments. Finish Time <"<<  (int)(simTime().dbl() ) << ">"<<std::endl;

}

void SOTISTestApp::handleSelfMsg(cMessage *msg) {
    std::cout << "At handleSelfMsg <" << findHost()->getId() << ">" << std::endl;
    findHost()->getDisplayString().updateWith("r=6,yellow");
}

void SOTISTestApp::handleLowerMsg(cMessage* msg) {
    findHost()->getDisplayString().updateWith("r=6,blue");
    WaveShortMessage *wsm = (WaveShortMessage *)msg;
    std::cout << "<"<<  (int)(simTime().dbl())<<"> Vehicle <" << findHost()->getId()<< "> received message at time <" <<  simTime().dbl()  << ">"<<std::endl;

    messagesReceived++;
    InatisBuffer recvBuffer;
    recvBuffer.deserialize(wsm->getWsmData());

    inatisBuffer.Merge(recvBuffer);

	delete msg;
}

void SOTISTestApp::receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj, cObject* details) {
    Enter_Method_Silent();
	if (signalID == mobilityStateChangedSignal) {
		handlePositionUpdate();
	}
}

void SOTISTestApp::handlePositionUpdate() {

    int executionTime =(int)(simTime().dbl() - startTime);
    if (((executionTime % sotisMsgInterval) == 0) && (executionTime > lastSentTimedMessage)){
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
        sendSOTISMessage(inatisBuffer.serialize());
        lastSentTimedMessage=executionTime;
    }

    // Update current segment
    std::string r =  mobility->getRoadId();

}

void SOTISTestApp::onData(WaveShortMessage* wsm) {

    std::cout << "At  onData <" << findHost()->getId() << ">" << std::endl;
    findHost()->getDisplayString().updateWith("r=16,gray");

    annotations->scheduleErase(1, annotations->drawLine(wsm->getSenderPos(), mobi->getCurrentPosition(), "blue"));

}

void SOTISTestApp::onBeacon(WaveShortMessage* wsm) {

}


void SOTISTestApp::sendSOTISMessage(std::string inatisMessage) {

    t_channel channel = dataOnSch ? type_SCH : type_CCH;
    WaveShortMessage* wsm = prepareWSM("data", dataLengthBits, channel, dataPriority, -1,2);

    wsm->setWsmData(inatisMessage.c_str());
    sendWSM(wsm);

}
void SOTISTestApp::sendWSM(WaveShortMessage* wsm) {

    sendDelayedDown(wsm,individualOffset);
}


