/*
 * InatisEntry.cpp
 *
 *  Created on: Dec 11, 2016
 *      Author: bruno
 */

#include "InatisEntry.h"
#include <iostream>
#include <string>

namespace Veins {
InatisEntry::InatisEntry() {
    // TODO Auto-generated constructor stub

}

InatisEntry::~InatisEntry() {
    // TODO Auto-generated destructor stub
}

    int InatisEntry::getDirection() const {
        return direction;
    }

    void InatisEntry::setDirection(int direction) {
        this->direction = direction;
    }

    int InatisEntry::getSpeed() const {
        return speed;
    }

    void InatisEntry::setSpeed(int speed) {
        this->speed = speed;
    }

    int InatisEntry::getTimestamp() const {
        return timestamp;
    }

    void InatisEntry::setTimestamp(int timestamp) {
        this->timestamp = timestamp;
    }

    long int InatisEntry::getSegmentId() const {
        return segmentId;
    }

    void InatisEntry::setSegmentId(long int segmentId) {
        this->segmentId = segmentId;
    }

    char * InatisEntry::serialize(){
        char * buf = (char *) calloc (15, sizeof(char));

        sprintf(buf,"%07ld%03d%04d",segmentId,speed,timestamp);

        //EV << "Buf eh " << buf << std::endl;
        return buf;
     }

    void InatisEntry::deserialize(const char * data){

        std::string str(data);

        segmentId = atol(str.substr(0,7).c_str());
        speed = atoi(str.substr(7,3).c_str());
        timestamp = atoi(str.substr(10,4).c_str());

        return;
    }


    /*char * InatisEntry::serialize(){

        char *data = new char[INATISENTRYSIZE];

        long int *vId = (long int*)data;
        *vId = this->vehicleId; vId++;
        char *vDir = (char *)vId;
        *vDir = this->direction; vDir++;
        unsigned char *vSpeed = (unsigned char *)vDir;
        *vSpeed = this->speed;vSpeed++;
        int *vTime = (int *)vSpeed;
        *vTime = this ->timestamp; vTime++;

        return data;

    }


    void InatisEntry::deserialize(const char * data){

        long int *vId = (long int*)data;
        this->setVehicleId(*vId);
        vId++;
        char *vDir = (char *)vId;
        this->setDirection(*vDir);
        vDir++;
        unsigned char *vSpeed = (unsigned char *)vDir;
        this->setSpeed(*vSpeed);
        vSpeed++;
        int *vTime = (int *)vSpeed;
        this->setTimestamp(*vTime);
        vTime++;

        return;
    }*/

    void InatisEntry::printMe(){
                /*EV << "\t######### Inatis entry print start ######\n ";
                EV << "\t\tSegment:\t <" << this->getSegmentId() << ">\n";
                EV << "\t\tDirection:\t <" << this->getDirection() << ">\n";
                EV << "\t\tSpeed:\t <" << this->getSpeed() << ">\n";
                EV << "\t\tTime:\t <" << this->getTimestamp() << ">\n";
                EV << "\t######### Inatis entry print end ######\n ";*/


                EV << "\t######### Inatis entry print start ######" << std::endl;
                EV << "\t\tSegment\t\tSpeed\t\tTime <" << this->getSegmentId() << ">\n";
                EV << "\t\t"<<this->getSegmentId()<<"\t\t"<<this->getSpeed()<<"\t\t"<<this->getTimestamp() << std::endl;
                EV << "\t######### Inatis entry print end ######\n ";


        return;
    }

} /* namespace entry */
