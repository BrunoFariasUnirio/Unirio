/*
 * InatisEntry.h
 *
 *  Created on: Dec 11, 2016
 *      Author: bruno
 */

#ifndef INATISENTRY_H_
#define INATISENTRY_H_

#include <omnetpp.h>
namespace Veins {

class InatisEntry {
protected:
    long int segmentId;
    int direction = 0;
    int speed;
    int timestamp;
public:
    InatisEntry();
    virtual ~InatisEntry();

    int getDirection() const;
    void setDirection(int direction);
    int getSpeed() const;
    void setSpeed(int speed);
    int getTimestamp() const;
    void setTimestamp(int timestamp);
    long int getSegmentId() const;
    void setSegmentId(long int segmentId);

    char * serialize();
    void deserialize(const char * data);

    void printMe();
};

} /* namespace inatis */

#endif /* INATISENTRY_H_ */
