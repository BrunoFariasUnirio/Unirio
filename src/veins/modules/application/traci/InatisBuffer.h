/*
 * InatisBuffer.h
 *
 *  Created on: Dec 12, 2016
 *      Author: bruno
 */

#include <map>
#include <list>


#include "InatisEntry.h"

#ifndef INATISBUFFER_H_
#define INATISBUFFER_H_

#define INATISBUFFERSIZE 168
#define INATISBUFFERENTRYSIZE 14
namespace Veins {

class InatisBuffer {
protected:
    std::multimap<long int, InatisEntry> buffer;
    std::list<std::string> impactedSegmentIds;


public:
    InatisBuffer();
    virtual ~InatisBuffer();

    bool addEntry(InatisEntry entry);
    InatisEntry retrieveSegmentInfo(long int segmentId);
    bool removeEntry(InatisEntry entry);
    void printMe();
    std::string serialize();
    bool deserialize(std::string inBuffer);
    bool Merge(InatisBuffer bufferToMerge);
    bool forceEntry(InatisEntry entry);
    void removeOldestEntry();
    std::list<std::string> getImpactedSegmentIds();
    int getSize();

};

} /* namespace veins */

#endif /* INATISBUFFER_H_ */
