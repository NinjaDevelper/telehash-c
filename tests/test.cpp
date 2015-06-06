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

#include "StorjTelehash.hpp"

#include "picojson.h"
#include <stdio.h>
#include <tap.h>
#include <vector>  
#include <string>
#include <thread>
#include <string.h>

using namespace std;
using namespace picojson;

typedef char * (*CHANNEL_HANDLER)(char *json);

class TestChannelHandler : public ChannelHandler{
private:
    /**
     *  current number of handling.
     */
    unsigned int n;
    
    /**
     * handlers 
     */
    vector<CHANNEL_HANDLER> h;

public:
    /**
     * create myself with handlers.
     * 
     * @param h channels.
     */
	TestChannelHandler(vector<CHANNEL_HANDLER> &h){
 		n=0;
		this->h.assign(h.begin(),h.end());
	}
	
    /**
     * handle one packet.
     * 
     * @param json one packet described in json.
     * @return a json packet that should be sent back. not be sent if NULL.
     */
	char* handle(char *json){
		if(n<h.size()){
			CHANNEL_HANDLER f=h.at(n++);
 		    return f(json);
		}
		return NULL;
	}
    
    ~TestChannelHandler(){
    }
};

class TestChannelHandlerFactory : public ChannelHandlerFactory{
private:
    /**
     * handlers and channel name map. 
     */
	map<string,vector<CHANNEL_HANDLER> > hmap;
public:

    /**
     * add  handlers with channel name.
     * 
     * @param name channel name.
     * @param h channel handlers.
     */
	void addChannelHandlers(string name, vector<CHANNEL_HANDLER> &h){
		hmap[name]=h;
	}
    /**
     * return myself which is associated channel name.
     * 
     * @param name channel name.
     * @return myself associated the channel name.
     */
	virtual ChannelHandler* createInstance(string name){
        if (hmap.find(name) == hmap.end()) {
            return NULL;
        }
		return new TestChannelHandler(hmap[name]);
	}
    ~TestChannelHandlerFactory(){
    }
};

int statusO=0;
char *openerHandler_1(char *json){
    char *message=NULL;
   
    message=(char *)malloc(256);
    strcpy(message,"{\"count\":0}");
    statusO++;
    return message;
}

char *openerHandler_2(char *json){

    value v1;
    parse(v1, json);
    double count=v1.get<object>()["count"].get<double>();
    ok(count==1,"channel message test 1");
    statusO++;
   
    return NULL;
}

char *openerHandler_3(char *json){
    statusO=-9999;
   
    return NULL;
}


int statusR=0;
char *receiverHandler_1(char *json){
    char *message=NULL;
   
    message=(char *)malloc(256);
    value v1;
    parse(v1, json);
    double count=v1.get<object>()["count"].get<double>();
    ok(count==0,"channel message test 0");
    strcpy(message,"{\"count\":1}");
    statusR++;

    return message;
}

char *receiverHandler_2(char *json){
    statusR=-9999;
   
    return NULL;
}



class StorjTelehash2 {
private:
    StorjTelehash *s;
    thread t;
    static vector<StorjTelehash2 *> st2;

public:
    StorjTelehash2(StorjTelehash &s){
        this->s=&s;
        st2.push_back(this);
    }

    void startThread(){
        s->setStopFlag(0);
        t=std::thread([&](){
            s->start();
        });
    }
    void stopThread(){
        s->setStopFlag(1);
        t.join();
    }

    void ping(char *loc){
        stopThread();
        s->ping(loc);
        startThread();
    }
    void openChannel(char *loc,char *name, ChannelHandler &handler){
        stopThread();
        s->openChannel(loc,name,handler);
        startThread();
    }
    static void stopAll(){
        vector<StorjTelehash2 *>::iterator it = st2.begin();
        while( it != st2.end() ){
            (*it)->stopThread();
            it++;
        }
    }
    static void startAll(){
        vector<StorjTelehash2 *>::iterator it = st2.begin();
        while( it != st2.end() ){
            (*it)->startThread();
            it++;
        }
    }
};

vector<StorjTelehash2 *> StorjTelehash2::st2;

TestChannelHandlerFactory factory;

//m1 cannot connect to m2 because they share a key
StorjTelehash m1(0,factory);
StorjTelehash m2(1234,factory);
StorjTelehash m3(-9999,factory);
StorjTelehash m4(-9999,factory);
StorjTelehash2 s2(m2);
StorjTelehash2 s3(m3);
StorjTelehash2 s4(m4);


string location_;
char *location=NULL;


void locationTest(){
    LOG("starting location test...");
    ChannelHandlerFactory *h=m1.getChannelHandlerFactory();
    ok( h==&factory,"getChannelHandlerFactory check.");
    string str;
    LOG("%x at %s",&m1,m1.getMyLocation(str).c_str());
    LOG("%x at %s",&m2,m2.getMyLocation(str).c_str());
    LOG("%x at %s",&m3,m3.getMyLocation(str).c_str());
    LOG("%x at %s",&m4,m4.getMyLocation(str).c_str());
   
    string info1,info2,id;
    m1.getMyId(id);
    LOG("id %s",id.c_str());
    ok(id.length() == 52 , "hashname size check");

    m1.getMyLocation(info1);
    m2.getMyLocation(info2);
    LOG("info %s",info2.c_str());
    value v1,v2;
    parse(v1, info1);
    parse(v2, info2);
    
    string h1=v1.get<object>()["hashname"].get<string>();
    string h2=v2.get<object>()["hashname"].get<string>();
    LOG("hashname=%s",h1.c_str());
    ok( h1==h2,"hashname check.");

    string k1=v1.get<object>()["keys"].get<object>()["1a"].get<string>();
    string k2=v2.get<object>()["keys"].get<object>()["1a"].get<string>();
    LOG("keys=%s",k1.c_str());
    ok( k1==k2,"keys check.");

    int p2=v2.get<object>()["paths"].get<picojson::array>()[0].
            get<object>()["port"].get<double>();
    LOG("port=%d",p2);
    ok( p2==1234,"port check.");
    
    ok( m1._isLocalTest((char *)"192.168.1.1"),"isLocal() check 1.");
    ok( m1._isLocalTest((char *)"172.16.22.22"),"isLocal() check 2.");
    ok( m1._isLocalTest((char *)"172.22.22.22"),"isLocal() check 3.");
    ok( !m1._isLocalTest((char *)"172.32.22.22"),"isLocal() check 4.");
    ok( !m1._isLocalTest((char *)"172.15.22.22"),"isLocal() check 5.");

    location_=
        "{\"keys\":{\"1a\":\""+k2+"\"},\"paths\":[{\"type\":\"udp4\",\"ip\""
        ":\"127.0.0.1\",\"port\":1234}]}";
    location=(char *)location_.c_str();
    LOG("%s",location);

}


void channelTest(){
    LOG("starting channel test...");
    s2.startThread();
    s3.startThread();
    s4.startThread();

    s4.ping(location);
    string str;
    m4.getMyLocation(str);
    value v;
    parse(v, str);
    
    string p=v.get<object>()["paths"].get<picojson::array>()[0].
            get<object>()["ip"].get<string>();
    LOG("global ip=%s",p.c_str());    
    ok(p.length() > 8 ,"global ip test");


    vector<CHANNEL_HANDLER> h1;
    h1.push_back(receiverHandler_1);
    h1.push_back(receiverHandler_2);
    
    factory.addChannelHandlers((char *)"counter_test",h1);

    vector<CHANNEL_HANDLER> h2;
    h2.push_back(openerHandler_1);
    h2.push_back(openerHandler_2);
    h2.push_back(openerHandler_3);
    TestChannelHandler *tch=new TestChannelHandler(h2);

    statusO=0;
    statusR=0;
    s3.openChannel((char *)location,(char *)"counter_test", *tch);
    sleep(10);
    LOG("receiver status=%d",statusR);
    LOG("opener status=%d",statusO);
    ok( statusO==2,"channel opener check.");
    ok( statusR==1,"channel reciever check.");
}

int main (int argc, char *argv[]) {

    locationTest();
    channelTest();

    StorjTelehash2::stopAll();

    done_testing();

}
