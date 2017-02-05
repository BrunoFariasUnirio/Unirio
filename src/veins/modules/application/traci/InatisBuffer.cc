/*
 * InatisBuffer.cc
 *
 *  Created on: Dec 12, 2016
 *      Author: bruno
 */


#include <map>
#include <iostream>
#include <string>
#include "InatisBuffer.h"
using namespace std;

namespace Veins {

InatisBuffer::InatisBuffer() {
    // TODO Auto-generated constructor stub

}

InatisBuffer::~InatisBuffer() {
    // TODO Auto-generated destructor stub
}

bool InatisBuffer::forceEntry(InatisEntry entry){
    buffer.erase(entry.getSegmentId());
    this->buffer.insert(std::make_pair(entry.getSegmentId(),entry));
    return true;
}

bool InatisBuffer::addEntry(InatisEntry entry){

    InatisEntry temp;

    temp = this->retrieveSegmentInfo(entry.getSegmentId());

    if (temp.getSegmentId()==0L){
        //No entry exists for segment
        if (this->buffer.size() == INATISBUFFERSIZE)
            removeOldestEntry();

        this->buffer.insert(std::make_pair(entry.getSegmentId(),entry));
        impactedSegmentIds.push_back(std::to_string(entry.getSegmentId()));
        return true;
    }else{
        if (temp.getTimestamp() < (entry.getTimestamp())){
            double newSpeed = (
                    (entry.getSpeed() + temp.getSpeed())
                    / 2);

            temp.setSpeed((int)newSpeed);
            temp.setTimestamp(entry.getTimestamp());
            removeEntry(entry);
            this->buffer.insert(std::make_pair(temp.getSegmentId(),temp));
            impactedSegmentIds.push_back(std::to_string(temp.getSegmentId()));
            return true;
        }
    }

    return false;
}


InatisEntry InatisBuffer::retrieveSegmentInfo(long int segmentId){

    InatisEntry retVal;
    retVal.setSegmentId(0L);

    pair<multimap<long int, InatisEntry>::iterator, multimap<long int, InatisEntry>::iterator> ii;
    multimap<long int, InatisEntry>::iterator it;
    ii = buffer.equal_range(segmentId);

    for(it = ii.first; it != ii.second; ++it)
    {
      retVal = (InatisEntry) it->second;
      break;
    }

    return retVal;
}

void InatisBuffer::removeOldestEntry(){

   InatisEntry entry,entryToDelete;
   long int oldestTimeStamp=999999999;
   entryToDelete.setTimestamp(oldestTimeStamp);
   multimap<long int, InatisEntry>::iterator it = buffer.begin();

   while(it != buffer.end())
   {
     entry=(InatisEntry)it->second;
     if (entry.getTimestamp() < entryToDelete.getTimestamp()){
         entryToDelete = entry;
     }

     it++;
   }

   removeEntry(entryToDelete);
}


bool InatisBuffer::removeEntry(InatisEntry entry){

    buffer.erase(entry.getSegmentId());

    return true;
}

std::string InatisBuffer::serialize(){

    InatisEntry entry;
    std::string outBuffer;
    multimap<long int, InatisEntry>::iterator it = buffer.begin();

    while(it != buffer.end())
    {
      entry=(InatisEntry)it->second;
      outBuffer.append(entry.serialize());
      it++;
    }

    return outBuffer;
}

bool InatisBuffer::deserialize(std::string inBuffer){

    InatisEntry entry;

    int length = inBuffer.length();

    if ((length % INATISBUFFERENTRYSIZE) != 0 ){
        std::cout << "Buffer deserialize failed: Invalid buffer size " << length << std::endl;
        return false;
    }

    for (int i=0; i < length / INATISBUFFERENTRYSIZE; i++){
        entry.deserialize(inBuffer.substr(
                                length - (i+1) * INATISBUFFERENTRYSIZE,
                                INATISBUFFERENTRYSIZE).c_str());
        this->buffer.insert(std::make_pair(entry.getSegmentId(),entry));
    }

    return true;
}

bool InatisBuffer::Merge(InatisBuffer bufferToMerge){

    bool retVal = false;
    impactedSegmentIds.clear();

/*
    std::cout << "Merging buffers " <<std::endl;
    std::cout << "Internal buffer - Inicio:" << std::endl;
    printMe();
    std::cout << "Internal buffer - End:" << std::endl;
    std::cout << "ToMerge buffer - Inicio:" << std::endl;
    bufferToMerge.printMe();
    std::cout << "ToMerge buffer - End:" << std::endl;

*/

    multimap<long int, InatisEntry>::iterator it = bufferToMerge.buffer.begin();
    while(it != bufferToMerge.buffer.end())
    {
      InatisEntry entry;
      entry=(InatisEntry)it->second;
      retVal = retVal || addEntry(entry);
      it++;
    }


    /*std::cout << "Final buffer - Inicio:" << std::endl;
    printMe();
    std::cout << "Final buffer- End:" << std::endl;
*/


    return retVal;
}

void InatisBuffer::printMe(){

    InatisEntry entry;
    std::cout << "### Inatis buffer print start ####" <<std::endl;

    multimap<long int, InatisEntry>::iterator it = buffer.begin();

    while(it != buffer.end())
    {
      entry=(InatisEntry)it->second;
      entry.printMe();
      it++;
    }

    std::cout << "### Inatis buffer print end ####" <<std::endl;

    return;
}

std::list<std::string> InatisBuffer::getImpactedSegmentIds(){
    return impactedSegmentIds;
}

int InatisBuffer::getSize(){
    return this->buffer.size();
}


} /* namespace Veins*/
