/*
 * Copyright (c) 2015, Shinya Yagyu
 * All rights reserved.
 * Redistribution and use in source and binary forms, with or without 
 * modification, are permitted provided that the following conditions are met:
 * 
 * 1. Redistributions of source code must retain the above copyright notice, 
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. Neither the name of the copyright holder nor the names of its 
 *    contributors may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" 
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR 
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF 
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN 
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE 
 * POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef _StorjTelehash_H_
#define _StorjTelehash_H_

#include <vector>  
#include <string>  
#include <list>  
#include <map>  

extern "C"{
	#include "mesh.h"
    #include "net_udp4.h"
}

using namespace std;

/**
 * Class for handling  received packets in telehash-c channel.
 * 
 * handle() method is called per one packet,
 */
class ChannelHandler{

public:
    /**
     * handle one packet.
     * 
     * @param json one packet described in json.
     * @return a json packet that should be sent back. If NULL, no packets 
     *          are sent back.
     */
	virtual char* handle(char *json)=0;
    
    /**
     * destructor
     */
    virtual ~ChannelHandler(){}
};

/**
 * Class for creating ChannelFactory.
 */
class ChannelHandlerFactory{
public:
    /**
     * return a ChannelHandler instance which is associated channel name.
     * 
     * @param name channel name.
     * @return ChannelHandler instance associated the channel name.
     */
    virtual ChannelHandler* createInstance(string name)=0;
 
    /**
     * destructor
     */
    virtual ~ChannelHandlerFactory(){}
};



 /**
  * class for managing telehash-c.
  * 
  * Everything is not thread safe. Call a function after stopoping
  *  start() loop by setStopFlag(1).
  * It is NOT recommended to make more than one instance. If you want to do so
  * (e.g. for test use), use setGC(0) to stop GC.
  */
class StorjTelehash{
private:
    /**
     * telehash-c mesh.
     */
    mesh_t mesh;

    /**
     * telehash-c stuffs for handling evnet(reading packet).
     */
    net_udp4_t udp4;

    /**
     * stop flag while looping to read packets.
     */
    int stopFlag;

    /**
     * make id.json, where key and hashname is stored.
     * 
     * @return 0 if failed, others if success.
     */
    int writeID();

    /**
     * make a link. net_udp4_received() is looped until finishing a link.
     * 
     * @param location where linked. 
     */
    link_t _link(char *location);


public:
    /**
     * constructor.
     * 
     * If id.json including hashname is not existed,it will be created.
     * Instances are always created based on id.json except port=-9999.
     * Never use port=-9999 except for test use.
     * 
     * @param port port number to be listened packets. if 0, port number is 
     *         selected randomly.
     * @param factory ChannelHanderFactory instance.
     */
    StorjTelehash(int port,ChannelHandlerFactory &factory);

    /**
     * get registered ChannelHandlerFactory.
     * 
     * @return registered ChannelHandlerFactory
     */
    ChannelHandlerFactory* getChannelHandlerFactory();


    /**
     * destructor.
     * free some telehash-c stuffs.
     */
    ~StorjTelehash();

    /**
     * return my location information. format is:
     * {"keys":{"1a":"al45izsjxe2sikv7mc6jpnwywybbkqvsou"},
     * "paths":[{"type":"udp4","ip":"127.0.0.1","port":1234}]
     * 
     * @param location wherere location info is stored.
     * @return location info.
     */
    string &getMyLocation(string &location);

    /**
     * return my id. format is:
     * jlde3uibwflz4hqnk4zehvj5o5kd4goyqtrwqwhiotw6n4qtrf2a
     * 
     * @param id  id info is stored.
     * @return id string.
     */
    string &getMyId(string &id);

    /**
     * open a channel associated a channel name.
     * it calls _link(), and net_udp4_received() is looped.
     * be careful when threading not to be reentrance.
     * 
     * ChannelHandler &h will be deleted in this function.
     * 
     * @param location destination to be linked.
     * @param name channel name associated to channel.
     * @param h channel handler to send a first packet and handle 
     *         received packets.
     */
    void openChannel(char *location, char *name, ChannelHandler &h);

    /**
     * start receiving packet loop.
     * 
     */
    void start();

    /**
     * set stopFlag that stop/continue a loop..
     *
     * @param flag 1 if stop. others if run.
     */
    void setStopFlag(int flag);

    /**
     * test use only. don't use it.
     * @return address to be tested.
     */
    int _isLocalTest(char *addr);

    /**
     * open ping channel. expect a global ip address.
     * 
     * @param location destination to ping.
     */
    void ping(char *location);
};
#endif
