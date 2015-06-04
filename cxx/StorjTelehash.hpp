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
    #include "tgc.h"
}

#ifndef __NO_TGC__
extern "C"{
	#include "tgc.h"
}
#endif

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
     * link status flag for detecting it while reading packets.
     */
    static int status;

    /**
     * link status flag for detecting it while reading packets.
     */
    static int count;

    /**
     * target for detecting a link while reading packets.
     */
    static link_t targetLink;

    /**
     * determin whethere adr is local or not.
     * 
     * @param adr address to be checked.
     * @return 0 if global IP, others if local IP.
     */
    int isLocal(char *adr);

    /**
     * try to get global IP from ethernet IF. 
     * 
     * @param global ip would be stored if found.
     * @return pointer of ip.
     */
    char *getGlobalIP(char *ip);

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

    /**
     * called when receving one packet on a channel.
     * call ChannelHandler.handle() and send back handle()'s result.
     * called from serviceOnOpenHandler() and serviceOnOpen().
     * 
     * @param link link used in this channel.
     * @param chan channel
     * @param packet paccket that is sent.
     * @param c ChannelHandler instance.
     */
    void static sendOnChannel(link_t link, e3x_channel_t chan,
                                      lob_t packet,ChannelHandler *c);
                                          
    /**
     * handler called when finishing making a link. 
     * (an argument in mesh_on_link())
     * 
     * @param link link
     */
    static void onLink(link_t link);

    /**
     * handler called when opening a channel. intend to be called when 
     * requested a general channel. 
     * (an argument in mesh_on_open())
     * 
     * @param link link used in this channel.
     * @param open packet content.
     * @return NULL if channel is processed in this handler. open if not.
     */
    static lob_t serviceOnOpen(link_t link,lob_t open);

    /**
     * handler when requested to add broadcastee.
     * (an argument in mesh_on_open())
     * 
     * @param link link used in this channel.
     * @param open packet content.
     * @return NULL if channel is processed in this handler. open if not.
     */
    static lob_t signupOnOpen(link_t link,lob_t open);

    /**
     * handler when receiving a broadcaste.
     * (an argument in mesh_on_open())
     * 
     * @param link link used in this channel.
     * @param open packet content.
     * @return NULL if channel is processed in this handler. open if not.
     */
    static lob_t broadcastOnOpen(link_t link,lob_t open);

    /**
     * handler when receiving a return of request to add broadcastee.
     * receive IP address and store it.
     * called from addBroadcaster().
     * 
     * @param link link used in this channel.
     * @param chan channel.
     * @param arg not used. NULL.
    */
    static void addBroadcasterHandler(link_t link, e3x_channel_t chan, 
        void *arg);

    /**
     * handler when receiving a return of opening channel and sending a 
     * message to a channel.
     * called from openChannel() and serviceOnOpen().
     * 
     * @param link link used in this channel.
     * @param chan channel.
     * @param arg ChannelHandler instance.
     */
    static void serviceOnOpenHandler(link_t link, e3x_channel_t chan, 
        void *arg);
    
public:
    /**
     * global IP address, if found.
     * try to retrieve from ethere IF and return of addBroadcast().
     */
    static char globalIP[3*4+3+1];

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
     * @param broadcastHandler ChannelHandler instnace for handlinkg broadcasts 
     */
    StorjTelehash(int port,ChannelHandlerFactory &factory, 
                      ChannelHandler& broadcastHandler);

    /**
     * get registered ChannelHandlerFactory.
     * 
     * @return registered ChannelHandlerFactory
     */
    ChannelHandlerFactory* getChannelHandlerFactory();

    /**
     * get registered ChannelHandler instance for handlinb broadcast.
     * 
     * @return registered broadcast ChannelHandler
     */
    ChannelHandler* getBroadcastHandler();

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
     * 
     * add/remove a broadcaster. broadcast messages will be sent from here 
     * after adding a broadcaster.
     * 
     * @param location broadcaster location.
     * @param add 0 if removing. others if adding.
     */
    void addBroadcaster(char *location,int add);

    /**
     * broadcast a packet.
     * 
     * @param location first node to be broadcasted.
     * @param json packet to be broadcasted.
     */
    void broadcast(char *location, char *json);

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
     * run garbage collection.
     * You must go out from functions in telehash-c library and 
     * stop all threads before calling this method. 
     */
    static void gcollect();

    /**
     * test use only. don't use it.
     * @return address to be tested.
     */
    int _isLocalTest(char *addr);
};
#endif
