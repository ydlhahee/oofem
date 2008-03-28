/* $Header:$ */
/*

                   *****    *****   ******  ******  ***   ***                            
                 **   **  **   **  **      **      ** *** **                             
                **   **  **   **  ****    ****    **  *  **                              
               **   **  **   **  **      **      **     **                               
              **   **  **   **  **      **      **     **                                
              *****    *****   **      ******  **     **         
            
                                                                   
               OOFEM : Object Oriented Finite Element Code                 
                    
                 Copyright (C) 1993 - 2000   Borek Patzak                                       



         Czech Technical University, Faculty of Civil Engineering,
     Department of Structural Mechanics, 166 29 Prague, Czech Republic
                                                                               
    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.                                                                              
*/

//
// class CommunicationPacket
//

#ifndef dyncombuff_h
#define dyncombuff_h

#ifdef __PARALLEL_MODE

#include "parallel.h"
#include "combuff.h"
#ifndef __MAKEDEPEND
#include <list>
#endif

class IntArray;
class FloatArray;
class FloatMatrix;

#define __CommunicationPacket_DEFAULT_SIZE 10240

/**
 Class CommunicationPacket represent a data-packet, that is used to implement dynamic 
 communicator. Dynamic Communicator can pack messages into a dynamic message.
 This dynamic message is splitted into a series 
 of data packets of fixed size (this is not necessary) that are send over network.

 A special header is put at the begining of each packet buffer. This header keeps the message number 
 as well as the EOF flag indicating the last packet in message. This header is packed at the begining 
 of each packet.
*/

class CommunicationPacket : public MPIBuffer
{
 protected:
  int number;
  bool EOF_Flag;

public:
 
#ifdef __USE_MPI
 /// Constructor. Creeates buffer of given size, using given communicator for packing
 CommunicationPacket (MPI_Comm comm, int size, int num);
 /// Constructor. Creeates empty buffer, using given communicator for packing
 CommunicationPacket (MPI_Comm comm, int num);
#endif
 /// Destructor.
 ~CommunicationPacket ();

 /**
  Initializes buffer to empty state. All packed data are lost.
 */
 virtual void init (MPI_Comm comm);
 
 /**@name Services for buffer sending/receiving */
 //@{
#ifdef __USE_MPI
 /**
  Starts standart mode, nonblocking send.
  @param dest rank of destination
  @param tag message tag
  @param communicator (handle)
  @return sends MIP_Succes if ok
  */
 int iSend (MPI_Comm communicator, int dest, int tag);
 /**
  Starts standart mode, nonblocking receive. The buffer must be large enough to receive all data.
  @param source rank of source 
  @param tag message tag
  @param count number of elements to receive (bytes). Causes receive buffer to resize to count elements.
  If zero (default value) buffer is not resized.
  @param reguest communicator request (handle)
  @return MIP_Succes if ok
  */
 int iRecv (MPI_Comm communicator, int source, int tag, int count = 0);
 /**
  Tests if the operation identified by this->request is complete. 
  In such case, true is returned and
  if communication was initiated by nonblocking send/receive, then request handle 
  is set to MPI_REQUEST_NULL. Otherwise call returns flag=false.
  @return true if operation complete, false otherwise.
  */
#endif
 //@}
 
 void setNumber(int _num) {this->number=_num;}
 void setEOFFlag() {this->EOF_Flag = true;}
 int getNumber() {return number;}
 bool hasEOFFlag() {return EOF_Flag;}
 
 /// packs packet header info at receiver beginning
 int packHeader (MPI_Comm);
 int unpackHeader (MPI_Comm);
};

class CommunicationPacketPool
{
 private:
  std::list<CommunicationPacket*> available_packets;
  std::list<CommunicationPacket*> leased_packets;
  
  int allocatedPackets, leasedPackets, freePackets;
 public:
  CommunicationPacketPool() : available_packets(), leased_packets() {allocatedPackets=leasedPackets= freePackets;}
  ~CommunicationPacketPool() {this->printInfo(); this->clear();}

  CommunicationPacket* popPacket (MPI_Comm);
  void pushPacket (CommunicationPacket*);

  void printInfo ();
  
 private:
  void clear();
};


class DynamicCommunicationBuffer : public  CommunicationBuffer
{
 protected:
  std::list<CommunicationPacket*> packet_list;
  /// iterator to iterate over received packets
  std::list<CommunicationPacket*>::iterator recvIt;
  /// active packet
  CommunicationPacket *active_packet;
  /// active rank and tag  (send by initSend,initReceive, and initExchange)
  int active_tag, active_rank;
  int number_of_packets;

  // static packet pool
  static CommunicationPacketPool packetPool;
 public: 
  /// Constructor. Creeates buffer of given size, using given communicator for packing
  DynamicCommunicationBuffer (MPI_Comm comm, int size, bool dynamic=0) ;
  /// Constructor. Creeates empty buffer, using given communicator for packing
  DynamicCommunicationBuffer (MPI_Comm comm, bool dynamic=0);
 /// Destructor.
 virtual ~DynamicCommunicationBuffer ();

 virtual int resize (int newSize) {return 1;}
 virtual void init ();
 
 /// Initialize for packing
 virtual void initForPacking ();
 /// Initialize for Unpacking (data already received)
 virtual void initForUnpacking();
 
  virtual int packArray (const int* src, int n) 
    {return __packArray(src, n, MPI_INT);}
  virtual int packArray (const long* src, int n) 
    {return __packArray(src, n, MPI_LONG);}
  virtual int packArray (const unsigned long* src, int n) 
    {return __packArray(src, n, MPI_UNSIGNED_LONG);}
  virtual int packArray (const double* src, int n) 
    {return __packArray(src, n, MPI_DOUBLE);}
  virtual int packArray (const char* src, int n) 
    {return __packArray(src, n, MPI_CHAR);}

  virtual int unpackArray (int* dest, int n) 
    {return __unpackArray(dest, n, MPI_INT);}
  virtual int unpackArray (long* dest, int n) 
    {return __unpackArray(dest, n, MPI_LONG);}
  virtual int unpackArray (unsigned long* dest, int n) 
    {return __unpackArray(dest, n, MPI_UNSIGNED_LONG);}
  virtual int unpackArray (double* dest, int n) 
    {return __unpackArray(dest, n, MPI_DOUBLE);}
  virtual int unpackArray (char* dest, int n) 
    {return __unpackArray(dest, n, MPI_CHAR);}
 
 
 virtual int iSend (int dest, int tag);
 virtual int iRecv (int source, int tag, int count = 0);
 virtual int bcast (int root) ;

 virtual int sendCompleted ();
 virtual int receiveCompleted ();

 static void printInfo () {packetPool.printInfo();}

 protected:
 CommunicationPacket* allocateNewPacket (int);
 void freePacket (CommunicationPacket*);

 void popNewRecvPacket ();
 void pushNewRecvPacket (CommunicationPacket*);

 void clear ();
 int giveFitSize(MPI_Datatype type, int availableSpace, int arrySize);

 /** templated low-level array packing method.
     templated version used since implementation is similar for different types
     but type info is needed since implementation is relying on pointer arithmetic
 */
 template <class T> int __packArray (T* src, int n, MPI_Datatype type) {
   int _result=1;
   int start_indx=0, end_indx, _size;
   int remaining_size=n;

   do {
     _size = this->giveFitSize(type, active_packet -> giveAvailableSpace(), remaining_size);
     end_indx = start_indx + _size;
     
     if (_size) _result &= active_packet -> packArray (communicator, src+start_indx,_size, type);
     if (end_indx >= n) break;
     // active packet full, allocate a new one
     active_packet = this->allocateNewPacket (++number_of_packets);
     packet_list.push_back(active_packet);
     start_indx = end_indx;
     remaining_size -= _size;
   } while (1);
   
   return _result;
 } 
 
 /** templated low-level array unpacking method.
     templated version used since implementation is similar for different types
     but type info is needed since implementation is relying on pointer arithmetic
 */
 template <class T> int __unpackArray (T* dest, int n, MPI_Datatype type) {

   int _result=1;
   int start_indx=0, end_indx, _size;
   int remaining_size=n;
   
   do {
     _size = this->giveFitSize(type, active_packet -> giveAvailableSpace(), remaining_size);
     end_indx = start_indx + _size;
     
     if (_size) _result &= active_packet->unpackArray (communicator,dest+start_indx,_size, type);
     if (end_indx >= n) break;
     // active packet exhausted, pop a new one
     this->popNewRecvPacket();
     start_indx = end_indx;
     remaining_size -= _size;
   } while (1);
   
   return _result;
 }


};



#endif
#endif // dyncombuff_h
