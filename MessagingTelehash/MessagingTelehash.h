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

#ifndef _MESSAGINGTELEHASH_H_
#define _MESSAGINGTELEHASH_H_

#include <vector>  
#include <list>  
#include <map>  

extern "C"{
	#include "mesh.h"
    #include "net_udp4.h"
}

using namespace std;
typedef char * (*CHANNEL_HANDLER)(char *json);

class ChannelHandler{
private:
    unsigned int n;
    vector<CHANNEL_HANDLER> h;
	static  map<string,vector<CHANNEL_HANDLER> > hmap;

public:
	static void addChannelHandler(string name, vector<CHANNEL_HANDLER> &h){
		hmap[name]=h;
	}
	static ChannelHandler* createInstance(string name){
        if (hmap.find(name) == hmap.end()) {
            return NULL;
        }
		return new ChannelHandler(hmap[name]);
	}
	
	ChannelHandler(vector<CHANNEL_HANDLER> &h){
		n=0;
		this->h.assign(h.begin(),h.end());
	}
	
	char* handle(char *json){
		if(n<h.size()){
			CHANNEL_HANDLER f=h.at(n++);
 		    return f(json);
		}
		return NULL;
	}
};

class MessagingTelehash{
private:
    mesh_t mesh;
    net_udp4_t udp4;
    int stopFlag;
    vector<link_t> links;
    static int status;
    static link_t targetLink;
    static list<link_t> broadcastee;
    static char * (*broadcastHandler)(char *json) ;
    
    int isLocal(char *adr);
    char *getGlobalIP(char *ip);
    int writeID();
    link_t _link(char *location);
    void static sendOnChannel(link_t link, e3x_channel_t chan,
                                      lob_t packet,ChannelHandler *c);
                                          
    static void onLink(link_t link);
    static lob_t serviceOnOpen(link_t link,lob_t open);
    static lob_t signupOnOpen(link_t link,lob_t open);
    static lob_t broadcastOnOpen(link_t link,lob_t open);

    static void addBroadcasterHandler(link_t link, e3x_channel_t chan, 
        void *arg);
    static void serviceOnOpenHandler(link_t link, e3x_channel_t chan, 
        void *arg);
    
public:
    static char globalIP[3*4+3+1];
    MessagingTelehash(int port);
    ~MessagingTelehash();
    char *getMyLocation();
    void openChannel(char *location, char *name);
    void setChannelHandlers(char* name,  vector<CHANNEL_HANDLER> &h);
    void addBroadcaster(char *location,int add);
    void setBroadcastHandler(char * (&bc)(char *json) );
    void broadcast(char *location, char *json);
    void start();
    void stop();
};
#endif
